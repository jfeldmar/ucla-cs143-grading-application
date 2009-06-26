/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"
#include "PageFile.h"

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{

	RC r = pf.open(indexname, mode);
	if(pf.endPid() != 0)
	{
		int buf[256];
		pf.read(0, buf);
		rootPid = buf[0];
		treeHeight = buf[1];
	}

    return r;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	int buf[256];
	buf[0] = rootPid;
	buf[1] = treeHeight;
	pf.write(0, buf);
	RC r = pf.close();

    return r;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	RC r;

	if(pf.endPid() == 0)
	{
		//reserve space for private variables in disk
		int buf[256] = {0};
		pf.write(0, buf);
		
		//initialize private variables
		rootPid = 1;
		treeHeight = 1;
		
		//initialize root
		BTNonLeafNode root;
		root.initializeRoot(2, key, 3);
		root.write(1, pf);
		
		BTLeafNode node1, node2;
		
		node1.write(2, pf);
		node2.insert(key, rid);
		node2.write(2, pf);

	}
	
	else
	{
		BTLeafNode leaf;
		BTNonLeafNode nonleaf;
		int siblingKey;
		r = recInsert(key, rid, 0, rootPid, leaf, nonleaf, siblingKey);
		if(r == RC_NODE_FULL)
		{
			int p = pf.endPid();
			nonleaf.write(p, pf);
			BTNonLeafNode root;
			root.initializeRoot(rootPid, siblingKey, p);
			p = pf.endPid();
			root.write(p, pf);
			
			rootPid = p;
			treeHeight++;
		}
		
	}

    return 0;
}

/*
 * Find the leaf-node index entry whose key value is larger than or 
 * equal to searchKey, and output the location of the entry in IndexCursor.
 * IndexCursor is a "pointer" to a B+tree leaf-node entry consisting of
 * the PageId of the node and the SlotID of the index entry.
 * Note that, for range queries, we need to scan the B+tree leaf nodes.
 * For example, if the query is "key > 1000", we should scan the leaf
 * nodes starting with the key value 1000. For this reason,
 * it is better to return the location of the leaf node entry 
 * for a given searchKey, instead of returning the RecordId
 * associated with the searchKey directly.
 * Once the location of the index entry is identified and returned 
 * from this function, you should call readForward() to retrieve the
 * actual (key, rid) pair from the index.
 * @param key[IN] the key to find.
 * @param cursor[OUT] the cursor pointing to the first index entry
 *                    with the key value.
 * @return error code. 0 if no error.
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
    return recFind(searchKey, cursor, 0, rootPid);
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
	RC r;
	
	BTLeafNode leaf;
	leaf.read(cursor.pid, pf);
	leaf.readEntry(cursor.eid, key, rid);
	cursor.eid++;
	if(cursor.eid>leaf.getKeyCount())
	{
		cursor.eid = 0;
		r = leaf.getNextNodePtr();
		if(r == RC_NO_SUCH_RECORD)
			return r;
		else
			cursor.pid = r;
	}

    return 0;
}

 //Recursively traverse tree to insert key.
 //the siblings are allocated in case of some kind of overflow.
  RC BTreeIndex::recInsert(int key, const RecordId& rid, int height, PageId& pid, BTLeafNode& leafsibling, BTNonLeafNode& nonleafsibling, int& siblingKey)
  {
  
	RC r;
	
	if(height == treeHeight)
	{
		BTLeafNode leaf;
		leaf.read(pid, pf);
		
		//try to insert into leaf node
		r = leaf.insert(key, rid);
		if(r == RC_NODE_FULL)
		{
			leaf.insertAndSplit(key, rid, leafsibling, siblingKey);
			return r;
		}
	}
	
	else
	{
		BTNonLeafNode nonleaf;
		nonleaf.read(pid, pf);
		nonleaf.locateChildPtr(key, pid);
		r = recInsert(key, rid, height, pid, leafsibling, nonleafsibling, siblingKey);
		if(r == RC_NODE_FULL) //need to create page for leafsibling
		{
			if(height+1 == treeHeight)
				leafsibling.write(pf.endPid(), pf);
			else
				nonleafsibling.write(pf.endPid(), pf);
			
			r = nonleaf.insert(siblingKey, pid);
			if(r == RC_NODE_FULL)
			{
				nonleaf.insertAndSplit(siblingKey, pid, nonleafsibling, siblingKey);
				return r;
			}
		}
		
		
	
	}
	
	return 0;
  }
  
  RC BTreeIndex::recFind(int searchKey, IndexCursor& cursor, int height, PageId& pid)
  {
	
	RC r;
	
	if(height == treeHeight)
	{
		BTLeafNode leaf;
		leaf.read(pid, pf);
		if((r = leaf.locate(searchKey, cursor.eid)) != 0)
			return r;
		RecordId rid;
		int key;
		leaf.readEntry(cursor.eid, key, rid);
		cursor.pid = rid.pid;
	}
	
	else
	{
		BTNonLeafNode nonleaf;
		nonleaf.read(pid, pf);
		nonleaf.locateChildPtr(searchKey, pid);
		recFind(searchKey, cursor, height+1, pid);
	}
	
	return 0;
  }

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

#include <iostream>

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
	rootPid = -1;
	treeHeight = 0;
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
	if(mode != 'W' && mode != 'w' && mode != 'R' && mode != 'r')
		return RC_INVALID_FILE_MODE;

	RC rc = pf.open(indexname + ".idx", mode);

	char buffer[PageFile::PAGE_SIZE];

	if(pf.endPid() > 0)
	{
		pf.read(0, buffer);
		rootPid = buffer[0];
		treeHeight = buffer[1];
	}
	else if(pf.endPid() == 0)
	{
		rootPid = 1;
		treeHeight = 0;
		
		buffer[0] = rootPid;
		buffer[1] = treeHeight;
		pf.write(0, buffer);
		
		BTLeafNode root;
		root.setNextNodePtr(-1);
		root.write(rootPid, pf);
	}
	else
		rc = RC_INVALID_PID;

	return rc;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	char buffer[PageFile::PAGE_SIZE];
	
	buffer[0] = rootPid;
	buffer[1] = treeHeight;

	pf.write(0, buffer);

	return pf.close();
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	if(rid.pid < 0 || rid.sid < 0)
		return RC_INVALID_RID;

	PageId startPid = rootPid;
	bool isRootFull = traverseInsert(0, key, rid, startPid);
	if(isRootFull)
	{
		BTNonLeafNode newRoot;
		PageId pidNewRoot = pf.endPid();
		newRoot.initializeRoot(rootPid, key, startPid);
		rootPid = pidNewRoot;
		treeHeight++;
		newRoot.write(rootPid, pf);
		newRoot.printNode();
	}

	return 0;
}

bool BTreeIndex::traverseInsert(int depth, int& searchKey, const RecordId& rid, PageId& pid)
{
	if(depth == treeHeight)
	{
		BTLeafNode leaf;
		leaf.read(pid, pf);
		if(leaf.insert(searchKey, rid) == RC_NODE_FULL)
		{
			PageId pidNewNode = pf.endPid();
			BTLeafNode siblingLeafNode;
			int siblingKey;
			leaf.insertAndSplit(searchKey, rid, siblingLeafNode, 
						siblingKey);
			leaf.setNextNodePtr(pidNewNode);
			siblingLeafNode.write(pidNewNode, pf);
			leaf.write(pid, pf);

			searchKey = siblingKey;
			pid = pidNewNode;

			return true;
		}
		else
		{
			leaf.write(pid, pf);
			return false;
		}
	}

	BTNonLeafNode node;
	node.read(pid, pf);
	PageId pidLocate;
	node.locateChildPtr(searchKey, pidLocate);
	bool isFull = traverseInsert(depth + 1, searchKey, rid, pidLocate);

	if(isFull)
	{
		BTNonLeafNode nonLeaf;
		nonLeaf.read(pid,pf);
		if(nonLeaf.insert(searchKey, pidLocate) == RC_NODE_FULL)
		{
			PageId pidNewNode = pf.endPid();
			BTNonLeafNode siblingNonLeafNode;
			int midKey;
			nonLeaf.insertAndSplit(searchKey, pid, 
					siblingNonLeafNode, midKey);
			siblingNonLeafNode.write(pidNewNode, pf);
			nonLeaf.write(pid, pf);
			
			searchKey = midKey;
			pid = pidNewNode;

			return true;
		}
		else
		{
			nonLeaf.write(pid, pf);
			return false;
		}
	}
	return false;
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
	PageId rPid = rootPid;
	int eid = 0;
	traverseLocate(0, searchKey, rPid, eid);

	cursor.pid = rPid;
	cursor.eid = eid;

	return 0;
}

RC BTreeIndex::traverseLocate(int depth, int searchKey, PageId& pid, int &eid)
{
	if(depth == treeHeight)
	{
		BTLeafNode leafNode;
		leafNode.read(pid, pf);
		leafNode.locate(searchKey, eid);
		return 0;
	}

	BTNonLeafNode nonLeaf;
	nonLeaf.read(pid, pf);
	nonLeaf.locateChildPtr(searchKey, pid);

	return traverseLocate(depth + 1, searchKey, pid, eid);
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
	if(cursor.pid < 0 || cursor.pid > pf.endPid() || 
		cursor.eid < 0 || cursor.eid > (MAX_NUM_LEAF_PTRS - 1))
		return RC_INVALID_CURSOR;

	BTLeafNode leafNode;

	leafNode.read(cursor.pid, pf);
	leafNode.readEntry(cursor.eid, key, rid);

	if((cursor.eid + 1) == (leafNode.getKeyCount()))
	{
		cursor.pid = leafNode.getNextNodePtr();
		cursor.eid = 0;
		if(cursor.pid == -1)
			return RC_END_OF_TREE;
	}
	else
		cursor.eid++;

	return 0;
}

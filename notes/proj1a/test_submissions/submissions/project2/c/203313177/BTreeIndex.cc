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

// My Additions
#include <stdlib.h>
#include <stdio.h>


using namespace std;

const int PAGESIZE = PageFile::PAGE_SIZE;
const int MAX_LEAF_KEYS = (PAGESIZE-4)/12;
const int MAX_NONLEAF_KEYS = (PAGESIZE-4)/8;
const int ERROR = -1;
/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = 0;
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
	RC rc;
	
	if ((rc = pf.open(indexname, mode)) < 0) return rc;
	
	return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
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
	IndexCursor ic;
	if (treeHeight == 0) // EMPTY TREE
	{
		BTLeafNode newLeaf;
		newLeaf.insert(key, rid);
		rootPid = pf.endPid();
		if (newLeaf.write(rootPid, pf))
			return ERROR;
		treeHeight = 1;
		return 0;
	}
	
	if (locate(key, ic))
		return ERROR;
	
	BTLeafNode tempLeaf;
	if (tempLeaf.read(ic.pid, pf))
		return ERROR;
	
	if (tempLeaf.getKeyCount() < MAX_LEAF_KEYS) // ROOM IN LEAF NODE
	{
		tempLeaf.insert(key, rid);
		if(tempLeaf.write(ic.pid, pf))
			return ERROR;
		return 0;
	}
	
	int midKey;
	BTLeafNode newLeaf;
	if (tempLeaf.insertAndSplit(key, rid, newLeaf, midKey))
		return ERROR;
	
	newLeaf.setNextNodePtr(tempLeaf.getNextNodePtr());
	tempLeaf.setNextNodePtr(pf.endPid());
	if (newLeaf.write(pf.endPid(), pf) || tempLeaf.write(ic.pid, pf))
		return ERROR;
	// NEW LEAF NODES WRITTEN
	
	BTNonLeafNode tempNonLeaf;
	PageId nextPid = tempLeaf.getNextNodePtr();
	PageId prevPid = ic.pid;
	while (true)
	{
		if (nodePath.empty()) //NEED NEW ROOT
		{
			BTNonLeafNode newNonLeaf;
			newNonLeaf.initializeRoot(prevPid, midKey, nextPid);
			rootPid = pf.endPid();
			if (newNonLeaf.write(rootPid, pf))
				return ERROR;
			treeHeight++;
			return 0;
		}
		
		if (tempNonLeaf.read(nodePath.back(), pf))
			return ERROR;
		
		if (tempNonLeaf.getKeyCount() < MAX_NONLEAF_KEYS) // PARENT HAS ROOM FOR NEW KEY
		{
			tempNonLeaf.insert(midKey, nextPid);
			if (tempNonLeaf.write(nodePath.back(), pf))
				return ERROR;
			return 0;
		}
		
		BTNonLeafNode newNonLeaf;
		tempNonLeaf.insertAndSplit(midKey, nextPid, newNonLeaf, midKey);
		if (tempNonLeaf.write(nodePath.back(), pf) || newNonLeaf.write(pf.endPid(), pf))
			return ERROR;
		
		prevPid = nodePath.back();
		nextPid = pf.endPid() - 1;
		nodePath.pop_back();
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
	BTNonLeafNode tempNode;
	PageId currentPid = rootPid;
	int currentHeight = 0;
	if (treeHeight < 0)
	{
		return ERROR;
	}
	
	nodePath.clear();
	while (currentHeight < (treeHeight - 1))
	{
		nodePath.push_back(currentPid);
		if (tempNode.read(currentPid, pf))
			return ERROR;
		if (tempNode.locateChildPtr(searchKey, currentPid))
			return ERROR;
		currentHeight++;
	}
	
	BTLeafNode tempLeaf;
	if (tempLeaf.read(currentPid, pf))
	{
		return ERROR;
	}
	tempLeaf.locate(searchKey, cursor.eid);
	cursor.pid = currentPid;
	return 0;
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
	BTLeafNode tempNode;
	if (tempNode.read(cursor.pid, pf))
	{
		return ERROR;
	}
	
	if (cursor.eid >= tempNode.getKeyCount())
	{
		if (tempNode.getNextNodePtr() == -2)
		{
			return ERROR;
		}
		
		cursor.pid = tempNode.getNextNodePtr();
		cursor.eid = 0;
	}
	
	if (tempNode.readEntry(cursor.eid, key, rid))
		return ERROR;
	cursor.eid++;
	
	return 0;
}

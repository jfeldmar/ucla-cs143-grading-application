/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
	curLeafPid = -1;
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
	string idxFileName = indexname + ".idx";

	RC rc = pf.open(idxFileName, mode);

	if (rc)
		return rc;

	if (pf.endPid()) {
		char page[PageFile::PAGE_SIZE];

		// 'w' mode can both read and write
		pf.read(0, page);
		memcpy(&rootPid, page, sizeof(PageId));
		memcpy(&treeHeight, page+sizeof(PageId), sizeof(int));
	}

    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	curLeafPid = -1;
    rootPid = -1;
	treeHeight = 0;
	
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
	PageId pid = rootPid;

	// Tree does not exist
	if (treeHeight == 0) {
		char page[PageFile::PAGE_SIZE];

		rootPid = 1;

		memcpy(page, &rootPid, sizeof(PageId));
		memcpy(page+sizeof(PageId), &++treeHeight, sizeof(int));
		pf.write(0, page);

		BTLeafNode leafRoot;
		leafRoot.insert(key, rid);

		// Debug purpose ------------------------------------------------------
		// int indicator = -1;
		// memcpy(leafRoot.page+PageFile::PAGE_SIZE-4, &indicator, sizeof(int));
		//---------------------------------------------------------------------

		leafRoot.write(rootPid, pf);

		return 0;
	}

	RC rc = insertRecursion(key, rid, pid);

	if (rc == RC_NODE_FULL) {
		char page[PageFile::PAGE_SIZE];

		BTNonLeafNode nLeafRoot;
		nLeafRoot.initializeRoot(rootPid, key, pid);

		rootPid = pf.endPid();
		nLeafRoot.write(rootPid, pf); 

		memcpy(page, &rootPid, sizeof(PageId));
		memcpy(page+sizeof(PageId), &++treeHeight, sizeof(int));
		pf.write(0, page);

		return 0;
	}
	else
		return rc;
}


// key[IN/OUT] - key[IN] key to insert, key[OUT] midKey/siblingKey
// rid[IN]
// pid[IN/OUT] - pid[IN] childPtr, pid[OUT] child sibling pid to insert
RC BTreeIndex::insertRecursion(int& key, const RecordId& rid, PageId& pid)
{
	static int curDepth = 1;

	// Insertion at leaf level
	if (curDepth == treeHeight) {
		if (pid != curLeafPid) {
			leafNode.read(pid, pf);
			curLeafPid = pid;
		}

		curDepth = 1;

		if (!leafNode.insert(key, rid)) {
			leafNode.write(pid, pf);
			return 0;
		}
		else {
			BTLeafNode sibLeafNode;
			PageId sibPid = pf.endPid();

			leafNode.insertAndSplit(key, rid, sibLeafNode, sibPid, key);

			// Debug purpose ------------------------------------------------------
			// int indicator = -1;
			// memcpy(sibLeafNode.page+PageFile::PAGE_SIZE-4, &indicator, sizeof(int));
			//---------------------------------------------------------------------

			leafNode.write(pid, pf);
			sibLeafNode.write(sibPid, pf);

			pid = sibPid;

			return RC_NODE_FULL;
		}
	}
	// Insertion at non-leaf level
	else {	
		BTNonLeafNode nLeafNode;
		PageId nLeafPid = pid;

		nLeafNode.read(pid, pf);
		nLeafNode.locateChildPtr(key, pid);
		curDepth++;

		RC rc = insertRecursion(key, rid, pid);

		if (!rc)
			return 0;
		else {
			if (!nLeafNode.insert(key, pid)) {
				nLeafNode.write(nLeafPid, pf);
				return 0;
			}
			else {
				BTNonLeafNode sibNode;
				PageId sibPid = pf.endPid();

				nLeafNode.insertAndSplit(key, pid, sibNode, key);
				nLeafNode.write(nLeafPid, pf);
				sibNode.write(sibPid, pf);

				pid = sibPid;

				return RC_NODE_FULL;
			}
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
	int count = 1;
	cursor.pid = rootPid;	
	BTNonLeafNode nLeafNode;
	
	// NonLeaf level
	while (count < treeHeight) {
		nLeafNode.read(cursor.pid, pf);
		nLeafNode.locateChildPtr(searchKey, cursor.pid);
		count++;
	}
	
	// Leaf level
	if (cursor.pid != curLeafPid) {
		leafNode.read(cursor.pid, pf);
		curLeafPid = cursor.pid;
	}

	RC returnedValue = leafNode.locate(searchKey, cursor.eid);
	
	if (returnedValue) {
		PageId nextPid = leafNode.getNextNodePtr();

		if (nextPid == RC_INVALID_PID)
			return RC_NO_SUCH_RECORD;
		else {
			cursor.pid = nextPid;
			cursor.eid = 0;
			return 0;
		}
	}
	else 
		return 0;
}	// end of locate

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
	if (cursor.pid == RC_INVALID_PID)
		return RC_END_OF_TREE;
	else if (cursor.pid <= 0)
		return RC_INVALID_CURSOR;
	else if (cursor.pid != curLeafPid) {
		leafNode.read(cursor.pid, pf);
		curLeafPid = cursor.pid;
	}

	if (cursor.eid >= 0 && cursor.eid < leafNode.getKeyCount())
		leafNode.readEntry(cursor.eid, key, rid);
	else
		return RC_INVALID_CURSOR;

	if (++cursor.eid == leafNode.getKeyCount()) {
		cursor.pid = leafNode.getNextNodePtr();
		cursor.eid = 0;
	}

    return 0;
}

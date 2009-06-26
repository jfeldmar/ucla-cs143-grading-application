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
    data.rootPid = -1;
    data.treeHeight = 1;
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
	return pf.open(indexname, mode);
}

RC BTreeIndex::readFile()
{
	char buf[PageFile::PAGE_SIZE];
	int temp = 0;

	temp = pf.read(0, buf);

	if (temp < 0) return temp;

	memcpy(&data, buf, 1024);

	return 0;
}

RC BTreeIndex::writeFile()
{
	char buf[PageFile::PAGE_SIZE];

	memcpy(buf, (char *) &data, 1024);

	return pf.write(0, buf);
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
	int sKey = INT_MIN;
	int sPid = INT_MIN;

	BTreeIndex::insert(key, rid, data.treeHeight, data.rootPid, sKey, sPid);

	if (data.treeHeight != 1) nonLeafNode.read(data.rootPid, pf);

	if (sKey != INT_MIN)
	{
		int rPid = pf.endPid();

		nonLeafNode.initializeRoot(data.rootPid, sKey, sPid);
		data.treeHeight++;
		data.rootPid = rPid;

		nonLeafNode.write(rPid, pf);
	}

	return 0;
}

RC BTreeIndex::insert(int key, const RecordId& rid, int currentHeight, PageId currentPid, int& nKey, PageId& nPid)
{
	if (data.treeHeight == 1 && currentPid == -1)
	{
		currentPid = pf.endPid() + 1;
		data.rootPid = 1;
		leafNode.insert(key, rid);
		leafNode.write(currentPid, pf);

		return 0;
	}

	if (currentHeight == 1)
	{
		leafNode.read(currentPid, pf);

		int errCheck = leafNode.insert(key, rid);
		if (errCheck == 0)
		{
			leafNode.write(currentPid, pf);
		}
		else
		{
			int sKey = 0;
			PageId sPid = pf.endPid();

			BTLeafNode newNode;

			leafNode.insertAndSplit(key, rid, newNode, sKey);

			newNode.setNextNodePtr(leafNode.getNextNodePtr());
			leafNode.setNextNodePtr(sPid);//printf("%d %d %d\n", currentPid, sPid, newNode.getNextNodePtr());fflush(stdout);

			newNode.write(sPid, pf);
			leafNode.write(currentPid, pf);

			nKey = sKey;
			nPid = sPid;
		}

		return 0;
	}
	else
	{
		int sKey = INT_MIN;
		PageId sPid = INT_MIN;

		int childPtr = INT_MIN;

		nonLeafNode.read(currentPid, pf);

		nonLeafNode.locateChildPtr(key, childPtr);//printf("%d %d\n", childPtr, key);fflush(stdout);

		BTreeIndex::insert(key, rid, currentHeight - 1, childPtr, sKey, sPid);

		nonLeafNode.read(currentPid, pf);

		if (sKey == INT_MIN) return 0;

		if (nonLeafNode.insert(sKey, sPid) == 0)
		{
			nonLeafNode.write(currentPid, pf);
		}
		else
		{
			int ssKey = INT_MIN;
			int ssPid = pf.endPid();

			BTNonLeafNode newNode;

			nonLeafNode.insertAndSplit(sKey, sPid, newNode, ssKey);

			newNode.write(ssPid, pf);
			nonLeafNode.write(currentPid, pf);

			nKey = ssKey;
			nPid = ssPid;
		}

		return 0;
	}
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
	return BTreeIndex::locate(searchKey, cursor, data.treeHeight, data.rootPid);
}

RC BTreeIndex::locate(int searchKey, IndexCursor& cursor, int currentHeight, PageId currentPid)
{
	if (currentHeight == 1)
	{
		int eid = 0;

		leafNode.read(currentPid, pf);

		int temp = leafNode.locate(searchKey, eid);
		if (temp < 0) return temp;

		cursor.pid = currentPid;
		cursor.eid = eid;

		return 0;
	}
	else
	{
		int childPid = 0;

		nonLeafNode.read(currentPid, pf);

		int temp = nonLeafNode.locateChildPtr(searchKey, childPid);
		if (temp < 0) return temp;

		return BTreeIndex::locate(searchKey, cursor, currentHeight - 1, childPid);
	}

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
	if (cursor.pid < 0) return INT_MIN;

	int temp = 0;

	temp = leafNode.read(cursor.pid, pf);
	if (temp < 0) return temp;

	temp = leafNode.readEntry(cursor.eid, key, rid);
	if (temp < 0) return temp;

	if (cursor.eid == leafNode.getKeyCount() - 1)
	{
		cursor.pid = leafNode.getNextNodePtr();
		cursor.eid = 0;
	}
	else
	{
		cursor.eid++;
	}

    return 0;
}

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

RC BTreeIndex::recursiveLocate(int searchKey, BTNonLeafNode *parent, int curDepth, IndexCursor &cursor)
{
	if(curDepth == header.height)
	{
		parent->locateChildPtr(searchKey, cursor.pid);

		BTLeafNode leaf;
		leaf.read(cursor.pid, pf);

		return leaf.locate(searchKey, cursor.eid);
	}

	PageId nextParentPage;
	parent->locateChildPtr(searchKey, nextParentPage);

	BTNonLeafNode nextParent;
	nextParent.read(nextParentPage, pf);

	return recursiveLocate(searchKey, &nextParent, curDepth + 1, cursor);
}

RC BTreeIndex::recursiveInsert(int key, const RecordId &rid, BTNonLeafNode *parent, int curDepth, int &newKey, PageId &newPID)
{
	if(curDepth == header.height) // leaf level
	{
		PageId destPage;
		parent->locateChildPtr(key, destPage);

		BTLeafNode dest;
		dest.read(destPage, pf);

		if(dest.insert(key, rid) == RC_NODE_FULL) // need to split
		{
			//cout << "recursive split leaf " << key << endl;
			//cout << "contents: " << dest.data[0].key << ", " << dest.data[1].key << endl;
			BTLeafNode newNode;
			newPID = header.nPages++;

			dest.insertAndSplit(key, rid, newNode, newKey);
			newNode.setNextNodePtr(dest.getNextNodePtr());
			dest.setNextNodePtr(newPID);

			//cout << "new contents: " << dest.data[0].key << ", " << dest.data[1].key << endl;

			dest.write(destPage, pf);
			newNode.write(newPID, pf);
			return RC_NODE_FULL;
		}

		dest.write(destPage, pf);

		return 0;
	}

	PageId nextParentPage;
	parent->locateChildPtr(key, nextParentPage);

	BTNonLeafNode nextParent;
	nextParent.read(nextParentPage, pf);

	if(recursiveInsert(key, rid, &nextParent, curDepth+1, newKey, newPID) == RC_NODE_FULL)
	{
		//cout << parent->header.nKeys << endl;
		//cout << "aha :" << nextParent.header.nKeys << endl;
		// split the node, now what?
		if(nextParent.insert(newKey, newPID) == RC_NODE_FULL) // need to split this one too
		{
			BTNonLeafNode newNode;
			nextParent.insertAndSplit(newKey, newPID, newNode, newKey);

			newPID = header.nPages++;

			nextParent.write(nextParentPage, pf);
			newNode.write(newPID, pf);

			return RC_NODE_FULL;
		}

		// we were fine adding the new key, so just write it back
		nextParent.write(nextParentPage, pf);
	}

	// Didn't have to split the child node, so we're all good

	return 0;
}

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    header.root = END_OF_FILE;
    header.height = 0;
    header.nPages = 1;

    writing = false;
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
	pf.open(indexname, mode);

	if(mode == 'r' || mode == 'R')
		return read();
	else if(mode == 'w' || mode == 'W')
	{
		writing = true;
		return 0;
	}

    return RC_INVALID_FILE_MODE;
}

RC BTreeIndex::read()
{
	char *buffer = new char[PageFile::PAGE_SIZE];

	RC err = pf.read(0, buffer);

	if(err)
	{
		delete [] buffer;
		return err;
	}

	memcpy(&header, buffer, sizeof(BTIndexHeader));

	return 0;
}

RC BTreeIndex::write()
{
	char *buffer = new char[PageFile::PAGE_SIZE];

	memcpy(buffer, &header, sizeof(BTIndexHeader));

	RC err = pf.write(0, buffer);

	delete [] buffer;
	return err;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	if(writing)
		write();

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
	if(empty())
		initRoot(key, rid);

	int    newKey; // may be needed if the root gets split!
	PageId newPID;

	if(header.height == 1) // leaf root
	{
		BTLeafNode root;
		root.read(header.root, pf);
		if(root.insert(key, rid) == RC_NODE_FULL)
		{
			BTLeafNode newNode;
			newPID = header.nPages++;

			root.insertAndSplit(key, rid, newNode, newKey);
			newNode.setNextNodePtr(root.getNextNodePtr());
			root.setNextNodePtr(newPID);

			root.write(header.root, pf);
			newNode.write(newPID, pf);

			BTNonLeafNode newRoot;
			newRoot.initializeRoot(header.root, newKey, newPID);
			header.root = header.nPages++;
			newRoot.write(header.root, pf);
			header.height++;

			return 0;
		}

		root.write(header.root, pf);

		return 0;
	}

	BTNonLeafNode root;
	root.read(header.root, pf);

	if(recursiveInsert(key, rid, &root, 2, newKey, newPID) == RC_NODE_FULL)
	{
		//cout << "root insert " << newKey << ", " << newPID << endl;
		if(root.insert(newKey, newPID) == RC_NODE_FULL)
		{
			// time to split the root!
			BTNonLeafNode newNode;
			root.insertAndSplit(newKey, newPID, newNode, newKey);

			newPID = header.nPages++;

			// write old root and new node as first two children of new root
			root.write(header.root, pf);
			newNode.write(newPID, pf);

			BTNonLeafNode newRoot;
			newRoot.initializeRoot(header.root, newKey, newPID);
			header.root = header.nPages++;
			newRoot.write(header.root, pf);

			header.height++;

			return 0;
		}

		// OK, just write out root
		root.write(header.root, pf);
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
	if(empty())
		return RC_NO_SUCH_RECORD;

	if(header.height == 1) // leaf root
	{
		BTLeafNode root;
		root.read(header.root, pf);
		cursor.pid = header.root;

		return root.locate(searchKey, cursor.eid);
	}

	BTNonLeafNode root;
	root.read(header.root, pf);

	return recursiveLocate(searchKey, &root, 2, cursor);
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
	BTLeafNode node;
	node.read(cursor.pid, pf);

	if(cursor.eid >= node.getKeyCount()) // past the end
	{
		cursor.eid = 0;
		cursor.pid = node.getNextNodePtr();

		if(cursor.pid == END_OF_FILE)
			return RC_END_OF_TREE;

		node.read(cursor.pid, pf);
	}

	node.readEntry(cursor.eid++, key, rid);
    return 0;
}

void BTreeIndex::initRoot(int key, const RecordId &rid)
{
	BTLeafNode root;

	header.root = header.nPages++;
	header.height = 1;

	root.write(header.root, pf);
}

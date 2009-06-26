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
#include <fstream>
#include <iostream>

using namespace std;

#define	DEBUG

// BTreeIndex constructor
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
	#ifdef DEBUG
	cout << "BTreeIndex::open" << endl;
	#endif
	RC err;
	// call pagefile's open function
	err = pf.open(indexname + ".idx", 'w');
	// check for error
	if (err != 0)
		return err;
	// buffer to hold page data
	char* buff = new char[1024];
	
	// if the index was just created
	if (pf.endPid() == 0)
	{
		// set the root node pid
		*buff = 1;
		// set the current node pid
		*(buff+1023) = 0;
		// write the page to the page file
		err = pf.write(0, buff);
	}
	
	// read a page
	err = pf.read(0, buff);
	// get the root pid
	char c = *buff;
	// and store it
	rootPid = c;
	// deallocate the memory
	delete [] buff;
	// return result
    return err;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	#ifdef DEBUG
	cout << "BTreeIndex::close" << endl;
	#endif
	RC err;
	// make a buffer
	char* buff = new char[1024];
	err = pf.read(0, buff);
	// check for error
	if (err != 0)
		return err;
	// set the first byte of page to the root pid
	char c = rootPid;
	*buff = c;
	// write the page back to the file
	err = pf.write(0, buff);
	// deallocate the memory
	delete [] buff;
	// check for error
	if (err != 0)
		return err;
	// close the file
	err = pf.close();
	// return result
    return err;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	RC err;
	#ifdef DEBUG
	cout << "BTreeIndex::insert" << endl;
	#endif
	// instanciate variables
	BTLeafNode b1, b2;
	BTLeafNode *node = &b1;
	BTLeafNode *edon = &b2;
	double arr[20];
	IndexCursor index;
	index.pid = rootPid;
	int savedPid;
	int skey;
	
	// find the node that should contain the key
	err = locate(key, index);
	// check for error
	if (err != 0) return err;
	
	// save the pid
	savedPid = index.pid;
	
	// get the node
	err = pf.read(index.pid, node);
		
	// if there was a problem
	if (err != 0) return err;
	
	// try to insert the key into the node
	err = node->insert(key, rid);
	// if that fails
	if (err == -1010)
	{
		// get the first unused page id
		int end = pf.endPid();
		// call insert and split on the node
		err = node->insertAndSplit(key, rid, *edon, skey);
			
		// store the newly updated nodes
		err = pf.write(index.pid, node);
		
		// if there was an error
		if (err != 0)
			return err;
		err = pf.write(end, edon);
		return err;
	}
	// if the record was inserted without trouble
	// save the node
	err = pf.write(savedPid, node);
	// return what that returned
    return err;
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
	#ifdef DEBUG
	cout << "BTreeIndex::locate" << endl;
	#endif
	RC err;
	// instanciate node pointers
	BTNonLeafNode b1;
	BTLeafNode b2;
	BTNonLeafNode* node = &b1;
	BTLeafNode* edon = &b2;
	double arr[20];
	// instanciate a page id variable
	int pageid, eid;
	// get the root node
	err = pf.read(rootPid, node);
	
	// if the root.keyCount = 0, the index has just been initialized
	if (node->getKeyCount() == 0)
	{
		// store the root node as a leaf node
		err = pf.write(1, edon);
		// return root node information
		cursor.pid = 1;
		cursor.eid = 0;
		return 0;
	}
	
	// check if the root is a leaf node
	if (!(node->nonLeaf))
	{
		// get the root node as a leaf node
		err = pf.read(rootPid, edon);
		// check for error
		if (err != 0)
			return err;
		// call leaf node's locate function
		err = edon->locate(searchKey, eid);
		// check for error
		if (err != 0)
			return err;
		// set the return parameters
		cursor.pid = 1;
		cursor.eid = eid;
		return 0;
	}
	// call node's locate child pointer function
	err = node->locateChildPtr(searchKey, pageid);
	
	// call the recursive locate to do the real work
	err = locateRecursive(searchKey, pageid, eid);
	// check for error
	if (err != 0)
		return err;
	
	// set the output parameters
	cursor.pid = pageid;
	cursor.eid = eid;
	// return
    return 0;
}

// this function locates a node recursively
// @param key[IN] the key to find
// @param pid[OUT] the page id of the key we're locating
// @param eid[OUT] the entry number of the key
RC BTreeIndex::locateRecursive(int key, int& pid, int& eid)
{
	#ifdef DEBUG
	cout << "BTreeIndex::locateRecursive" << endl;
	#endif
	RC err;
	// instanciate node pointers
	BTNonLeafNode b1;
	BTLeafNode b2;
	BTNonLeafNode* node = &b1;
	BTLeafNode* edon = &b2;
	double arr[20];
	// get the node
	err = pf.read(pid, node);
	// check for error
	if (err != 0)
		return err;
	// check if the node is a leaf
	if (node->nonLeaf)
	{
		// now we know we have a non leaf node
		err = node->locateChildPtr(key, pid);
		// check for error
		if (err != 0)
			return err;
		// call locate recursive with the new pid
		err = locateRecursive(key, pid, eid);
		// return
		return err;
	}
	// if it's a leaf node, read the page into a leaf node
	err = pf.read(pid, edon);
	// check for error
	if (err != 0)
		return err;
	// just to make sure this is a leaf node
	if (edon->nonLeaf)
		return -1;
	// call the leaf node's locate function
	err = edon->locate(key, eid);

	// return, finally
	return err;
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
	#ifdef DEBUG
	cout << "BTreeIndex::readForward" << endl;
	#endif
	RC err;
	// instanciate a leaf node pointer and a leaf tuple
	BTLeafNode b;
	BTLeafNode* node = &b;
	double arr[20];
	leafTuple foo;
	// read a page from the page file into node
	err = pf.read(cursor.pid, node);
	// check for error
	if (err != 0)
		return err;
	// get the key, rid from the node
	node->readEntry(cursor.eid, key, rid);
	// if that was the last entry
	if (cursor.eid == node->getKeyCount()-1)
	{
		// increment the cursor.pid
		cursor.pid += 1;
		// reset the cursor.eid to 0
		cursor.eid = 0;
		// return
		return 0;
	}
	// if there are more entries in node, increment the cursor.eid
	cursor.eid += 1;
	// return
    return 0;
}

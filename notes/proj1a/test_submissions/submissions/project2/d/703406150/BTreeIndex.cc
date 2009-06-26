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
#include <string>

using namespace std;

//#define	DEBUG

// Turns out I don't need this
// parse the index name to remove the extension
// if there is no extension, does nothing
void BTreeIndex::parse(string &s)
{
	// create a new empty string
	string t = "";
	// look at each letter
	for (int i = 0; i < s.length(); i++)
	{
		// if it's not a '.'
		if (s[i] != '.')
			// add it to t
			t += s[i];
		// if it is a dot, we're done
		else
		{
			// set s to t
			s = t;
			// return
			return;
		}
	}
}

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
	// remove the file entension, if it exists
	//string str = indexname;
	//parse(str);
	// call pagefile's open function
	err = pf.open(indexname + ".idx", mode);
	// check for error
	if (err != 0) return err;
	// buffer to hold page data
	int* buff = new int[256];
	
	// if the index was just created, write overhead information
	if (pf.endPid() == 0)
	{
		#ifdef DEBUG
		cout << "BTreeIndex::open - pf.endPid() == 0" << endl;
		#endif
		// set the root node pid
		buff[0] = 1;
		// set the current node pid
		buff[255] = 0;
		// write the page to the page file
		err = pf.write(0, buff);
	}
	// if the index already exists
	// get the overhead information
	err = pf.read(0, buff);
	// get the root pid
	rootPid = buff[0];
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
	int* buff = new int[256];
	// read a page
	err = pf.read(0, buff);
	// check for error
	if (err != 0) return err;
	
	// set the first int of page to the root pid
	buff[0] = rootPid;
	// reset the current node entry
	buff[255] = 0;
	
	// write the page back to the file
	err = pf.write(0, buff);
	// deallocate the memory
	delete [] buff;
	// check for error
	if (err != 0) return err;
	
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
	#ifdef DEBUG
	cout << "BTreeIndex::insert" << endl;
	#endif
	RC err;
	// create nodes
	BTNonLeafNode node, node2, node3;
	BTLeafNode edon, edon2;
	// variables
	int pid, eid, key2, key3;
	
	// find the location of the rid matching the search key
	err = locate(key, pid, eid);
	// check for error
	if (err != 0) return err;
	
	// get the node
	err = edon.read(pid, pf);
	// check for error
	if (err != 0) return err;
	// make sure it's a leaf node
	if (edon.isLeaf() != 1) return -1;
	
	// try to insert the key, rid pair into the node
	err = edon.insert(key, rid);
	// if the leaf node is full
	if (err == -1010)
	{
		// get the first unused page id
		int end = pf.endPid();
		// call insert and split on the leaf node
		err = edon.insertAndSplit(key, rid, edon2, key2);
		// check for error
		if (err != 0) return err;
		
		// set the new node's sibling pid
		err = edon2.setSibPid(edon.getSibPid());
		// check for error
		if (err != 0) return err;
		
		// set the old node's sibling pid
		err = edon.setSibPid(end);
		// check for error
		if (err != 0) return err;
		
		// set the new node's parPid to the old node's parPid
		edon2.setParPid(edon.getParPid());
		
		// if we had to split the root node when it was a leaf node
		if (pid == rootPid)
		{
			// get the next unused page id
			int end2 = end + 1;
			// set the new rootPid
			rootPid = end2;
			// initialize the new root node
			err = node.initializeRoot(pid, key2, end);
			// check for error
			if (err != 0) return err;
			
			// set the new parent pids
			edon.setParPid(end2);
			edon2.setParPid(end2);
			
			// store the new root node
			err = node.write(end2, pf);
			// check for error
			if (err != 0) return err;
		}
		// if we had to split a non-root leaf node
		else
		{
			// get the parent non leaf node
			err = node.read(edon.getParPid(), pf);
			// check for error
			if (err != 0) return err;
			
			// try to insert the midkey into the parent non leaf node
			err = node.insert(key2, end);
			// if that non leaf node is full
			if (err == -1010)
			{
				// get the next unused page id
				int end2 = end + 1;
				// call insert and split on the non leaf node
				err = node.insertAndSplit(key2, end, node2, key3);
				// check for error
				if (err != 0) return err;
				
				// change all the new node's children's parent pids
				err = node2.setChildrenParPids(end2, pf);
				// check for error
				if (err != 0) return err;
				
				// save the new sibling node
				err = node2.write(end2, pf);
				// check for error
				if (err != 0) return err;
				
				// check if we just split the root node
				if (edon.getParPid() == rootPid)
				{
					// get the next unused page id
					int end3 = end2 + 1;
					// set the new rootPid
					rootPid = end2;
					// initialize the new root
					node3.initializeRoot(edon.getParPid(), key3, end2);
					// save the new root node
					err = node3.write(end3, pf);
					// check for error
					if (err != 0) return err;
				}
				// if the non leaf node we just split wasn't the root
				else
				{
					// get the parent non leaf node
					err = node3.read(node.getParPid(), pf);
					// check for error
					if (err != 0) return err;
					
					// insert the midkey into the parent
					err = node3.insert(key3, end2);
					// check for error
					if (err != 0) return err;
					
					// save the parent non leaf node
					err = node3.write(node.getParPid(), pf);
					// check for error
					if (err != 0) return err;
				}
			}
			// check for other insertion problems
			else if (err != 0) return err;
			// if there was no problem inserting
			else
			{
				// save the parent node
				err = node.write(edon.getParPid(), pf);
				// check for error
				if (err != 0) return err;
			}
		}
		// store the newly updated nodes
		err = edon.write(pid, pf);
		err = edon2.write(end,pf);
		// return
		return err;
	}
	// check for other insertion problems
	else if (err != 0) return err;
	// if the entry was inserted successfully
	else err = edon.write(pid, pf);
	// return
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
 * @param pid[OUT] the pid of the first index entry >= the key value
 * @param eid[OUT] the eid of the first index entry >= the key value
 * @return error code. 0 if no error.
 */
RC BTreeIndex::locate(int searchKey, int& pid, int& eid)
{
	#ifdef DEBUG
	cout << "BTreeIndex::locate" << endl;
	#endif
	// create variables
	RC err;
	
	// create nodes
	BTNonLeafNode node;
	BTLeafNode edon;
	
	// check if the index has just beem initialized
	if (pf.endPid() == 1)
	{
		// store the root node as a leaf node
		edon.write(1, pf);
		// return first entry of root node
		pid = 1;
		return 0;
	}
	// if there is at least one entry already in the index
	// get the root node as a non leaf node
	err = node.read(rootPid, pf);
	// check for error
	if (err != 0) return err;
		
	// check if the root is actually a leaf node
	if (node.isLeaf())
	{
		// get the node as a leaf node
		err = edon.read(rootPid, pf);
		if (err != 0) return err;
		
		// find the first entry number >= key
		err = edon.locate(searchKey, eid);
		if (err != 0) return err;
		
		// since there is only one node, return the rootPid, which will be 1
		pid = 1;
		return 0;
	}
	// if the root node is a non leaf node
	// call node's locate child pointer function
	err = node.locateChildPtr(searchKey, pid);
	// check for error
	if (err != 0) return err;
	
	// call the recursive locate to do the real work
	err = locateRecursive(searchKey, pid, eid);
	// return
	return err;
}

// Locates a leaf node recursively
// @param key[IN] the key to find
// @param pid[OUT] the page id of the key we're locating
// @param eid[OUT] the entry number of the key
RC BTreeIndex::locateRecursive(int key, int& pid, int& eid)
{
	#ifdef DEBUG
	cout << "BTreeIndex::locateRecursive" << endl;
	#endif
	RC err;
	// create nodes
	BTNonLeafNode node;
	BTLeafNode edon;
	
	// get the node as a nonleaf
	err = node.read(pid, pf);
	// check for error
	if (err != 0) return err;
	
	// check if the node is actually a leaf, if it is, we're done
	if (node.isLeaf() == 1)
	{
		// get the node as a leaf node
		err = edon.read(pid, pf);
		// call the leaf node's locate function
		err = edon.locate(key, eid);
		// return
		return err;
	}
	// make sure the node is actually a non leaf
	if (node.isLeaf() == 0)
	{
		// find the child to follow
		err = node.locateChildPtr(key, pid);
		// check for error
		if (err != 0) return err;
		
		// call locate recursive with the new pid
		err = locateRecursive(key, pid, eid);
		// return
		return err;
	}
	// should never happen
	return -1;
}

/* 
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param pid[IN] the pid of the first index entry >= the key value
 * @param eid[IN] the eid of the first page entry >= the key value 
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(int& pid, int& eid, int& key, RecordId& rid)
{
	#ifdef DEBUG
	cout << "BTreeIndex::readForward" << endl;
	#endif
	RC err;
	// create a leaf node pointer and a leaf tuple
	BTLeafNode node;
	leafTuple stu;
	
	// get the node
	err = node.read(pid, pf);
	// check for error
	if (err != 0) return err;
	
	// get the key, rid from the node
	node.readEntry(eid, key, rid);
	// if that was the last entry
	if (eid == (node.getKeyCount()-1))
	{
		// if this node has a sibling
		if (node.getSibPid() != -1)
		{
			// set the new pid
			pid = node.getSibPid();
		}
		// if this node was the last
		else return -1;
		// reset the cursor.eid to 0
		eid = 0;
		// return
		return 0;
	}
	// if there are more entries in node, increment the cursor.eid
	eid += 1;
	// return
    return 0;
}

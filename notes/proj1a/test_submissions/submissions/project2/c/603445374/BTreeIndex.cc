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
RC BTreeIndex::open(const string& indexname, char mode) {
	RC   rc;
	char page[PageFile::PAGE_SIZE];

	// open the index file
	if ((rc = pf.open(indexname, mode)) < 0) return rc;

	// if index is empty
	if (rootPid == -1) {	// if rootPid is -1, first time this index is being opened
		rootPid = 0;	//set root node pid to 0
		//BTLeafNode root;	//create original leaf root node, key count is set at 0 by leafnode constructor
		//treeHeight = 1;	// since root node has been created, treeHeight becomes 1
		return 0;
	}
	// if index is not empty
	else {
		//reconstruct root and treeheight vars
		//read  all nodes into memory for searching
	}
	return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close() {
	//store values of rootPid and treeHeight somewhere in disk so they can be reconstructed when the index is opened again later
	//how do we do that? what does that mean?
	return pf.close();
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid) {
	RC rc;
	
	//if first key ever inserted into index, must initializeRoot
	if (treeHeight == 0) {
		cout << "treeHeight: " << treeHeight << endl;
		BTLeafNode root;
		root.read(rootPid, pf);	//read new root into memory to allow insertion
		root.insert(key, rid);	//insert 1st key into root
		root.write(rootPid, pf);	//write new root leaf node w/ key inserted back to disk
		treeHeight = 1;		//set treeHeight to 1
		cout << "treeHeight: " << treeHeight << endl;
	}
	
	//otherwise, locate the leaf node to insert into using locate()
	else {
		IndexCursor cursor;	//for referring to the leaf node
		locate(key, cursor);
		//try to insert (key, rid) at that indexCursor
		BTLeafNode tempLeaf;
		tempLeaf.read(cursor.pid, pf);	//read leaf node into memory for manipulation
		if ( rc = tempLeaf.insert(key, rid) == RC_NODE_FULL) {
			//if leaf node is full
			//leaf node overflow -> split leaf node in half, copy first key of sibling node to parent non-leaf node
			//non-leaf node overflow -> split node in half, move middle key to parent
			//root node overflow -> split node in half, create new root node using initializeRoot(), move middle key to new root node
				//increment tree height
		}
		//if leaf node is not full, pair just inserted into leaf
		tempLeaf.write(cursor.pid, pf);	//write edited leaf node back onto disk
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
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor) {
	//IndexCursor = (PageId pid, int eid)
	int compKey; //key to compare with searchKey
	int compHeight = 1; //height to compare with treeHeight, start at 1 for root
	PageId curPid = rootPid; //pid to go through each level of nodes; start at root
	int eidTemp;
	BTNonLeafNode NLtemp;
	BTLeafNode Ltemp;
	
	//ALGORITHM
	//traverse down the tree to find the leaf-node index entry whose key value is >= searchKey using readForward()
	//traverse by going through each node until you reach a key that is >= searchKey, then go to following pid
	//while traversing, increment a height counter and store nodes visited in an array
	//once traversal hits treeHeight, the leaf node level has been hit. stop on that node and return the indexCursor
	
	while (compHeight < treeHeight) {
		if (compHeight == treeHeight) {	//current node is a leaf
			Ltemp.read(curPid, pf);	//read leaf node into memory to allow searching
			Ltemp.locate(searchKey, eidTemp);	//get eid of entry whose key is >= searchKey
			break;
		}
		//current node is a non-leaf
		NLtemp.read(curPid, pf);	//read current node into memory to allow searching
		NLtemp.locateChildPtr(searchKey, curPid);	//get childNode with key >= searchKey
		compHeight++;	//increment height count
	}
	//set pid and eid of cursor
	cursor.pid = curPid;
	cursor.eid = eidTemp;
	
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
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid) {
	RC rc;
	BTLeafNode temp;
	
	//read the leafnode into memory using the pid in indexcursor
	if (rc = temp.read(cursor.pid, pf) != 0) {
		return rc;
	}
	
	//set the key and rid using readEntry() with the eid in indexcursor
	if (rc = temp.readEntry(cursor.eid, key, rid) != 0) {
		return rc;
	}
	
	//increment the eid and return
	cursor.eid++;	
	
	//what happens if the eid hits the end of the node?
    return 0;
}

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
    rootPid = 1;
	treeHeight = 0;
	tracePos = 0;
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

	// open the index file
	if ((rc = pf.open(indexname, mode)) < 0) return rc;
	
	// if index is empty
	if (pf.endPid() == 0) {	// if endPid() is 0, first time this index is being opened
		rootPid = 1;	//set root node pid to 1 since 0 is used to store rootPid and treeHeight
		return 0;
	}
	// if index is not empty
	else {
		//reconstruct rootPid and treeHeight
		char buffer[1024];
		if (rc = pf.read(0, buffer) < 0) return rc;	//read the content of the 1st page of pf into buffer
		memcpy(&rootPid, &buffer, sizeof(int));	//set rootPid
		memcpy(&treeHeight, &(buffer[4]), sizeof(int));	//set treeHeight
	}
	return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close() {
	//store values of rootPid and treeHeight in the 1st page of pf for reconstruction later
	RC rc;
	
	//write rootPid and treeHeight to a temporary buffer
	char buffer[1024];
	memcpy(&buffer, &rootPid, sizeof(int));
	memcpy(&buffer[4], &treeHeight, sizeof(int));
	
	//write the buffer to the 1st page of pf
	if (rc = pf.write(0, buffer) < 0) return rc;
	
	//close pf
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
	//if first key ever inserted into index, must create initial root leaf node
	if (treeHeight == 0) {
		BTLeafNode root;
		root.read(rootPid, pf);	//read new root into memory to allow insertion
		root.insert(key, rid);	//insert 1st key into root
		root.setNextNodePtr(-1);	//set end of root node as NULL
		root.write(rootPid, pf);	//write new root leaf node w/ key inserted back to disk
		treeHeight = 1;		//set treeHeight to 1
	}
	
	//otherwise, locate the leaf node to insert into using locate()
	else {
		int tempHeight = treeHeight;
		
		//locate the leaf node to insert into
		IndexCursor cursor;	//for referring to the leaf node
		locate(key, cursor);

		//try to insert (key, rid) at that indexCursor
		BTLeafNode tempLeaf;
		tempLeaf.read(cursor.pid, pf);	//read leaf node into memory for manipulation

		//BEGIN INSERTION INTO LEAF
		if ( rc = tempLeaf.insert(key, rid) == RC_NODE_FULL ) {
			//LEAF NODE OVERFLOW
			//insert and split the leaf node
			BTLeafNode leafSibNode;
			int leafSibKey;
			PageId sibPid = pf.endPid();
			leafSibNode.read(sibPid, pf);	//read the new sibling node into buffer
			tempLeaf.insertAndSplit(key, rid, leafSibNode, leafSibKey);
			//set new ptrs between leaf nodes that were split
			PageId tempPid = tempLeaf.getNextNodePtr();
			leafSibNode.setNextNodePtr(tempPid);
			tempLeaf.setNextNodePtr(sibPid);

			//write nodes back to disk
			tempLeaf.write(cursor.pid, pf);
			leafSibNode.write(sibPid, pf);
			
			//CASE 1: root is leaf node, overflow, must split and create new nonleaf root
			if (tracePos == 0) {
				//create new nonleaf root node
				BTNonLeafNode newRoot;
				rootPid = pf.endPid();	//set new rootPid to the newly created root
				newRoot.read(rootPid, pf);	//read new root node into memory for manipulation
				
				//create new root node using initializeRoot()
				//original node's pid = cursor.pid
				//sibling node's pid = sibPid
				newRoot.initializeRoot(cursor.pid, leafSibKey, sibPid);
				
				//write new root node back to disk
				newRoot.write(rootPid, pf);
				
				treeHeight++;	//increment treeHeight
			}
			
			//CASE 2: parent is nonleaf, so just insert new sibling key into non-leaf parent
			if (tracePos > 0) {
				//get nonleaf parent node
				BTNonLeafNode NLParent;
				tracePos--;	//to get the index position of the parent node in trace
				PageId parentPid = trace[tracePos];	//get the pid of parent node
				NLParent.read(parentPid, pf);	//read parent node into buffer for manipulation
				
				//insert new key and pid into nonleaf parent
				if (rc = NLParent.insert(leafSibKey, sibPid) < 0) {
					//NONLEAF OVERFLOW
					//CASE 3: nonleaf root node overflow, must create new root,
					
					//Split current NL Parent Node
					//create new nonleaf root node
					BTNonLeafNode NLSibling;
					PageId NLSibPid = pf.endPid();
					NLSibling.read(NLSibPid, pf);	//read new sibling node into memory
					int NLMidKey;
					NLParent.insertAndSplit(leafSibKey, sibPid, NLSibling, NLMidKey);
					NLSibling.write(NLSibPid, pf);
					
					if (tracePos == 0) {
						//Are at top level, need to create new nonleaf root node
						BTNonLeafNode newRoot2;
						rootPid = pf.endPid();	//set new rootPid to the newly created root
						newRoot2.read(rootPid, pf);	//read new root node into memory for manipulation

						//create new root node using initializeRoot()
						//original node's pid = cursor.pid
						//sibling node's pid = NLSibPid
						newRoot2.initializeRoot(parentPid, NLMidKey, NLSibPid);
						
						//write nodes back to disk
						newRoot2.write(rootPid, pf);
						
						treeHeight++;	//increment tree height
					}
					if (tracePos > 0) {
						//already has a nonleaf parent root node, can just insert
						
						//get nonleaf parent node
						BTNonLeafNode NLParent2;
						tracePos--;	//to get the index position of the parent node in trace
						PageId parentPid2 = trace[tracePos];	//get the pid of parent node
						NLParent2.read(parentPid2, pf);	//read parent node into buffer for manipulation
						
						//insert (key, pid) into parent nonleaf root node
						NLParent2.insert(NLMidKey, NLSibPid);
						
						NLParent2.write(parentPid2, pf);	//write back to disk
					}
				}
				
				//write all nodes back to disk
				NLParent.write(parentPid, pf);
				tempLeaf.write(cursor.pid, pf);
				leafSibNode.write(sibPid, pf);
			}
		}
		//cout << "insertion was successful" << endl;
		else {
			//if leaf node is not full, pair was inserted into leaf
			tempLeaf.write(cursor.pid, pf);	//write edited leaf node back onto disk
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
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor) {
	RC rc;
	//IndexCursor = (PageId pid, int eid)
	int compKey; //key to compare with searchKey
	int compHeight = 1; //height to compare with treeHeight, start at 1 for root
	int eidTemp;	//temp eid holder

	BTNonLeafNode NLtemp;	//temporary node for traversing nonleaf nodes
	BTLeafNode Ltemp;		//temporary node for traversing leaf nodes
	PageId curPid = rootPid; //pid to go through each level of nodes; start at root
	
	trace[0] = rootPid;		//set the 1st node visited to root node
	tracePos = 0;	//index position for array that stores visited node pid's
	
	//ALGORITHM
	//traverse down the tree to find the leaf-node index entry whose key value is >= searchKey using readForward()
	//traverse by going through each node until you reach a key that is >= searchKey, then go to following pid
	//while traversing, increment a height counter and store nodes visited in an array
	//once traversal hits treeHeight, the leaf node level has been hit. stop on that node and return the indexCursor

	while (compHeight < treeHeight+1) {
		if (compHeight == treeHeight) {	//current node is a leaf
			if (rc = Ltemp.read(curPid, pf) < 0) return rc;	//read leaf node into memory to allow searching
			if (rc = Ltemp.locate(searchKey, eidTemp) < 0) return rc;	//get eid of entry whose key is >= searchKey
			break;
		}
		//current node is a non-leaf
		if (rc = NLtemp.read(curPid, pf) < 0) return rc;	//read current node into memory to allow searching
		if (rc = NLtemp.locateChildPtr(searchKey, curPid) < 0) return rc;	//get childNode with key >= searchKey
		
		compHeight++;	//increment height count
		tracePos++;	//increment trace count
		trace[tracePos] = curPid;	//store node visited
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
	if (rc = temp.read(cursor.pid, pf) < 0) return rc;
	//set the key and rid using readEntry() with the eid in indexcursor
	if (rc = temp.readEntry(cursor.eid, key, rid) != 0) return rc;
	//if eid is at the end, readEntry doesn't know and just grabs the next value even though it's garbage
	
	//increment the eid and return
	cursor.eid++;
	
	if (cursor.eid >= 83 || cursor.eid >=temp.getKeyCount()) {	//hit last possible entry in current page
		//set it to the beginning of the next leaf node page
		cursor.pid = temp.getNextNodePtr();
		if (cursor.pid == -1) {
			return -1;
		}
		cursor.eid = 0;
	}
    return 0;
}

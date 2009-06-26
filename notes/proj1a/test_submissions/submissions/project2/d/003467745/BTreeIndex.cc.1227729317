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

#include <stack>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

/**
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
	// Initialize the variables
    rootPid = -1;
	treeHeight = 0;
	count = 0;

	// Just for debugging
	if (DEBUG)
	{
		// This empties this file for the debug output
		ofstream myfile ("output.txt");
		myfile.close();
	}
}

/**
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
    int error_code = pf.open(indexname, mode);
	if ( error_code != 0 )
		return error_code;

	// Check if this is a new file
	int buffer[BLOCK_SIZE/sizeof(int)];
	if ( pf.endPid() == 0 )
	{
		treeHeight = 0;
		rootPid = -1;
		
		// Store rootPid and treeHeight to disk
		pf.read(0, buffer);
		buffer[0] = rootPid;
		buffer[1] = treeHeight;
		pf.write(0, buffer);
	}

	// Retrieve the treeHeight and rootPid from disk
	pf.read(0, buffer);
	rootPid = buffer[0];
	treeHeight = buffer[1];

	return 0;
}

/**
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	// Store rootPid and treeHeight to disk
	int buffer[BLOCK_SIZE/sizeof(int)];
	pf.read(0, buffer);
	buffer[0] = rootPid;
	buffer[1] = treeHeight;
	pf.write(0, buffer);

	// Close the file
    return pf.close();
}

/**
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	// Create a stack to remember how we traversed the B+ tree
	stack<PageId> treePath;

	// This function works on a case-by-case basis
	// There are 5 possible cases, some of which carry-over into the
	// next case depending on its state. The cases are:
	// 1) No Root Node - the tree does not exist yet
	// 2) Simple Case - insert into an available slot in a leaf node
	// 3) Leaf Overflow - insert into a full leaf, requires split
	// 4) NonLeaf Overflow - insert into a full non-leaf, requires split(s)
	// 5) Root Overflow - insert into a full root, requires split and a new root node

	// No Root Node Case
	if ( rootPid == -1 )
	{
		if (DEBUG)
			cerr << "NO ROOT NODE" << endl;

		// Create the root node
		BTNonLeafNode rootNode;
		rootPid = pf.endPid();
		rootNode.write(rootPid, pf);
		
		// Create 2 leaf nodes
		BTLeafNode leafNode1;
		BTLeafNode leafNode2;
		PageId leafNodePid1 = pf.endPid();
		leafNode1.write(leafNodePid1, pf);
		PageId leafNodePid2 = pf.endPid();
		leafNode2.write(leafNodePid2, pf);

		// Connect the nodes
		leafNode1.setNextNodePtr(leafNodePid2);
		leafNode1.write(leafNodePid1, pf);

		// Store the children in the root node
		rootNode.initializeRoot(leafNodePid1, key, leafNodePid2);
		rootNode.write(rootPid, pf);

		// Increment Tree Height
		++treeHeight;

		++count;

		// Store rootPid and treeHeight to disk
		int buffer[256];
		pf.read(0, buffer);
		buffer[0] = rootPid;
		buffer[1] = treeHeight;
		pf.write(0, buffer);

		// Now move on to the Simple Case: Insert a (key,rid) pair into an empty leaf node
	}

	// Simple Case
	// The leaf node has an available slot open
	if (DEBUG)
		cerr << "SIMPLE CASE" << endl;

	/* STEPS:
	 * 1) Go through the nonleaf nodes and find the leaf node we want to insert into
	 * 2) Check if there is enough room
	 * 3) Insert the value into the availble space in the leaf node
	 */

	// Create a temporary non-leaf node
	BTNonLeafNode nonLeafNode;
	PageId nextPid = rootPid;

	// Follow each node until we get to a leaf node
	for (int i = 0; i < treeHeight; i++)
	{
		treePath.push(nextPid);
		nonLeafNode.read(nextPid, pf);
		nonLeafNode.locateChildPtr( key, nextPid);
	}

	// nextPid points to the child node where the key belongs

	BTLeafNode leafNode;
	leafNode.read( nextPid, pf);
	if ( leafNode.getKeyCount() < leafNode.MAX_KEYS )
	{
		// Simple Case:
		leafNode.insert( key, rid);
		leafNode.write(nextPid, pf);
		++count;

		// return success
		return 0;
	}


	// Leaf overflow
	// The leaf node you're trying to insert into is full
	if (DEBUG)
		cerr << "LEAF OVERFLOW" << endl;

	/* STEPS:
	 * 1) Insert and Split the leaf node
	 * 2) Copy the first key of the new sibling node to the parent
	 */

	// Create a new sibling node
	int siblingKey;
	PageId siblingPid = pf.endPid();
	BTLeafNode siblingNode;
	siblingNode.setNextNodePtr(leafNode.getNextNodePtr());
	leafNode.setNextNodePtr(siblingPid);
	siblingNode.write( siblingPid, pf);

	// Insert and Split the leaf node
	leafNode.insertAndSplit( key, rid, siblingNode, siblingKey);
	++count;

	leafNode.write(nextPid, pf);
	siblingNode.write( siblingPid, pf);

	// Insert the sibling key into the parent
	nonLeafNode.read(treePath.top(), pf);
	if  ( nonLeafNode.getKeyCount() < nonLeafNode.MAX_KEYS )
	{
		// Insert the first key of the new sibling into the parent
		nonLeafNode.insert( siblingKey, siblingPid);
		nonLeafNode.write(treePath.top(), pf);

		// Return success
		return 0;
	}

	// Rename the variables
	int childKey = siblingKey;
	PageId childPid = siblingPid;
	int midKey;

	// Non-Leaf Overflow
	// The leaf node you're trying to insert into is full AND the parent node is also full
	if ( (treePath.size() > 1) && DEBUG )
		cerr << "NON-LEAF OVERFLOW" << endl;

	/* STEPS:
	 * 1) Insert and Split the leaf node
	 * 2) Copy the first key of the new sibling node to the parent
	 * 3) So Insert and Split the parent non leaf node and move the middle key up to its parent
	 * 4) So Insert the middle key into the parent
	 */

	// Required Vars from above
	// childKey = siblingKey of split child node below
	// childPid = siblingPid of split child node below

	// Do this until we're at the root node (treePath.size() == 1) or we have found an empty non leaf node
	while ( treePath.size() > 1 )
	{
		// Current Node
		BTNonLeafNode parentNode;
		parentNode.read(treePath.top(), pf);

		// Create a sibling node
		BTNonLeafNode siblingParentNode;
		PageId siblingParentNodePid = pf.endPid();
		siblingParentNode.write(siblingParentNodePid, pf);

		// Insert And Split the Parent Node
		parentNode.insertAndSplit(childKey, childPid, siblingParentNode, midKey);
		parentNode.write(treePath.top(), pf);
		siblingParentNode.write(siblingParentNodePid, pf);

		// Move up to the next level
		treePath.pop();

		// Set up childKey and childPid
		childKey = midKey;
		childPid = siblingParentNodePid;

		// Re read the new parent now
		parentNode.read(treePath.top(), pf);

		// Check if its full
		// if it isn't we insert the pid,key and return
		if ( parentNode.getKeyCount() < parentNode.MAX_KEYS )
		{
			// Insert the pid,key from the siblingParent
			parentNode.insert( childKey, childPid);
			parentNode.write(treePath.top(), pf);

			// Return success
			return 0;
		}
	}

	// New Root
	// When there non-leaf overflow on the current root
	if (DEBUG)
		cout << "ROOT OVERFLOW" << endl;

	/* STEPS:
	 * 1) Insert and Split the leaf node
	 * 2) Copy the first key of the new sibling node to the parent
	 * 3) Insert and Split the parent non leaf node and move the middle key up to its parent
	 * 4) Do this until the root node is also split
	 * 5) Create a new root node
	 * 6) Insert the middle key as the only value of the root node
	 */

	// Required Vars from above
	// childKey = siblingKey of split child node below
	// childPid = siblingPid of split child node below

	// Read in the root node
	BTNonLeafNode rootNode;
	rootNode.read(rootPid, pf);

	// Create the root node sibling
	BTNonLeafNode rootSiblingNode;
	PageId rootSiblingPid = pf.endPid();
	rootSiblingNode.write(rootSiblingPid, pf);

	// Insert and Split the Root
	rootNode.insertAndSplit( childKey, childPid, rootSiblingNode, midKey);
	rootNode.write(rootPid, pf);
	rootSiblingNode.write(rootSiblingPid, pf);

	// Create the new Root node
	BTNonLeafNode newRootNode;
	PageId newRootNodePid = pf.endPid();
	newRootNode.write(newRootNodePid, pf);

	// Initialize the new root
	newRootNode.initializeRoot(rootPid, midKey, rootSiblingPid);
	newRootNode.write(newRootNodePid, pf);

	// Adjust vars
	++treeHeight;
	rootPid = newRootNodePid;

	// Return Success
    return 0;
}

/**
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
	// Create a temporary non-leaf node
	BTNonLeafNode nonLeafNode;
	PageId nextPid = rootPid;

	// Follow each node until we get to a leaf node
	for (int i = 0; i < treeHeight; i++)
	{
		nonLeafNode.read(nextPid, pf);
		nonLeafNode.locateChildPtr( searchKey, nextPid);
	}

	// nextPid points to the child node containing the searchKey

	// Set the Cursor values
	cursor.pid = nextPid;
	cursor.eid = 0;

	// Create temporary values
	int tempKey;
	RecordId tempRid;

	// Read each value linearly in the node
	do
	{
		int error_code = readForward( cursor, tempKey, tempRid);
		if ( error_code != 0 )
			return error_code;
	} while ( searchKey > tempKey ); // TODO: double check this function

	// Adjust the read pointer
	if ( cursor.eid > 0 )
		cursor.eid -= 1;

	// return success
	return 0;
}

/**
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
	// Check the cursor
	if ( cursor.pid < 0 )
		return RC_END_OF_TREE;

    // Create a temporary leaf node and read the (key, rid) pair
	BTLeafNode leafNode;

	int error_code = leafNode.read( cursor.pid, pf);
	if ( error_code != 0 )
		return error_code;

	error_code = leafNode.readEntry( cursor.eid, key, rid);
	if ( error_code != 0 )
		return error_code;

	// Determine if the next entry is in the current node or the next sibling node
	if ( cursor.eid < leafNode.getKeyCount() - 1 )
	{
		++cursor.eid;
	}
	else
	{
		cursor.eid = 0;
		cursor.pid = leafNode.getNextNodePtr();
	}

	// return success
	return 0;
}

int BTreeIndex::getTreeHeight()
{
	return treeHeight;
}

int BTreeIndex::getRootPid()
{
	return rootPid;
}

RC BTreeIndex::getInitialCursor(IndexCursor& cursor)
{
	return locate( -1, cursor);
}

void BTreeIndex::printTree(PageId currentPid, int height)
{
	BTNonLeafNode nonLeafNode;
	nonLeafNode.read(currentPid, pf);
	if ( height == 0 )
	{
		cout << endl;
		for(int i = 0; i < (nonLeafNode.MAX_KEYS + nonLeafNode.MAX_PTRS); i=i+2)
		{
			cout << "Leaf (" << currentPid << ") [ ";
	
			// Print Leaf Node (only print keys)
			BTLeafNode leafNode;
			leafNode.read(currentPid, pf);
			cout << *(leafNode.buffer + 0);
			for(int i = 1; i < (3*leafNode.MAX_KEYS); i=i+2)
			{
				if ( i < leafNode.getKeyCount() )
					cout << " | " << *(leafNode.buffer + i);
				else
					cout << " | XX";
			}
			cout << " | sibling:" << leafNode.getNextNodePtr();
			cout << " ]" << endl;
		}
		cout << endl;
		return;
	}
	else if ( currentPid == rootPid )
	{
		cout << "Root (" << currentPid << ") [ ";
	} else
	{
		cout << "NonLeaf (" << currentPid << ") [ ";
	}

	// Print the current node
	cout << *(nonLeafNode.buffer + 0);
	for(int i = 1; i < (nonLeafNode.MAX_KEYS + nonLeafNode.MAX_PTRS); i++)
	{
		cout << " | " << *(nonLeafNode.buffer + i);
	}
	cout << " ]" << endl;

	if ( currentPid == rootPid )
		cout << endl;

	// recurse
	for(int i = 0; i < (nonLeafNode.MAX_KEYS + nonLeafNode.MAX_PTRS); i=i+2)
	{
		printTree( *(nonLeafNode.buffer + 2*i), height-1);
	}
}

void BTreeIndex::printLeafNode(PageId currentPid)
{
	cout << "Leaf (" << currentPid << ") [ ";

	// Print Leaf Node (only print keys)
	BTLeafNode leafNode;
	leafNode.read(currentPid, pf);
	cout << *(leafNode.buffer + 0);
	for(int i = 1; i < (3*leafNode.getKeyCount()); i=i+2)
	{
		cout << " | " << *(leafNode.buffer + i);
	}
	cout << " | sibling:" << leafNode.getNextNodePtr();
	cout << " ]" << endl;
}

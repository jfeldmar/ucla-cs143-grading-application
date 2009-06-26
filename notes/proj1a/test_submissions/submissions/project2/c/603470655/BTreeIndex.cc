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
#include <cstdio>
#include <iostream>

using namespace std;

/*
 * BTreeIndex default constructor
 */
BTreeIndex::BTreeIndex()
{
	rootPid = -1;
	treeHeight = 0;
	pf = PageFile();
	//printf("Created BTreeIndex!   rootPid = %d   treeHeight = %d\n", rootPid, treeHeight);
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
	RC retval;

	if (retval = pf.open(indexname + ".idx", mode))
		return retval;
	
	// If pf.endPid() > 0, index wasn't just created
	// so update rootPid and treeheight by checking
	// pagefile with pid == 0.
	// Else, index is new, so rootPid should be left
	// as -1 and treeHeight should be left as 0.
	// DO NOT CALL initializeRoot() HERE!
	if (pf.endPid() > 0) {
		BTLeafNode infoPage;
		if (retval = infoPage.read(INFO_PID, pf))
			return retval;
		rootPid = (PageId) infoPage.getPadding(0);
		treeHeight = infoPage.getPadding(1);
	}
	//printf("Opened BTreeIndex!   rootPid = %d   treeHeight = %d\n", rootPid, treeHeight);
	return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	RC retval = 0;

    // If PageFile was actually used (i.e. pf.endPid() > 0)
	// save rootPid and treeheight to page with pid == 0
	if (pf.endPid() > 0) {
		BTLeafNode infoPage;
		infoPage.setPadding(0, (int) rootPid);
		infoPage.setPadding(1, treeHeight);
		if (retval = infoPage.write(INFO_PID, pf))
			return retval;
	}

	if (retval = pf.close())
		return retval;
	//printf("Closed BTreeIndex!   rootPid = %d   treeHeight = %d\n", rootPid, treeHeight);
	return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	RC retval = 0;

	// If root doesn't exist, initialize it along with 2 leaf nodes
	if (rootPid == -1) {
		BTNonLeafNode root;
		rootPid = 1;
		treeHeight = 1;
		
		BTLeafNode L, R;
		PageId lPid = 2;
		PageId rPid = 3;
		if (retval = L.write(lPid, pf))
			return retval;
		if (retval = R.write(rPid, pf))
			return retval;
		
		root.initializeRoot(lPid, key, rPid);
		if (retval = root.write(rootPid, pf))
			return retval;
		//printf("L: ");
		//L.printKeys(); cout << "\n";
		//printf("R: ");
		//R.printKeys(); cout << "\n";
		//printf("Root: ");
		//root.printKV(); cout << "\n";
		treeHeight++;
	}
	
	if (retval = insertRec(key, rid, rootPid, 1))
		return retval;

	//printf("Insertion!   rootPid = %d   treeHeight = %d\n", rootPid, treeHeight);	
	return 0;
}

/*
 * Recursive helper function for insert().
 */
RC BTreeIndex::insertRec(int& key, const RecordId& rid, PageId& nodePid, int level) {
	RC retval = 0;
	
	if (level == 0) {				// overflow at root level --> new root must be initialized
		// Note: only for this case, nodePid is the PageId for the node key you are adding
		//       because the current node does not yet exist (i.e. must make a new root)
		BTNonLeafNode newRoot;			// new root node
		PageId nRPid = pf.endPid();		// new root node's PageId
		
		BTNonLeafNode oldRoot;			// current root node
		if (retval = oldRoot.read(rootPid, pf))
			return retval;
		
		BTNonLeafNode rootSib;			// current root node's sibling node
		int rSKey = 0;					// current root node's sibling node's key
		PageId rSPid = pf.endPid() + 1;	// current root node's sibling node's PageId
		
		// Uninitialize old root node
		oldRoot.uninitializeRoot();
		
		// Insert (key, nodePid) pair
		if (retval = oldRoot.insertAndSplit(key, nodePid, rootSib, rSKey))
			return retval;
		if (retval = rootSib.write(rSPid, pf))
			return retval;
		if (retval = oldRoot.write(rootPid, pf))
			return retval;
		
		// Initialize new root
		newRoot.initializeRoot(rootPid, rSKey, rSPid);
		rootPid = nRPid;
		treeHeight++;
		if (retval = newRoot.write(rootPid, pf))
			return retval;
		//printf("Root: ");
		//newRoot.printKV(); cout << "\n";
		//printf("ORoot: ");
		//oldRoot.printKV(); cout << "\n";
		//printf("RSib: ");
		//rootSib.printKV(); cout << "\n";
		return 0;
	}
	
	else if (level == 1) {			// at the root node level
		BTNonLeafNode root;			// root node
		if (retval = root.read(nodePid, pf))
			return retval;
		PageId childPid = -1;
		root.locateChildPtr(key, childPid);
		if ((retval = insertRec(key, rid, childPid, level + 1)) < 0)
			return retval;
		else if (retval > 0) {
			// Overflow at lower levels; key now contains the value
			// for the new key and childPid now contains the value
			// for the new PageId; these are now inserted into the root
			if (root.insert(key, childPid))	// root is full
				return insertRec(key, rid, childPid, 0);
			// Root was not full and (key, childPid) pair were
			// successfully inserted into root node
			else if (retval = root.write(nodePid, pf))
				return retval;
		}
		//printf("Root: ");
		//root.printKV(); cout << "\n";
	}
	
	else if (level < treeHeight) {	// at a nonleaf node level
		BTNonLeafNode nonleaf;		// nonleaf node
		if (retval = nonleaf.read(nodePid, pf))
			return retval;
		PageId childPid = -1;
		nonleaf.locateChildPtr(key, childPid);
		if ((retval = insertRec(key, rid, childPid, level + 1)) < 0)
			return retval;
		else if (retval > 0) {
			// Overflow at lower levels; key now contains the value
			// for the new key and childPid now contains the value
			// for the new PageId; these are now inserted into the root
			if (nonleaf.insert(key, childPid)) {// nonleaf is full
				// Create new sibling and insertAndSplit with nonleaf
				BTNonLeafNode sNonleaf;			// sibling nonleaf node
				int sNKey = 0;					// sibling nonleaf node's key
				PageId sNPid = pf.endPid();		// sibling nonleaf node's PageId
				if (retval = nonleaf.insertAndSplit(key, childPid, sNonleaf, sNKey))
					return retval;
				key = sNKey;		// new sibling's key must be returned
				nodePid = sNPid;	// new sibling's PageId must be returned
				//printf("NLeaf: ");
				//nonleaf.printKV(); cout << "\n";
				//printf("NSib: ");
				//sNonleaf.printKV(); cout << "\n";
				return 1;
			}
			// Nonleaf was not full and (key, childPid) pair were
			// successfully inserted into leaf node
			else if (retval = nonleaf.write(nodePid, pf))
				return retval;
		}
		//printf("NLeaf: ");
		//nonleaf.printKV(); cout << "\n";
	}

	else {							// at a leaf node level
		BTLeafNode leaf;			// leaf node
		if (retval = leaf.read(nodePid, pf))
			return retval;
		if (leaf.insert(key, rid)) {		// leaf is full
			// Create new sibling and insertAndSplit with leaf
			BTLeafNode sLeaf;				// sibling leaf node
			int sLKey = 0;					// sibling leaf node's key
			PageId sLPid = pf.endPid();		// sibling leaf node's PageId
			if (retval = leaf.insertAndSplit(key, rid, sLeaf, sLKey))
				return retval;
			if (retval = sLeaf.write(sLPid, pf))
				return retval;
			// Set next node pointer of this leaf to sibling's PageId
			leaf.setNextNodePtr(sLPid);
			if (retval = leaf.write(nodePid, pf))
				return retval;
			key = sLKey;		// new sibling's key must be returned
			nodePid = sLPid;	// new sibling's PageId must be returned
			//printf("Leaf: ");
			//leaf.printKeys(); cout << "\n";
			//printf("LSib: ");
			//sLeaf.printKeys(); cout << "\n";
			return 1;
		}
		// Leaf was not full and (key, rid) pair were
		// successfully inserted into leaf node
		else if (retval = leaf.write(nodePid, pf))
			return retval;
		//printf("Leaf: ");
		//leaf.printKeys(); cout << "\n";
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
	RC retval = 0;
	BTNonLeafNode nonleaf;
	BTLeafNode leaf;
	cursor.pid = rootPid;		// First nonleaf to get is the root node

	// Traverse through nonleaf nodes to find leaf node cursor points to
	for (int height = 1; height < treeHeight; height++) {
		if (retval = nonleaf.read(cursor.pid, pf))
			return retval;
		//printf("NonLeaf: ");
		//nonleaf.printKV(); cout << "\n";
		nonleaf.locateChildPtr(searchKey, cursor.pid);
	}
	if (retval = leaf.read(cursor.pid, pf))
		return retval;
	return leaf.locate(searchKey, cursor.eid);
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
	RC retval = 0;

	// Check validity of cursor's pid value
	if (cursor.pid == RC_END_OF_TREE)
		return RC_END_OF_TREE;
	if (cursor.pid < 0 || cursor.pid >= pf.endPid())
		return RC_INVALID_CURSOR;

    // Find the Leaf Node with pid == cursor.pid
	BTLeafNode leaf;
	if (retval = leaf.read(cursor.pid, pf))
		return retval;
	
	// Read the (key, rid) pair for the eid in the Leaf Node
	if (retval = leaf.readEntry(cursor.eid, key, rid))
		return retval;

	// Increment cursor.eid, and check to see if it is a valid eid.
	// If not, set cursor.pid to the pid of the sibling leaf and set
	// cursor.eid to the first eid in the sibling leaf (i.e. 0).
	cursor.eid++;
	if (cursor.eid >= leaf.getKeyCount()) {
		cursor.pid = leaf.getNextNodePtr();
		cursor.eid = 0;
	}

	return 0;
}

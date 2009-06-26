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
	cout << "BTreeIndex Created" << endl;
    this->rootPid = -1;
	this->treeHeight = 0;
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
	switch(mode)
	{ 
		case 'r':
		case 'R':
			mode = 'r';
			break;
		case 'w':
		case 'W':
			mode = 'w';
			break;
		default:
			return RC_INVALID_FILE_MODE;
	}

	if (pf.open(indexname, mode != 0))
		return RC_FILE_OPEN_FAILED;

    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    pf.close();
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
////////////////RC BTreeIndex::insert(int key, const RecordId& rid)
////////////////{
////////////////  PageId pid[8] = {1,2,3,4,5,6,7,8};
////////////////  RecordId p[30] = { {1,1},
////////////////					  {2,2},
////////////////					  {3,3},
////////////////					  {4,4},
////////////////					  {5,5},
////////////////					  {6,6},
////////////////					  {7,7},
////////////////					  {8,8},
////////////////					  {4,4},
////////////////					  {5,5},
////////////////					  {6,6},
////////////////					  {7,7},
////////////////					  {8,8},
////////////////					  {4,4},
////////////////					  {5,5},
////////////////					  {6,6},
////////////////					  {7,7},
////////////////					  {8,8}
////////////////  };
////////////////
////////////////  BTLeafNode leaf1(pf),leaf2(pf);
////////////////  leaf1.insert(1000, p[2]);
////////////////  leaf1.insert(500, p[1]);
////////////////  leaf1.insert(30,p[0]);
////////////////  leaf2.insert(1500,p[3]);
////////////////  leaf2.insert(3000, p[4]);
////////////////  leaf2.insert(99999, p[4]);
////////////////  leaf2.insert(9000, p[5]);
////////////////  leaf2.insert(10000, p[4]);
////////////////  leaf2.insert(12000,p[3]);
////////////////  leaf2.insert(88888, p[4]);
////////////////  leaf2.insert(14000, p[4]);
////////////////  leaf2.insert(66666, p[4]);
////////////////  leaf2.insert(77777,p[3]);
////////////////
////////////////  BTNonLeafNode nonleaf1(pf), nonleaf2(pf);
////////////////  nonleaf1.insert(1500, leaf2.getPid());
////////////////  nonleaf1.nodePid[0] = leaf1.getPid();
////////////////
////////////////  BTNonLeafNode root(pf);
////////////////  RC i = root.initializeRoot(nonleaf1.getPid(), 25000, nonleaf2.getPid());
////////////////  this->rootPid = root.getPid();
////////////////
////////////////  cout << "**Initializing root**" <<  endl;
////////////////
////////////////	if (pf.open("Random.pf", 'w') != 0)
////////////////		int i = RC_FILE_OPEN_FAILED;
////////////////
////////////////	root.printNode();
////////////////
////////////////
////////////////	if (pf.open("Random.pf", 'w') != 0)
////////////////		int i = RC_FILE_OPEN_FAILED;
////////////////
////////////////	root.write(root.getPid(), pf);
////////////////	leaf1.write(leaf1.getPid(), pf);
////////////////	leaf2.write(leaf2.getPid(), pf);
////////////////	nonleaf1.write(nonleaf1.getPid(), pf);
////////////////	nonleaf2.write(nonleaf2.getPid(), pf);
////////////////	
////////////////	if(pf.close() != 0)
////////////////		int i = RC_FILE_CLOSE_FAILED;
////////////////	
////////////////
////////////////  IndexCursor myCursor;
////////////////  locate(14000, myCursor);
////////////////
////////////////  return 0;
////////////////}




/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	#pragma region //Initialize the first root node if never created

	//////////////////////////////////////////////////
	// Initialize the first root node if never created
	//////////////////////////////////////////////////
	if (this->rootPid == -1)
	{
		cout <<  "***    Leaf root initialized and inserted with key:" << key  << endl;

		// Insert the very very first tuple
		BTLeafNode newroot(pf);
		newroot.insert(key , rid);

		// Set the BTIndex's root
		this->rootPid = newroot.getPid();
		this->treeHeight = 1;

		// Write it to disk
		if (pf.open("Random.pf", 'w') != 0)
			int i = RC_FILE_OPEN_FAILED;

		newroot.write(newroot.getPid(), pf);

		if (pf.close() != 0)
			int i = RC_FILE_CLOSE_FAILED;
	}

	#pragma endregion

	#pragma region //*Root is Leaf Node*  Insert into the root node

	//////////////////////////////////////////////////
	// *Root is Leaf Node*  Insert into the root node
	//////////////////////////////////////////////////
	else if (treeHeight == 1)
	{
		BTLeafNode myLeaf(pf);

		// Read from disk
		if (pf.open("Random.pf", 'r') != 0)
			int i = RC_FILE_OPEN_FAILED;

		myLeaf.read(this->rootPid, pf);

		if (pf.close() != 0)
			int i = RC_FILE_CLOSE_FAILED;

		// Insert tuples until full
		if (myLeaf.insert(key, rid) == RC_NODE_FULL)
		{
			cout <<  "***    Leaf root node has been split after insert of key:" << key  << endl;

			// Split the leaf into two and initialize a new root
			BTNonLeafNode newroot(pf);
			BTLeafNode sibling(pf);
			int siblingFirstKey;

			myLeaf.insertAndSplit(key, rid, sibling, siblingFirstKey);
			
			// Initialize the new root
			newroot.initializeRoot(myLeaf.getPid(), siblingFirstKey, sibling.getPid());

			// Set the child nodes' parent pid
			myLeaf.setParent(newroot.getPid());
			sibling.setParent(newroot.getPid());

			// Set the new BTIndex root
			this->rootPid = newroot.getPid();

			// Increase the height of the tree
			this->treeHeight++;

			// Write it to disk
			if (pf.open("Random.pf", 'w') != 0)
				int i = RC_FILE_OPEN_FAILED;

			newroot.write(newroot.getPid(), pf);
			sibling.write(sibling.getPid(), pf);
			myLeaf.write(myLeaf.getPid(), pf);

			if (pf.close() != 0)
				int i = RC_FILE_CLOSE_FAILED;
		}
		else
		{
			
			cout <<  "***    Leaf root non overflow, insert of key:" << key  << endl;

			// Write it to disk
			if (pf.open("Random.pf", 'w') != 0)
				int i = RC_FILE_OPEN_FAILED;

			myLeaf.write(myLeaf.getPid(), pf);

			if (pf.close() != 0)
				int i = RC_FILE_CLOSE_FAILED;
		}
	}

	#pragma endregion 

	#pragma region//*Root is Non-Leaf Node* Insert into leaf
	else
	{
		BTNonLeafNode myNonLeaf(pf);
		BTLeafNode myLeaf(pf);

		// Read from disk
		if (pf.open("Random.pf", 'r') != 0)
			int i = RC_FILE_OPEN_FAILED;

		myNonLeaf.read(this->rootPid, pf);

		if (pf.close() != 0)
			int i = RC_FILE_CLOSE_FAILED;

		// Locate which leaf node we want to insert into
		IndexCursor myCursor;
		locate(key, myCursor);

		// Read leaf node from disk
		if (pf.open("Random.pf", 'r') != 0)
			int i = RC_FILE_OPEN_FAILED;

		myLeaf.read(myCursor.pid, pf);

		if (pf.close() != 0)
			int i = RC_FILE_CLOSE_FAILED;

		// Leaf is going to be split
		if (myLeaf.insert(key, rid) == RC_NODE_FULL)
		{	
			cout <<  "***    Leaf node has been split after insert of key:" << key << endl;

			// Split the leaf into two
			BTLeafNode sibling(pf);
			int siblingFirstKey;

			myLeaf.insertAndSplit(key, rid, sibling, siblingFirstKey);

			// Set the sibling node's parent pid
			sibling.setParent(myLeaf.getParentPid());

			// Write it to disk
			if (pf.open("Random.pf", 'w') != 0)
				int i = RC_FILE_OPEN_FAILED;

			sibling.write(sibling.getPid(), pf);
			myLeaf.write(myLeaf.getPid(), pf);

			if (pf.close() != 0)
				int i = RC_FILE_CLOSE_FAILED;

			// Recursively fix the tree
			fixTree(siblingFirstKey, sibling.getPid(), sibling.getParentPid(), this->treeHeight);
		}

		// Otherwise write to disk
		else
		{	
			cout << "***    NonLeaf insert of key:" << key << endl;

			// Write it to disk
			if (pf.open("Random.pf", 'w') != 0)
				int i = RC_FILE_OPEN_FAILED;

			myLeaf.write(myLeaf.getPid(), pf);

			if (pf.close() != 0)
				int i = RC_FILE_CLOSE_FAILED;
		}
	}

	#pragma endregion

	return 0;
}

void BTreeIndex::fixTree(int key, PageId selfpid, PageId parentpid, int level)
{
	BTNonLeafNode	myParent(pf);

	// Read from disk
	if (pf.open("Random.pf", 'r') != 0)
		int i = RC_FILE_OPEN_FAILED;

	myParent.read(parentpid,pf);

	if (pf.close() != 0)
		int i = RC_FILE_CLOSE_FAILED;

	// Stopping Condition - current nonleaf node is not full
	if (myParent.insert(key, selfpid) != RC_NODE_FULL)
	{
		cout << "***    NonLeaf node has been inserted at level:" << level<< "   after insert of key:" << key << endl;

		// Write it to disk
		if (pf.open("Random.pf", 'w') != 0)
			int i = RC_FILE_OPEN_FAILED;

		myParent.write(myParent.getPid(), pf);

		if (pf.close() != 0)
			int i = RC_FILE_CLOSE_FAILED;
		
	}

	// Recursively fix it
	else
	{
		cout << "***    NonLeaf node has been split at level:" << level<< "   after insert of key:" << key << endl;

		// Split the leaf into two
		BTNonLeafNode parentSibling(pf);
		int parentMidKey;

		myParent.insertAndSplit(key, selfpid, parentSibling, parentMidKey);

		#pragma region// Reset the parents of all pids pointed to from sibling
	
		// kids are leaves
		if(level == treeHeight)
		{
			BTLeafNode templeaf(pf);

			for (int k = 0; k <= parentSibling.getKeyCount(); k++)
			{
				// Read from disk
				if (pf.open("Random.pf", 'r') != 0)
					int i = RC_FILE_OPEN_FAILED;

				templeaf.read(parentSibling.pidFinder(k),pf);

				if (pf.close() != 0)
					int i = RC_FILE_CLOSE_FAILED;

				templeaf.setParent(parentSibling.getPid());

				// Write from disk
				if (pf.open("Random.pf", 'w') != 0)
					int i = RC_FILE_OPEN_FAILED;

				templeaf.write(parentSibling.pidFinder(k),pf);

				if (pf.close() != 0)
					int i = RC_FILE_CLOSE_FAILED;
			}
		}

		// kids are nonleaves
		else
		{
			BTNonLeafNode tempnonleaf(pf);

			for (int k = 0; k <= parentSibling.getKeyCount(); k++)
			{
				// Read from disk
				if (pf.open("Random.pf", 'r') != 0)
					int i = RC_FILE_OPEN_FAILED;

				tempnonleaf.read(parentSibling.pidFinder(k),pf);

				if (pf.close() != 0)
					int i = RC_FILE_CLOSE_FAILED;

				tempnonleaf.setParent(parentSibling.getPid());

				// Write from disk
				if (pf.open("Random.pf", 'w') != 0)
					int i = RC_FILE_OPEN_FAILED;

				tempnonleaf.write(parentSibling.pidFinder(k),pf);

				if (pf.close() != 0)
					int i = RC_FILE_CLOSE_FAILED;
			}
		}

		#pragma endregion
		
		// Set the sibling node's parent pid
		parentSibling.setParent(myParent.getParentPid());

		// Write it to disk
		if (pf.open("Random.pf", 'w') != 0)
			int i = RC_FILE_OPEN_FAILED;

		parentSibling.write(parentSibling.getPid(), pf);
		myParent.write(myParent.getPid(), pf);

		if (pf.close() != 0)
			int i = RC_FILE_CLOSE_FAILED;

		
		// Stopping Condition - newroot
		if (level-1 == 1)
		{			
			cout << "***    New root node is initialized at level:" << level<< "   after insert of key:" << key << endl;

			// Create new root	
			BTNonLeafNode newroot(pf);
			
			// Initialize the new root
			newroot.initializeRoot(myParent.getPid(), parentMidKey, parentSibling.getPid());

			// Set the child nodes' parent pid
			myParent.setParent(newroot.getPid());
			parentSibling.setParent(newroot.getPid());

			// Set the new BTIndex root
			this->rootPid = newroot.getPid();

			// Increase the height of the tree
			this->treeHeight++;

			// Write it to disk
			if (pf.open("Random.pf", 'w') != 0)
				int i = RC_FILE_OPEN_FAILED;

			newroot.write(newroot.getPid(), pf);
			myParent.write(myParent.getPid(), pf);
			parentSibling.write(parentSibling.getPid(), pf);
			if (pf.close() != 0)
				int i = RC_FILE_CLOSE_FAILED;

			// Notice this return!!!
			return;
		}
		else
		{
			BTNonLeafNode myParentsParent(pf);

			// Read from disk
			if (pf.open("Random.pf", 'r') != 0)
				int i = RC_FILE_OPEN_FAILED;

			myParentsParent.read(parentSibling.getParentPid(),pf);

			if (pf.close() != 0)
				int i = RC_FILE_CLOSE_FAILED;

			// Stopping condition - sucessful insert of midkey into parent
			if (myParentsParent.insert(parentMidKey, parentSibling.getPid()) != RC_NODE_FULL)
			{
				// Write it to disk
				if (pf.open("Random.pf", 'w') != 0)
					int i = RC_FILE_OPEN_FAILED;

				myParentsParent.write(myParentsParent.getPid(), pf);

				if (pf.close() != 0)
					int i = RC_FILE_CLOSE_FAILED;

			}
			// Otherwise recursively fix the tree
			else
				fixTree(parentMidKey, parentSibling.getPid(), parentSibling.getParentPid(), level-1);
		}
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
	cursor = findLeaf(searchKey, this->rootPid, 0+1);

	return 0;
}

IndexCursor BTreeIndex::findLeaf(int searchKey, PageId root, int height)
{
	// Stopping condition -- Search Key should be on this level
	if (height == this->treeHeight)
	{
		IndexCursor returnvalue;

		// Read the current leaf node
		BTLeafNode curNode(pf);
		if (pf.open("Random.pf", 'r') != 0)
			int i = RC_FILE_OPEN_FAILED;

		curNode.read(root, pf);
		
		if(pf.close() != 0)
			int i = RC_FILE_CLOSE_FAILED;

		// Find the searchKey and set returnvalues
		int eid;

		curNode.locate(searchKey, eid);

		returnvalue.eid = eid;
		returnvalue.pid = root;

		return returnvalue;
	}
	
	// Read the current node
	BTNonLeafNode curNode(pf);
	if (pf.open("Random.pf", 'r') != 0)
		int i = RC_FILE_OPEN_FAILED;

	curNode.read(root, pf);
	
	if(pf.close() != 0)
		int i = RC_FILE_CLOSE_FAILED;

	// Find the correct child ptr to follow
	PageId nextPid;
	curNode.locateChildPtr(searchKey, nextPid);


	// Next recursive call down tree to get closer to the searchKey
	return findLeaf(searchKey, nextPid, height+1);
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
	// Read the current node
	BTLeafNode curNode(pf);
	if (pf.open("Random.pf", 'r') != 0)
		int i = RC_FILE_OPEN_FAILED;

	curNode.read(cursor.pid, pf);
	
	if(pf.close() != 0)
		int i = RC_FILE_CLOSE_FAILED;
	
	// Read the key and rid at the cursor
	curNode.readEntry(cursor.eid, key, rid);

	// Move the cursor to the next entry
	if (cursor.eid == BTLeafNode_N-1)
	{
		cursor.pid = curNode.getNextNodePtr();
		cursor.eid = 0;
	}
	else
		cursor.eid++;

	return 0;
}
	

PageId BTreeIndex::getRoot()
{
	return this->rootPid;
}

void BTreeIndex::printEntireTree(PageId selfpid, int level)
{
	cout << "CURRENT LEVEL:" << level << "       FULL TREE HEIGHT:" << treeHeight << endl << endl;

	if (level == treeHeight)
	{
		BTLeafNode leaf(pf);

		if (pf.open("Random.pf", 'r') != 0)
			int x = RC_FILE_OPEN_FAILED;
	
		leaf.read(selfpid,pf);

		if (pf.close() != 0)
			int x = RC_FILE_CLOSE_FAILED;
		
		leaf.printNode();

		return;
	}

	else
	{
		BTNonLeafNode root(pf);

		// Read from disk
		if (pf.open("Random.pf", 'r') != 0)
			int x = RC_FILE_OPEN_FAILED;

		root.read(selfpid,pf);

		if (pf.close() != 0)
			int x = RC_FILE_CLOSE_FAILED;
		
		root.printNode();

		
		cout << endl << "CURRENT LEVEL:" << level + 1 << "       FULL TREE HEIGHT:" << treeHeight << endl << endl;

		for (int i = 0; i <= root.getKeyCount(); i++)
		{	
			PageId pid = root.pidFinder(i);

			if ((level + 1) == treeHeight)
			{
					
				BTLeafNode leaf(pf);
				if (pf.open("Random.pf", 'r') != 0)
					int x = RC_FILE_OPEN_FAILED;

				leaf.read(pid,pf);

				if (pf.close() != 0)
					int x = RC_FILE_CLOSE_FAILED;
				
				leaf.printNode();
			}
			else
			{
				BTNonLeafNode nonleaf(pf);
				
				// Read from disk
				if (pf.open("Random.pf", 'r') != 0)
					int x = RC_FILE_OPEN_FAILED;

				nonleaf.read(pid,pf);

				if (pf.close() != 0)
					int x = RC_FILE_CLOSE_FAILED;

				nonleaf.printNode();

				cout << endl<<  "*************************************" << endl;
				cout << "* PRINTING TREE FROM A NONLEAF NODE *" << endl;
				cout << "*************************************" << endl << endl;

				printEntireTree(pid,level+1);
				
				cout << endl << "*************************************" << endl ;
				cout << "***********  DONE        ***********" << endl ;
				cout << "*************************************" << endl << endl;
		

			}
		}
	}
	return;
}

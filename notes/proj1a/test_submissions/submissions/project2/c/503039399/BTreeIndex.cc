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

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
	treeHeight = 0;
	maxPageId = 0;
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
	printf("Opening the index\n");
	int rc = 0;
	// Open the page file
	if ((rc = pf.open(indexname, mode)) < 0)
		return rc;
		
	// If the file is not empty, read it into memory
	
	if (pf.endPid() != 0)
	{
		// Read the page file into main memory.  Metadata about the index is always stored at pid 0
		char buffer[PageFile::PAGE_SIZE];
		if ((rc = pf.read(0, buffer)) < 0)
			return rc;
		memcpy(this, buffer, PageFile::PAGE_SIZE);
		// printf("rootPid = %i, treeHeight = %i, maxPageId = %i\n", rootPid, treeHeight, maxPageId);

	}
	
	return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	printf("Closing the index\n");
	
	int rc = 0;
	// Write the metadata of the index to the pagefile

	char buffer[PageFile::PAGE_SIZE];
	memcpy(buffer, this, PageFile::PAGE_SIZE);
	
	if ((rc = pf.write(0, buffer)) < 0)
		return rc;

	// Close the pagefile
	
	if ((rc = pf.close()) < 0)
		return rc;
    
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
	int rc;
	
	// Check to see if there is a root node.  If not, create one
	if (rootPid == -1)
	{			
		maxPageId = 1;
		
		// Create the root node
		BTLeafNode rootNode;
		rootPid = maxPageId++;
	
		rootNode.insert(key, rid);

		if ((rc = rootNode.write(rootPid, pf)) < 0)
		{
			fprintf(stderr, "Error: unable to write the root node.  pid = %i\n", rootPid);
			return rc;
		}
		treeHeight = 1;
	}
	else
	{
		SplitInfo si;
		insert_rec(key, rid, 1, rootPid, si); // recursive call
	}
	
    return 0;
}

int BTreeIndex::insert_rec(int key, const RecordId& rid, int d, PageId currentPid, SplitInfo& si)
{
//	printf("Recursive call at depth %i\n", d);
	int split;
	int rc;
	
	// If we are at a leaf  node...
	if (d == treeHeight)
	{
		BTLeafNode currentLeaf;
		currentLeaf.read(currentPid, pf);
		
		//printf("CurrentPid: %i, rootPid: %i, keycount: %i\n", currentPid, rootPid, currentLeaf.getKeyCount());
		if (currentLeaf.isFull())
		{
			//printf("Splitting Leaf\n");
			BTLeafNode siblingLeaf;
			PageId siblingPid = maxPageId++;
			int siblingKey;
			currentLeaf.insertAndSplit(key, rid, siblingLeaf, siblingPid, siblingKey);
			siblingLeaf.write(siblingPid, pf);
			currentLeaf.write(currentPid, pf);
			
			si.newKey = siblingKey;
			si.newPid = siblingPid;
			return 1;
		
		}
		// If the root is a leaf node and is also full
		else if (currentPid == rootPid && currentLeaf.getKeyCount() == 2)
		{
			// Create a new root node
			//printf("Splitting the root\n");
			BTNonLeafNode newRoot;
			PageId newRootPid = maxPageId++;
			BTLeafNode siblingLeaf;
			PageId siblingPid = maxPageId++;
			int siblingKey;
			currentLeaf.insertAndSplit(key, rid, siblingLeaf, siblingPid, siblingKey);
			newRoot.initializeRoot(currentPid, siblingKey, siblingPid);
			treeHeight++;
			newRoot.write(newRootPid, pf);
			currentLeaf.write(currentPid, pf);
			siblingLeaf.write(siblingPid, pf);
			rootPid = newRootPid;
			return 1;
		}
		
		else
		{
			if ((rc = currentLeaf.insert(key, rid)) < 0)
			{
				fprintf(stderr, "Error: unable to insert into the leaf node.  pid = %i, %i\n", currentPid, rc);
				return rc;
			}
			
			if ((rc = currentLeaf.write(currentPid, pf)) < 0)
			{
				fprintf(stderr, "Error: unable to write the leaf node.  pid = %i, %i\n", currentPid, rc);
				return rc;
			}
			// currentLeaf.print();
			return 0;
		}
	}
	// If we're not a leaf node...
	
	BTNonLeafNode currentNode;
	currentNode.read(currentPid, pf);
	PageId childPid;
	currentNode.locateChildPtr(key, childPid);
	
	split = insert_rec(key, rid, d+1, childPid, si); // Recursive call
	
	// If we split a node
	if (split == 1)
	{
		
		if (currentNode.isFull())
		{
			//printf("Splitting again\n");
			BTNonLeafNode siblingNode;
			PageId siblingPid = maxPageId++;
			int midKey;
			currentNode.insertAndSplit(si.newKey, si.newPid, siblingNode, midKey);
			siblingNode.write(siblingPid, pf);
			currentNode.write(currentPid, pf);

			si.newKey = midKey;
			si.newPid = siblingPid;
			return 1;
		}
		// If we are splitting the root node
		else if (currentPid == rootPid && currentNode.getKeyCount() == 2)
		{
			//printf("Splitting the non-leaf root\n");
			// Create a new root node
			BTNonLeafNode newRoot;
			PageId newRootPid = maxPageId++;
			BTNonLeafNode siblingNode;
			PageId siblingPid = maxPageId++;
			int midKey;
			currentNode.insertAndSplit(si.newKey, si.newPid, siblingNode, midKey);
			newRoot.initializeRoot(currentPid, midKey, siblingPid);
			treeHeight++;
			newRoot.write(newRootPid, pf);
			currentNode.write(currentPid, pf);
			siblingNode.write(siblingPid, pf);
			rootPid = newRootPid;			
			return 1;
			
		}
		else
		{
			//printf("inserting into the non-leaf after a split\n");
			currentNode.insert(si.newKey, si.newPid);
			currentNode.write(currentPid, pf);
			return 0;
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
	return locate_rec(searchKey, rootPid, cursor, 1);
}



RC BTreeIndex::locate_rec(int searchKey, PageId currentPid, IndexCursor& cursor, int d)
{

	if (rootPid == -1)
		return RC_END_OF_TREE;
	
	// If we are at the bottom of the tree, search the node for searchKey
	if (treeHeight == d)
	{
		int rc;
		int eid;
		BTLeafNode leafNode;
		leafNode.read(currentPid, pf);
		rc = leafNode.locate(searchKey, eid);
		if (rc < 0)
			return rc;
		cursor.eid = eid;
		cursor.pid = currentPid;
		return 0;
	}
	
	else
	{
		PageId childPid;
		BTNonLeafNode currentNode;
		currentNode.read(currentPid, pf);
		currentNode.locateChildPtr(searchKey, childPid);
		
		return locate_rec(searchKey, childPid, cursor, d+1); // recursive call
		
	}
	
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to a leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
	BTLeafNode currentNode;
	currentNode.read(cursor.pid, pf);
	int keyCount = currentNode.getKeyCount();
	currentNode.readEntry(cursor.eid, key, rid);
	
	if (cursor.eid < keyCount-1)
		cursor.eid++;
	else
	{
		cursor.pid = currentNode.getNextNodePtr();
		if (cursor.pid == -1)
			return RC_END_OF_TREE;
		cursor.eid = 0;	
	}
	
    return 0;
}



void BTreeIndex::printTree(PageId currentPid, int d)
{
	if (currentPid == rootPid && treeHeight == 1)
	{
		BTLeafNode currentNode;
		currentNode.read(rootPid, pf);
		printf("*Root*");
		currentNode.print();
		return;
	}
	else if (d < treeHeight-1)
	{
		//printf("Level %i\n", d);
		BTNonLeafNode currentNode;
		currentNode.read(currentPid, pf);
		if(currentPid == rootPid)
			printf("*Root*");
		else
			printf("*Node%i-NL*", currentPid);
		currentNode.print();
		
		for(int i=0; i<=currentNode.getKeyCount(); i++)
		{
			PageId nextPid = currentNode.getPidAtEntry(i);
			printf("\n");
			printTree(nextPid, d+1);
			
		}
	}
	else
	{
		BTLeafNode currentNode;
		currentNode.read(currentPid, pf);
		printf("*Node%i-L*", currentPid);
		currentNode.print();
	}
	
	
}


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

RC BTreeIndex::loadMemberVariables()
{
	int buffer[PageFile::PAGE_SIZE/sizeof(int)];
	RC rc;

	rc = pf.read(0, buffer);

	if (rc == 0)
	{
		rootPid = buffer[0];
		treeHeight = buffer[1];
		//memcpy(&rootPid, buffer, 1);
		//memcpy(&treeHeight, buffer+1, 1);
	}
	return rc;
}

RC BTreeIndex::setMemberVariables()
{
	int buffer[2];
	buffer[0] = rootPid;
	buffer[1] = treeHeight;

	RC rc;
	rc = pf.write(0, buffer);

	return rc;
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
	//PageID 0 is reserved for header information
	RC rc;
	rc = pf.open(indexname, mode);
	if (rc == 0)
	{
		loadMemberVariables();		//Not an errror if rc is not 0
		if (mode == 'w' || mode == 'W')
			rc = setMemberVariables();		//Save intial values to disk, ensure pageId 0 is taken, save new rc
	}
	return rc;
}

/*
 *  the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	RC rc;
	rc = setMemberVariables();
	if (rc != 0)
		return rc;
}

/*
 * Insert (key, RecordId) pair to the index.
 * does work of Insert in a recursive manner 
 * @param pid[IN] the pageID of the node we are inspecting
  * @param depth[IN] how deep the node we are inspecting is in the tree
 * @param key[IN/OUT] the key for the value inserted into the index, on return, the key that should be inserted in the next-highest level
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return PageID. -1 if no overflow, otherwise, the sibling PageID of the splitAndInsert
 */
PageId BTreeIndex::insertHelper(const PageId pid, int depth, int& key, const RecordId& rid)
{
	PageId value = -1;
	RC rc;
	
	if(depth == treeHeight)		//base case (when we are at the leaf node)
	{
		BTLeafNode leaf;
		leaf.read(pid, pf);		//load the node
		
		if(leaf.getKeyCount() < leaf.getMaxKeyCount())		//if it won't overflow, insert the key and rid
		{
			rc = leaf.insert(key, rid);
			
			if(rc != 0) {
				fprintf(stderr, "Error %d in BTreeIndex::insertHelper", rc);
				return rc;
			}
		}
		else										//it will overflow
		{
			BTLeafNode sibling;
			int siblingKey;
			
			rc = leaf.insertAndSplit(key, rid, sibling, siblingKey);		// use insertAndSplit
			
			if(rc != 0) {
				fprintf(stderr, "Error %d in BTreeIndex::insertHelper", rc);
				return rc;
			}
			
			value = pf.endPid();
			sibling.write(value, pf);
			sibling.setNextNodePtr(leaf.getNextNodePtr());
			leaf.setNextNodePtr(value);			//set the next node 
			
			key = siblingKey;						//set siblingKey as a return value
		}
		
		leaf.write(pid, pf);			//save the current node
	}
	else			//we are in non-leaf nodes of tree
	{
		BTNonLeafNode nonLeaf;
		rc = nonLeaf.read(pid, pf);						//load node
		PageId returnedPid, pid2;

		if(rc != 0) {
			fprintf(stderr, "Error %d in BTreeIndex::insertHelper", rc);
			return rc;
		}
		
		rc = nonLeaf.locateChildPtr(key, pid2);							//find which PID to follow
		
		if(rc != 0) {
			fprintf(stderr, "Error %d in BTreeIndex::insertHelper", rc);
			return rc;
		}
				
		returnedPid = insertHelper(pid2, (depth + 1), key, rid);	//call insertHelper
		
		if(returnedPid >= 0)				//if there was an overflow in the child node	
		{
			if(nonLeaf.getKeyCount() < nonLeaf.getMaxKeyCount())		//if adding another value won't overflow the current node
			{
				//Insert the value into the current node
				rc = nonLeaf.insert(key, returnedPid);
				
				if(rc != 0) {
					fprintf(stderr, "Error %d in BTreeIndex::insertHelper", rc);
					return rc;
				}
			}
			else						//if the current node will overflow
			{
				BTNonLeafNode sibling;
				int midKey;

				rc = nonLeaf.insertAndSplit(key, returnedPid, sibling, midKey);		// use insertAndSplit'
				
				if(rc != 0) {
					fprintf(stderr, "Error %d in BTreeIndex::insertHelper", rc);
					return rc;
				}
				
				value = pf.endPid();				//set return value
				sibling.write(value, pf);			//save the sibling node
				
				key = midKey;						//set midKey as a return value
			}
		}

		nonLeaf.write(pid, pf);			//save the current node
	}
	return value;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
//don't forget to update tree height or to save; also save BTreeIndex vars; error codes?
	RC rc = 0;
	
	if(treeHeight == 0)			//if the tree height is 0, create a root
	{	
		BTLeafNode ln1;				//create a leaf node that will also be the root
		
		rc = ln1.insert(key, rid);	//insert key into leaf node
		
		treeHeight = 1;
		
		if(!rc)
		{
			rootPid = pf.endPid();
			rc = ln1.write(rootPid, pf);		//write the leaf/root node
		}
		
		if (!rc)
			rc = setMemberVariables();
	}
	else			//not the first insertion on the tree
	{
		PageId sibling = insertHelper(rootPid, 1, key, rid);		//do the insertion using the helper function
		
		if(sibling >= 0)				// if we have to create a new root
		{
			BTNonLeafNode newRoot;						//create the new root
			newRoot.initializeRoot(rootPid, key, sibling);
			
			rootPid = pf.endPid();				//set the new rootPid
			rc = newRoot.write(pf.endPid(), pf);		//write the new root
			
			if (!rc)
			{
				treeHeight++;				//increase the tree height
				rc = setMemberVariables();
			}
		}
	}
    return rc;
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
	if (treeHeight <= 0)
	{
		cursor.pid = -1;
		cursor.eid = -1;
		//fprintf(stderr, "Error %d in BTreeIndex::locate: No elements to search.\n", rc);
		return RC_NO_SUCH_RECORD;
	}
	RC rc;
	PageId childPtr = rootPid;	//initialize to root
	BTNonLeafNode nonleafNode;

	int depth = 0;
	while (depth++ < treeHeight-1)
	{
		//DEBUG:
		//cout << "childPtr = " << childPtr << endl;
		nonleafNode.read(childPtr, pf);
		rc = nonleafNode.locateChildPtr(searchKey, childPtr);
		if (rc != 0)
		{
			fprintf(stderr, "Error %d in BTreeIndex::locate: Could not read Non-Leaf node.\n", rc);
			return rc;
		}
	}

	//At a leaf node
	BTLeafNode leafNode;
	int eid = -1;

	leafNode.read(childPtr, pf);
	rc = leafNode.locate(searchKey, eid);
	if (rc!= 0)
	{
		cursor.pid = -1;
		cursor.eid = -1;
		fprintf(stderr, "Error %d in BTreeIndex::locate: Could not read Leaf Node.\n", rc);
		return rc;
	}

	cursor.pid = childPtr;
	cursor.eid = eid;

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
	RC rc = 0;
	BTLeafNode tempNode;
	
	rc = tempNode.read(cursor.pid, pf);
	
	if(rc)			//if there is an error
	{
		return rc;
	}
	
	rc = tempNode.readEntry(cursor.eid, key, rid);			//read the entry
	
	if(rc)			//if there is an error
	{
		return rc;
	}
	
	if((cursor.eid + 1) >= tempNode.getKeyCount())			//if it is beyond the end of the valid keys
	{
			cursor.eid = 0;
			cursor.pid = tempNode.getNextNodePtr();
	}
	else
		cursor.eid++;			//advance cursor
	
    return rc;
}

//void BTreeIndex::printNode(int n, bool isLeafNode)
//{
//	if (isLeafNode)
//	{
//	  BTLeafNode ln;
//	  ln.read(n, pf);
//	  ln.displayContents();
//	}
//	else
//	{
//	  BTNonLeafNode nl;
//	  nl.read(n, pf);
//	  nl.nodeState();
//	}
//}

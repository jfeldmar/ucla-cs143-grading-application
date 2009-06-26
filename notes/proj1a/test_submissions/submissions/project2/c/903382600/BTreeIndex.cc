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

using namespace std;

/*
 * BTreeIndex constructor
 */
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
 *
 * If the index is newly created, initialize height to 1 and root node to pid=1
 * Otherwise, read the height of tree and root node into main memory
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
  RC   rc;
  char page[PageFile::PAGE_SIZE];
  char* ptr;

  // open the page file
  if ((rc = pf.open(indexname, mode)) < 0) return rc;

  // if the pf is new, initialize height and rootPid
  if (pf.endPid() == 0) {
	  rootPid = -1;
	  treeHeight = 0;
  }
  // else copy the height and pid
  else {
	  pf.read(0,page);
	  ptr = page;
	  memcpy(&rootPid, ptr, sizeof(PageId));
	  ptr = ptr + sizeof(PageId);
	  memcpy(&treeHeight, ptr, sizeof(int));
  }
	   
    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	char page[PageFile::PAGE_SIZE];
	char* ptr = page;
	
	//save the rootPid and treeHeight
	memcpy(ptr, &rootPid, sizeof(PageId));
	ptr = ptr + sizeof(PageId);
	memcpy(ptr, &treeHeight, sizeof(int));
	pf.write(0, page);
	
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
	RC errcode;
	
	//If this is a new index, create the first two leaf nodes and root node
	if (treeHeight == 0)
	{
		BTLeafNode root; //Initialized to pid 1
		
		//Insert into leaf node and save to disk
		errcode = root.insert(key, rid);
		root.write(1, pf);
		root.setNextNodePtr(-1);
		
		//Update index bookkeeping variables
		treeHeight = 1;
		rootPid = 1;
		
		return errcode;
	}
	
	//If the tree currently consists of only a single leaf node
	else if (treeHeight == 1) {

		BTLeafNode leaf;
		int siblingkey;
			
		//Read the current leaf data into main memory
		leaf.read(rootPid, pf);
		
		//Attempt to insert into leaf
		errcode = leaf.insert(key, rid);
		
		//If the node was full
		if (errcode = RC_NODE_FULL) {
			//Create a sibling leaf node and a nonleaf node and grow the tree
			BTLeafNode sibling;
			BTNonLeafNode parent; 
			
			//Split leaf node
			leaf.insertAndSplit(key, rid, sibling, siblingkey);
			
			//Write new leaf data to disk
			leaf.write(rootPid, pf);
			
			//Allocate a new page for the sibling node and write to Disk
			int siblingpid = pf.endPid();
			sibling.write(siblingpid, pf);
			
			//Set next node pointers
			sibling.setNextNodePtr(leaf.getNextNodePtr());
			leaf.setNextNodePtr(siblingpid);
			
			//Initialize parent as root node
			int newrootpid = pf.endPid(); //Allocate a new page for new root
			parent.initializeRoot(1, siblingkey, siblingpid);
			parent.write(newrootpid, pf); //Write to disk
			
			//Update index bookkeeping variables
			treeHeight = 2;
			rootPid = newrootpid;
		}
		return 0;
	}
	
	//If the Index is already multitiered
	/* Pseudocode
	 * First run a search of the key to be inserted to find the correct leaf node
	 * Attempt to insert it into the leaf node
	 * If it doesn't fit, split the node and insert it, update parent
	 */
	
	else if (treeHeight > 1) {
		int currentdepth = 1;  //Set current depth to 1 (root)
		int pidlist[treeHeight]; //Build an array to record pid of nodes traversed
		int cursorpid = rootPid; //cursor pid for traversing tree
		BTNonLeafNode nonleaf;
		BTLeafNode leaf;
	
		pidlist[(currentdepth - 1)] = rootPid; //Note that the array starts at 0 while root node is at 1 (thus there's an offset of 1)
		nonleaf.read(rootPid,pf); //Read root node
	
		while (currentdepth < treeHeight){
			nonleaf.read(cursorpid, pf);
			nonleaf.locateChildPtr(key,cursorpid);
			currentdepth++; //Increment depth
			pidlist[(currentdepth - 1)] = cursorpid; //store the pid of the child into pid list
		}
		
		leaf.read(cursorpid, pf);
		errcode = leaf.insert(key, rid);
		
		if (errcode = RC_NODE_FULL) {
			//Create a sibling leaf node and a nonleaf node and grow the tree
			BTLeafNode sibling;
			BTNonLeafNode parent; 
			int siblingkey;
			
			//Split leaf node
			leaf.insertAndSplit(key, rid, sibling, siblingkey);
			
			//Write the updated leaf data to disk
			leaf.write(pidlist[(currentdepth - 1)], pf);
			
			//Allocate a new page for the sibling node and write to Disk
			int siblingpid = pf.endPid();
			sibling.write(siblingpid, pf);
			
			//Set next node pointers
			sibling.setNextNodePtr(leaf.getNextNodePtr());
			leaf.setNextNodePtr(siblingpid);
			
			 //Update the parent node
			 currentdepth--; //Go up one level
			 parent.read(pidlist[(currentdepth - 1)], pf); //Read parent node into main memory
			 errcode = parent.insert(siblingkey, siblingpid); //Attempt to insert	
			 
			//NonLeaf loop for updating higher parents
			while (errcode == RC_NODE_FULL) {
				//Initialize the non leaf sibling
				BTNonLeafNode siblingnonleaf;
				int nonleafsiblingpid = pf.endPid();
				siblingnonleaf.write(nonleafsiblingpid, pf);
				int midkey;
				
				//Insert and split the parent node
				parent.insertAndSplit(siblingkey, siblingpid, siblingnonleaf, midkey);
				
				//Write new nodes to disk
				parent.write(pidlist[(currentdepth - 1)], pf); 
				sibling.write(siblingpid, pf);
				
				//Insert midkey into new parent
				
				currentdepth--; //go up one level
				
				//If a new root has to be created
				if (currentdepth == 0) {
				    //Initialize the new root
					BTNonLeafNode newroot;
					int newrootpid = pf.endPid();
					newroot.write(newrootpid, pf);
					
					errcode = newroot.initializeRoot(pidlist[currentdepth],midkey, siblingpid);
					
					treeHeight++; //Increment the tree height
					rootPid = newrootpid;
					return errcode;
				}
				
				parent.read(pidlist[(currentdepth - 1)], pf);
				errcode = parent.insert(midkey, siblingpid);
			}
			
			parent.write(pidlist[(currentdepth -1)], pf);
			return errcode;
		}
	}
	
    return errcode;
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
	RC errcode;
	
	//If the index is only a single node
	if (treeHeight == 1) {
		//Initialize and load the leaf node
		BTLeafNode leaf;
		leaf.read(rootPid, pf);
		
		//Run a search in the leaf node
		errcode = leaf.locate(searchKey, cursor.eid);
		
		return errcode;
	}
	
	else if (treeHeight > 1) {
		int currentdepth = 1;  //Set current depth to 1 (root)
		BTNonLeafNode nonleaf;
		BTLeafNode leaf;
	
		cursor.pid = rootPid; //Set the cursor to root
	
		while (currentdepth < treeHeight){
			nonleaf.read(cursor.pid, pf);
			nonleaf.locateChildPtr(searchKey,cursor.pid);
			currentdepth++; //Increment depth
		}
		
		leaf.read(cursor.pid, pf);
		errcode = leaf.locate(searchKey, cursor.eid);
		return errcode;
	}

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
	BTLeafNode leaf;
	RC errcode;
	
	if (cursor.pid == -1) {
		return RC_END_OF_TREE;
	}
	
	//Read data into leaf node
	leaf.read(cursor.pid, pf);
	
	//read data into outputs
	errcode = leaf.readEntry(cursor.eid, key, rid);
	
	//Move the pointer forward
	cursor.eid = cursor.eid + 1;
	
	//If the end of the node is reached, set the cursor to point to sibling node 
	if (cursor.eid > BTLeafNode::MAX_LEAF_POINTER - 2) { //-2 to compensate for starting at 0 and sibling pointer
		cursor.pid = leaf.getNextNodePtr();
		cursor.eid = 0;
	}
	
    return errcode;
}

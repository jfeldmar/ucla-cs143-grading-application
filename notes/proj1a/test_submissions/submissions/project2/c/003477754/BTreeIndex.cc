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

//##### START OF PROXY FUNCTIONS #####

PageId BTreeIndex::getrootPid()
{
    return rootPid;
}

int BTreeIndex::getTreeHeight()
{
    return treeHeight;
}

PageId BTreeIndex::getParentPid(const PageId& pid)
{
	PageId parent;
	char page[PageFile::PAGE_SIZE];
	//Read the contents of a node with PageId of pid
	pf.read(pid, page);
	//Read the parent pointer of the node
	memcpy(&parent, page+8, sizeof(int));
	return parent;
}

RC BTreeIndex::insert_in_parent(PageId nodePid, int key, PageId siblingPid)
{	
	RC 		rc;

	//If nodePid is a root node
	if(nodePid == rootPid)
	{
		BTNonLeafNode	new_root;
		PageId			new_rootPid;
		
		char node1[PageFile::PAGE_SIZE];
		char node2[PageFile::PAGE_SIZE];
		
		new_rootPid = pf.endPid();	
		
		//get the two nodes
		if((rc = pf.read(nodePid, 		node1)) < 0 ) {return rc;}
		if((rc = pf.read(siblingPid, 	node2)) < 0 ) {return rc;}
		
		//update pointers
		new_root.setSelfPtr(new_rootPid);
		new_root.setParentPtr(-1);
		//update parent pointer
		memcpy(node1+8, &new_rootPid, sizeof(int));
		memcpy(node2+8, &new_rootPid, sizeof(int));
		
		if((rc = new_root.initializeRoot(nodePid, key, siblingPid)) < 0 ) {return rc;}
			
		//write back to pf
		if((rc = new_root.write(new_rootPid, pf)) < 0) {return rc;}
		if((rc = pf.write(nodePid, node1))    < 0) {return rc;}
		if((rc = pf.write(siblingPid, node2)) < 0) {return rc;}
			
		rootPid = new_rootPid;
		treeHeight++;
		return 0;
	}
	
	PageId 			parentPid;
	BTNonLeafNode	parentNode;
	
	parentPid = getParentPid(nodePid);
	if((rc = parentNode.read(parentPid, pf)) < 0 ) { return rc; }

	//insert key and siblingPid	
	rc = parentNode.insert(key, siblingPid);
	
	if( rc == 0 ) 
	{ 
		if((rc = parentNode.write(parentPid, pf)) < 0 ) {return rc;}	
		return 0; 
	}
	//need to split
	else if(rc == RC_NODE_FULL)
	{
		PageId			newNodePid;
		BTNonLeafNode	newNode;
		int				midKey;
			
		newNodePid 	= pf.endPid();	
		//update newNode pointers
		newNode.setSelfPtr(newNodePid);
		newNode.setParentPtr(parentPid);
		
		//split parentNode with newNode
		parentNode.insertAndSplit(key, siblingPid, newNode, midKey); 		
		
		//update parentNode and newNode in pf
		if(( rc = parentNode.write(parentPid, pf))  < 0 ) {return rc;}
		if(( rc = newNode.write(newNodePid, pf))    < 0 ) {return rc;}
	
		//update parentPtr of nodes copied over
		char pageNewNode[PageFile::PAGE_SIZE];
		char pageChild[PageFile::PAGE_SIZE];
		int  pid;
		int  i = 0;
		int  count = newNode.getKeyCount();
		
		pf.read(newNodePid, pageNewNode);
		char *ptr = pageNewNode + 20;
		
		// less than or equal because we have n pointers and count is equal to n-1 keys
		while( i <= count )
		{
			memcpy(&pid, ptr, sizeof(int));
			
			pf.read(pid, pageChild);
			memcpy(pageChild+8, &newNodePid, sizeof(int));
			pf.write(pid, pageChild);
			
			//Pointer goes to the next entry
			ptr+= 8;
			//Increment eid counter
			i++;
		}
	
		//recursive call by inserting midkey to parent of parentNode and newNode
		return insert_in_parent(parentPid, midKey, newNodePid);
	}
	else {return rc;}
}

PageId BTreeIndex::locate_helper(int key, PageId pid)
{
	RC 				rc;
	PageId 			pid_locate;
	BTLeafNode		leaf_node;
	BTNonLeafNode 	non_leaf_node;
	
	char page[PageFile::PAGE_SIZE];
	
	if((rc = pf.read(pid, page)) < 0 )	{ return -1; }
	//if node is a nonleaf, recurse call
	if(page[0] == 0)
	{
		non_leaf_node.read(pid,pf);
		non_leaf_node.locateChildPtr(key, pid_locate);
		return locate_helper(key, pid_locate);	
	}
	//if node is a leaf, 
	else if(page[0] == 1)
	{
		return pid;
	}
	
}

//##### END OF PROXY FUNCTIONS #####

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
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
    RC rc;
	char page[PageFile::PAGE_SIZE];
	
	// open the page file
	if ((rc = pf.open(indexname + ".idx", mode)) == 0) 
	{
		//if pagefile is new, 
		if (pf.endPid() == 0)
		{
			//store -1 for rootPid and treeHeight
			memcpy(page, &rootPid, sizeof(int));
			memcpy(page+4, &rootPid, sizeof(int));
			
			//store into page
			if(( rc = pf.write(0,page)) < 0 ) {return rc;}
			
		}
		//if pagefile is not new, change rootPid to the value stored 
		else
		{	
			//read the page into buffer called page
			pf.read(0,page);
		
			//read the pid and treeHeight -- a variable for BTreeIndex
			memcpy(&rootPid,	page, 	sizeof(int));
			memcpy(&treeHeight, page+4, sizeof(int));

		}
		
	}
	
	return rc;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	char page[PageFile::PAGE_SIZE];

	//update rootPid and treeHeight
	memcpy(page, &rootPid, sizeof(int));
	memcpy(page+4, &treeHeight, sizeof(int));
			
	//store into page
	pf.write(0,page);
	
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
	RC rc;

	BTLeafNode 		leafNode;
	BTLeafNode		siblingNode;
	BTNonLeafNode 	nonLeafNode;
	
	//##### 	Initialize pointers 		######
	//Initialize leafNode pointers
	leafNode.setNextNodePtr(-1);
	leafNode.setParentPtr(-1);
	leafNode.setSelfPtr(-1);
	
	//Initialize siblingNode pointers
	siblingNode.setNextNodePtr(-1);
	siblingNode.setParentPtr(-1);
	siblingNode.setSelfPtr(-1);
	
	//Initialize nonLeafNode pointers
	nonLeafNode.setParentPtr(-1);
	nonLeafNode.setSelfPtr(-1);	
	//##### 	End pointer initialization 	#####
	
	
    //If rootPid = -1 then there are no nodes in the tree
	if( rootPid == -1 )
	{
		rootPid = pf.endPid();
		treeHeight = 1;
		
		leafNode.insert(key, rid);

		//set pointer to self
		leafNode.setSelfPtr(rootPid);
		
		if( (rc = leafNode.write(rootPid,pf)) < 0) { return rc;}
		
		return 0;
	}
	//There is only one leaf node
	else if( treeHeight == 1)
	{
		leafNode.read(rootPid, pf);
		
		rc = leafNode.insert(key,rid);
		
		if( rc == RC_INVALID_ATTRIBUTE ){ return rc; }
		//Need to split the first and only leaf node and create the first and only nonLeafNode
		
		if( rc == RC_NODE_FULL)
		{
			int siblingKey;
			PageId siblingPid = pf.endPid();
			PageId parentPid  = siblingPid + 1;
			
			//set pointers
			leafNode.setNextNodePtr(siblingPid);
			leafNode.setParentPtr(parentPid);
			siblingNode.setSelfPtr(siblingPid);
			siblingNode.setParentPtr(parentPid);
			nonLeafNode.setSelfPtr(parentPid);
			//nonLeafNode.setParentPtr(-1);
			
			//insert and split
			leafNode.insertAndSplit(key, rid, siblingNode, siblingKey);
			
			//set up parent pointer values
			nonLeafNode.initializeRoot(leafNode.getSelfPtr(), siblingKey, siblingPid);
		
			//update rootPid and treeHeight
			rootPid 	= nonLeafNode.getSelfPtr();
			treeHeight 	= 2;
			
			//Write into pagefile siblingNode and parentNode
			if( (rc = siblingNode.write(siblingPid, pf)) < 0 ) { return rc; }
			if( (rc = nonLeafNode.write(parentPid,  pf)) < 0 ) { return rc; }
			if( (rc = leafNode.write(leafNode.getSelfPtr(), pf) ) < 0) { return rc; }
			
			return 0;
		}
	
		//Write into pagefile leafNode
		if( (rc = leafNode.write(leafNode.getSelfPtr(), pf) ) < 0) { return rc; }
		return 0;
	}
	else
	{
		IndexCursor cursor;
		PageId 		nodePid;
		
		//find the node to insert
		locate(key, cursor);
		nodePid = cursor.pid;
		
		//read content to leafNode
		if((rc = leafNode.read(nodePid, pf)) < 0 ) { return rc; }
		//insert key and rid
		rc = leafNode.insert(key, rid);
		
		if( rc == 0 ) 
		{ 
			if((rc = leafNode.write(nodePid, pf)) < 0 ) {return rc;}	
			return 0; 
		}
		else 
		{
			if(rc == RC_NODE_FULL)
			{
				PageId		newNodePid;
				BTLeafNode	newNode;
				int			siblingKey;
				
				newNodePid 	= pf.endPid();	
				//update pointers
				newNode.setSelfPtr(newNodePid);
				newNode.setParentPtr(leafNode.getParentPtr());
				newNode.setNextNodePtr(leafNode.getNextNodePtr());
				leafNode.setNextNodePtr(newNodePid);
				
				//split leaf node
				leafNode.insertAndSplit(key, rid, newNode, siblingKey);
				
				//update parentNode and newNode in pf
				if(( rc = leafNode.write(nodePid, pf))   < 0 ) {return rc;}
				if(( rc = newNode.write(newNodePid, pf)) < 0 ) {return rc;}
				
				return insert_in_parent(nodePid, siblingKey, newNodePid);
			}
		}

		return rc;
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
	RC 			rc;
	PageId 		pid_locate;
	int			eid_locate;
	BTLeafNode	leaf_node;
	
	pid_locate = locate_helper(searchKey, rootPid);
	if((rc = leaf_node.read(pid_locate, pf)) < 0) {return rc;}
	if((rc = leaf_node.locate(searchKey, eid_locate)) < 0) 
	{
		cursor.pid = pid_locate;
		cursor.eid = eid_locate;
		return rc;
		
	}
	
	//set values of cursor
	cursor.pid = pid_locate;
	cursor.eid = eid_locate;
	
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
	RC			rc;
	PageId		nodePid;
	BTLeafNode 	node;
	int 		nodeEid;
		
	nodePid = cursor.pid;
	nodeEid	= cursor.eid;
	
	if(nodePid == -1) {	return RC_END_OF_TREE; }
	
	if(( rc = node.read(nodePid, pf)) 			 < 0 ) {return rc;}
	if(( rc = node.readEntry(nodeEid, key, rid)) < 0 ) {return rc;}
	
	//eid is the last in the node
	if( nodeEid == (node.getKeyCount() - 1) )
	{
		cursor.pid = node.getNextNodePtr();
		cursor.eid = 0;
	}
	else
	{
		cursor.eid = nodeEid + 1;
	}
	
    return 0;
}

// More proxy functions
void BTreeIndex::print_nodes()
{
	cout<<"number of nodes : " << pf.endPid() - 1 <<endl;

	char 	page[PageFile::PAGE_SIZE];
	char 	nodeType;
	int 	entryCount;
	PageId 	parentPtr, nextNodePtr;
	
	for(int i = 1; i < pf.endPid() ; i++)
	{
		pf.read(i, page);
		memcpy(&nodeType, page, sizeof(char));
		memcpy(&entryCount, page+4, sizeof(int));
		memcpy(&parentPtr, page+8, sizeof(int));
	
		nodeType = nodeType + '0';
	
		if(nodeType == '0')
		{
			cout<<"pid: " << i << "\t node type: nonLeaf \t parent: " 	<< parentPtr << "\t entryCount: " << entryCount << endl;
		}
		else
		{
			memcpy(&nextNodePtr, page+1020, sizeof(int));
			cout<<"pid: " << i << "\t node type: leaf 	\t parent: " 	<< parentPtr << "\t entryCount: " << entryCount
				<<"\t nextNodePtr: " << nextNodePtr << endl;
		}
	}	
}

void BTreeIndex::test_readForward(int searchGreater)
{
  IndexCursor ic;
  int counter = 0;
  int key;
  RecordId rid;

  cout<<"-----------------------------------------------"<<endl;
  
  //cout<<"looking for greater than " << searchGreater << endl;
  locate(searchGreater,ic);

  while( readForward(ic, key, rid) >= 0 )
  {
		//cout<<"key: " << key << endl;
		counter++;
  }
  cout<< counter << " keys found "<< endl;

}

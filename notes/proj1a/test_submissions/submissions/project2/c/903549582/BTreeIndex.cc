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
    memset(tmp, 0, 	PageFile::PAGE_SIZE);
    
    //for debugging	
		leafNodeIndexCount =0;
		memset(leafNodeIndex, -1, sizeof(int) * 1000);
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
	int result = pf.open(indexname, mode);
	
	pf.read(TREE_PID, (void*) tmp);
	rootPid = tmp[0];
	treeHeight = tmp[1];

	if(SH_DEBUG)
		printf("rootPid:%d, treeHeight:%d \n", rootPid, treeHeight);
		
	return result;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	
	tmp[0] = rootPid;
	tmp[1] = treeHeight;
	pf.write(TREE_PID, (void*)tmp);
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

		if(SH_DEBUG){
			if(rootPid == TREE_PID){	//nothing exists
				printf("[BTreeIndex::insert]nothing exists..\n");
			}
			printf("[BTreeIndex::insert]rootPid:%d, treeHeight:%d\n", rootPid, treeHeight);
			printf("[BTreeIndex::insert]keyIn:%d, ridIn(%d, %d)\n", key, rid.pid, rid.sid);
		}
		
		BTLeafNode leafNode;
		BTNonLeafNode nonLeafNode;

		printf("KeyCount:%d\n", leafNode.getKeyCount());

		bool split = false;
		int siblingKey;
		PageId siblingPid;
	
		int level = treeHeight;
		
		
		if(treeHeight == 0){ //initial case: no root, only one leaf node
			if(SH_DEBUG){
				printf("[BTreeIndex::insert]Current Height is 0\n");
			}

			if(rootPid != TREE_PID) //not initial
				leafNode.read(rootPid, pf); //reading a leafNode
			else
				rootPid = 1; //initially

			insertLeafNode(leafNode, rootPid, key, rid, split, siblingKey, siblingPid);
			
			leafNode.write(rootPid, pf);
			
		}
		else{ //general case
			if(SH_DEBUG){
				printf("[BTreeIndex::insert]Current Height is not 0\n");
			}
			nonLeafNode.read(rootPid, pf); //starting from the root

			if(SH_DEBUG){
				printf("[BTreeIndex::insert]Read root\n");
				nonLeafNode.printNodeInfo();
			}
			findLeafNode(nonLeafNode, rootPid, key, rid, split, siblingKey, siblingPid, level);
		}
		
		//if spilit happened
		if(split){
			if(SH_DEBUG){
				printf("[BTreeIndex::insert]detected split at root, treeHeight:%d\n", treeHeight);
			}

			BTNonLeafNode newRoot;

			if(SH_DEBUG){
				printf("[BTreeIndex::insert]newRoot pid1:%d key:%d pid2:%d\n", rootPid, siblingKey, siblingPid);
			}
			
			newRoot.initializeRoot(rootPid, siblingKey, siblingPid);
			printf("init root is okay\n");
			//leafNodeIndex[leafNodeIndexCount] = rootPid;
			//leafNodeIndexCount++;

			rootPid = pf.endPid(); //update the rootPid;
			printf("get endPid is okay, rootPid:%d\n", rootPid);
			newRoot.printNodeInfo();
			newRoot.write(rootPid, pf);	//write the new root info into pageFile
			printf("write root is okay\n");
			treeHeight++;

			if(SH_DEBUG){
				printf("[BTreeIndex::insert]newRootPid:%d, treeHeight:%d\n", rootPid, treeHeight);
			}
			
		}
		else{
		//nothing to do				
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
		BTLeafNode leafNode;
		BTNonLeafNode nonLeafNode;
		int eid;

		if(treeHeight == 0){
				if(SH_DEBUG){
					printf("treeHeight==0 now it is leaf..");
				}
				leafNode.read(rootPid, pf);
				leafNode.locate(searchKey, eid);
				cursor.pid = rootPid;
				cursor.eid = eid;
				
				if(SH_DEBUG){
					printf("pid: %d, eid:%d\n", cursor.pid, cursor.eid);
				}
		}
		else{
			nonLeafNode.read(rootPid, pf);
			locateLeaf(nonLeafNode, treeHeight, searchKey, cursor);
		}
    return 0;
}

RC BTreeIndex::locateLeaf(BTNonLeafNode& nonLeafNode, int level, int searchKey, IndexCursor& cursor){
	
	BTLeafNode leafChild;
	BTNonLeafNode nonLeafChild;

	int nextPid;
	nonLeafNode.locateChildPtr(searchKey, nextPid);
	
	if(level == 1){
		if(SH_DEBUG){
			printf("Leaf Found..");
		}
		
		int eid;
		leafChild.read(nextPid, pf);
		leafChild.locate(searchKey, eid);

		if(SH_DEBUG){
			printf("pid:%d eid:%d\n", nextPid, eid);
		}
		
		cursor.eid = eid;
		cursor.pid = nextPid;
	}
	else{
		nonLeafChild.read(nextPid, pf);

		if(SH_DEBUG){
			printf("Leaf not found yet go down.. level:%d NextPid:%d\n", level, nextPid);
		}

		locateLeaf(nonLeafChild, --level, searchKey, cursor);
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
		int pid = cursor.pid;
		int eid = cursor.eid;
		
		BTLeafNode leafNode;
		leafNode.read(pid, pf);
		leafNode.readEntry(eid, key, rid);

		BTLeafNode sibling;	
			
		//move the cursor to the next entry
		if( leafNode.getKeyCount() - 1 == eid){
				//move to next sibling node...
				int siblingPid = leafNode.getNextNodePtr();
				
				if(siblingPid == -1) return RC_INVALID_CURSOR;//no sibling node next..
				
				sibling.read(siblingPid, pf);
				sibling.readEntry(0, key, rid);
				cursor.eid = 0;
				cursor.pid = siblingPid;
		}
		else{
			cursor.eid++;	
		}

    return 0;
}


RC BTreeIndex::insertLeafNode(BTLeafNode& leafNode, int leafNodePid, int key, const RecordId& rid, 
																		bool& split, int& siblingKey, PageId& siblingPageId){
	if(SH_DEBUG){
		printf("[BTreeIndex::insertLeafNode]\n");
	}
	
	if(leafNode.getKeyCount() == MAX_KEYS){ //node is full, need to split
		if(SH_DEBUG){
			printf("[BTreeIndex::insertLeafNode]split...leafNode.keyCount:%d\n", leafNode.getKeyCount());
		}		

		BTLeafNode sibling;
		leafNode.insertAndSplit(key, rid, sibling, siblingKey);

		siblingPageId = pf.endPid();
		leafNode.setNextNodePtr(siblingPageId);
		
		//after split write the info into page file
		leafNode.write(leafNodePid, pf);
		//write?
		sibling.write(siblingPageId, pf);
		split = true;

		if(SH_DEBUG){
			printf("siblingKey:%d, siblingPid:%d\n", siblingKey, siblingPageId);
		}
		leafNodeIndex[leafNodeIndexCount] = siblingPageId;
		leafNodeIndexCount++;

	}
	else{
		if(SH_DEBUG){
			printf("[BTreeIndex::insertLeafNode]no split");
		}

		leafNode.insert(key, rid);
		leafNode.write(leafNodePid, pf);
		split = false;

		if(SH_DEBUG){
			printf("...inserted\n");
		}
	}
}


RC BTreeIndex::findLeafNode(BTNonLeafNode& nonLeafNode, int nonLeafNodePid, int key, const RecordId& rid, 
																	bool& split, int& siblingKey, PageId& siblingPageId, int level){
	if(SH_DEBUG){
		printf("[BTreeIndex::findLeafNode] level:%d\n", level);
	}

	int nextPid;
	nonLeafNode.locateChildPtr(key, nextPid);

	printf("ChildPid:%d\n", nextPid);
	
	BTLeafNode leafChild;
	BTNonLeafNode nonLeafChild;
	
	if(level == 1){//reach the leafNode, next child is leaf
		printf("sreadChildNode: pid:%d\n", nextPid);
		leafChild.read(nextPid, pf);
		printf("After readChildNode: pid:%d\n", nextPid);

		if(SH_DEBUG){
			printf("[BTreeIndex::findLeafNode]leaf found!...leaf Node PageId is %d\n", nextPid);
		}
		
		leafChild.printNodeInfo(); //for debug
		
		insertLeafNode(leafChild, nextPid, key, rid, split, siblingKey, siblingPageId);

		if(SH_DEBUG){
			printf("siblingKey:%d, siblingPageId:%d\n", split, siblingKey, siblingPageId);
		}
	}
	else{
		printf("readChildNode: pid:%d\n", nextPid);
		nonLeafChild.read(nextPid, pf);
		printf("After readChildNode: pid:%d\n", nextPid);
		if(SH_DEBUG){
			printf("[BTreeIndex::findLeafNode]go down to the next level, current level:%d childPid:%d\n", level, nextPid);
		}

		findLeafNode(nonLeafChild, nextPid, key, rid, split, siblingKey, siblingPageId, --level);
	}

	if(split){
		if(SH_DEBUG){
			printf("[BTreeIndex::findLeafNode]splited!\n");
		}
		
		if(nonLeafNode.getKeyCount() == MAX_KEYS){
			if(SH_DEBUG){
				printf("[BTreeIndex::findLeafNode]also full, split up again\n");
			}
			
			BTNonLeafNode new_sibling;
	
			int new_siblingKey;
			int new_siblingPageId;
			nonLeafNode.insertAndSplit(siblingKey, siblingPageId, new_sibling, new_siblingKey);

			nonLeafNode.write(nonLeafNodePid, pf); //write the result
				
			new_siblingPageId = pf.endPid();
			new_sibling.write(new_siblingPageId, pf);
			//for return
			siblingKey = new_siblingKey;
			siblingPageId = new_siblingPageId;
			split = true;

			if(SH_DEBUG){
				printf("[BTreeIndex::findLeafNode]newSiblingKey:%d, newSiblingPageId:%d\n", siblingKey, siblingPageId);
			}
 		}
		else{
			nonLeafNode.insert(siblingKey, siblingPageId);
			nonLeafNode.write(nonLeafNodePid, pf);
			split = false;

			if(SH_DEBUG){
				printf("[BTreeIndex::findLeafNode]inserted w/o split newSiblingKey:%d, newSiblingPageId:%d\n", siblingKey, siblingPageId);
			}

		}
	}
}

void BTreeIndex::printPage(){
if(SH_DEBUG){
	printf("*****************PRINT PAGEFILE*******************\n");
	
	for(int i=1 ; i<pf.endPid(); i++){
		printf("Page:%d	", i);
		for(int j=0; j<MAX_KEYS; j++){
			if( !isLeaf(i) || i == 3 ){
				printf("Non-Leaf\n");
				BTNonLeafNode tmpNonLeaf;
				tmpNonLeaf.read(i, pf);
				tmpNonLeaf.printNodeInfo();				
			}
			else{
				printf("Leaf\n");
				BTLeafNode tmp;
				tmp.read(i, pf);
				tmp.printNodeInfo();
			}
		}
		printf("\n");
	}
}
}


void BTreeIndex::printIndexInfo(){
if(SH_DEBUG){
	printf("rootPid:%d treeHeight:%d\n", rootPid, treeHeight);
}
}

bool BTreeIndex::isLeaf(int pid){
	for(int i=0; i<1000; i++){
		if(leafNodeIndex[i] == pid) return true;
		else if(leafNodeIndex[i] == -1) return false;
	}
	return false;
}

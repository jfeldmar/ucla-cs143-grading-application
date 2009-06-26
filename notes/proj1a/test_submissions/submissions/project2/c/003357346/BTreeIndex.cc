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
#include <cassert>
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
 */

//make sure only calls once
RC BTreeIndex::open(const string& indexname, char mode)
{
	RC rc;

	if(mode == 'r') {
		return pf.open(indexname, 'r');
	}

	if(mode == 'w') {
		if((rc = pf.open(indexname, 'w')) < 0)	return rc;
	
		char page[PageFile::PAGE_SIZE];	
		rootPid = 1;	
		treeHeight = 0;
		int num_pids_used = 0;
	
		//clear the page
		memset(page, 0, PageFile::PAGE_SIZE);
		memcpy(page + ROOT_PID, &rootPid, sizeof(int));
		memcpy(page + TREE_HEIGHT, &treeHeight, sizeof(int));
		memcpy(page + NUM_PIDS_USED, &num_pids_used, sizeof(int));

		//write the meta page to pagefile
		if((rc = pf.write(0, page)) < 0) return rc;

		//test
		/*
		memset(page,0,PageFile::PAGE_SIZE);
		pf.read(0, page);
		memcpy(&treeHeight, page + TREE_HEIGHT, sizeof(int));		
		memcpy(&rootPid, page + ROOT_PID, sizeof(int));		
		printf("Rootpid: %i, treeHeight: %i\n", rootPid, treeHeight);
		*/

		return 0;
	}

	//invalid file mode
	return RC_INVALID_FILE_MODE;	
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	return pf.close();
}

RC BTreeIndex::insert_helper(int key, const RecordId rid, PageId pid, int currHeight,
				int& keyToParent, PageId& pidToParent, bool& overflow) {

	printf("insert_helper(key = %i, pid = %i, currHeight = %i)\n", key, pid, currHeight);

	//check if we are in leaf
	if(currHeight == 0) {

		//get the leaf node specified and insert into it
		BTLeafNode leaf;
		leaf.read(pid, pf);
		
		int numKeys = leaf.getKeyCount();

		//case: overflow
		if(numKeys >= NUM_KEYS_PER_PAGE) {
			printf("Leaf overflow!\n");
		
			//generate sibling node	
			BTLeafNode sibling;
			sibling.resetPage();
			PageId sibPid = pf.endPid();

			//insert and split the leaf node
			int firstSibKey;
			leaf.insertAndSplit(key, rid, sibling, firstSibKey);

			//correct "next" ptrs
			int oldNextPtr = leaf.getNextNodePtr();
			leaf.setNextNodePtr(sibPid);
			sibling.setNextNodePtr(oldNextPtr);
			sibling.setHeight(leaf.getHeight());

			//write nodes back to page file
			leaf.write(pid, pf);
			sibling.write(sibPid, pf);

			//send correct info to parent
			keyToParent = firstSibKey;
			pidToParent = sibPid;
			overflow = true; 	
		}

		//no overflow case, just insert directly	
		else  {
			printf("Inserting key <%i> into leaf <%i> with no overflow!\n", key, pid);
			leaf.insert(key, rid);
			leaf.write(pid, pf);
			overflow = false;
		}
	}

	//case non leaf
	else {

		//get the node
		BTNonLeafNode nonleaf;
		nonleaf.read(pid, pf);

		//find child to insert into
		int numKeys = nonleaf.getKeyCount();
		int myHeight = nonleaf.getHeight();
		int childToFind;
		nonleaf.locateChildPtr(key, childToFind);
		printf("looking for child %i\n", childToFind);

		//overflow variables
		int of_key = 0;
		int of_pid = 0;
		bool child_overflow = false;

		//try to insert into child 
		insert_helper(key, rid, childToFind, myHeight-1, of_key, of_pid, child_overflow);
	
		//if child overflowed	
		if(child_overflow) {
			printf("Child pid %i overflowed!\n", childToFind);
			numKeys = nonleaf.getKeyCount();

			//try to insert new key into our node
			//case: overflow
			if(numKeys >= NUM_KEYS_PER_PAGE) {
				printf("Non-leaf %i overflow at pid!\n", pid);
			
				//generate sibling	
				BTNonLeafNode sibling;
				sibling.resetPage();
				sibling.setHeight(nonleaf.getHeight());
				PageId sibPid = pf.endPid();			
				printf("New sibling has pid %i\n", sibPid);

				//create key to send to parent
				int midKey;
	
				//insert and split our current node
				nonleaf.insertAndSplit(of_key, of_pid, sibling, midKey);
				nonleaf.write(pid, pf);
				sibling.write(sibPid, pf);

				keyToParent = midKey;
				pidToParent = sibPid;
				overflow = true;
			
			}
			//case: no overflow
			else  {

				printf("Inserting into nonleaf <%i> with no overflow!\n", pid);
				nonleaf.insert(of_key, of_pid);				
				nonleaf.write(pid, pf);
				overflow = false;
			}
		}
	}
	
	printf("Leaving insert_help!\n");
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

	//check to see if we need to allocate root node
	if(treeHeight == 0) {

		printf("Allocating root node!\n");
		int c1_pid, c2_pid;
		c1_pid = 2;
		c2_pid = 3;

		//initialize root values
		BTNonLeafNode root;
		root.resetPage();
		root.initializeRoot(c1_pid, key, c2_pid);
		root.setKeyCount(1);
		root.setHeight(1);
		root.write(rootPid, pf);

		//create 2 child nodes (empty)
		BTLeafNode child1, child2;

		//set child pages
		child1.resetPage();
		child2.resetPage();
		//child1.setHeight(0);
		//child2.setHeight(0);

		//insert key into left child
		child2.insert(key, rid);

		//update "next" ptr
		child1.setNextNodePtr(c2_pid);

		//write 2 children to PageFile	
		pf.write(c1_pid, child1.m_page);
		pf.write(c2_pid, child2.m_page);

		treeHeight++;
	}

	//otherwise, we need to find the right node to insert in
	else {
		RC rc;
	
		BTNonLeafNode root;
		root.read(rootPid, pf);
		int child_to_take;

		//if((rc = root.locateChildPtr(key, child_to_take)) < 0) return rc;
		
		bool overflow = false;	
		int keyToParent = 0;
		PageId pidToParent = 0;
		int height = root.getHeight();

		printf("***Inserting!***\n");

		//call our recursive insert_helper
		insert_helper(key, rid, rootPid, height, keyToParent, pidToParent, overflow);

		//if we overflow, we need to set a new root	
		if(overflow) {
			printf("Root overflow!\n");
			treeHeight++;

			printf("keyToParent: %i, pidToParent: %i, current height: %i\n", keyToParent, pidToParent, treeHeight);

					//initialize new root	
			BTNonLeafNode newRoot;
			newRoot.resetPage();
			newRoot.setKeyCount(1);				
			newRoot.setHeight(treeHeight);
			PageId newRootId = pf.endPid();
			printf("New Root ID is %i\n", newRootId);
			newRoot.initializeRoot(rootPid, keyToParent, pidToParent);
			rootPid = newRootId;
			newRoot.write(newRootId, pf);
			//assert(false);
		
	/*	
			//change our meta page
			char page[PageFile::PAGE_SIZE];
			memset(page, 0, PageFile::PAGE_SIZE);
			int totalHeight = treeHeight + 1;						
			int num_pids_used = pf.endPid()-1;
			memcpy(page + ROOT_PID, &rootPid, sizeof(int));
			memcpy(page + TREE_HEIGHT, &totalHeight, sizeof(int));	
			memcpy(page + NUM_PIDS_USED, &num_pids_used, sizeof(int));
			pf.write(0, page);
	*/
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
	locate_helper(searchKey, rootPid, cursor, treeHeight);
	
    return 0;
}

RC BTreeIndex::locate_helper(int searchKey, PageId pid, IndexCursor& cursor, int currHeight) {

	//base case: if we are in leaf, find the correct entry and set cursor to that value
	if(currHeight == 0) {

		//get the leaf node
		BTLeafNode leaf;
		leaf.resetPage();
		leaf.read(pid, pf);

		//locate the eid of the key we are looking for	
		leaf.locate(searchKey, cursor.eid);
		cursor.pid = pid;

		printf("Located child in node pid [%i]\n", pid);

		return 0;
	}		
	
	//recursive case: find the child to take
	else {
		//get non leaf node
		BTNonLeafNode nonleaf;
		nonleaf.resetPage();
		nonleaf.read(pid, pf);

		//find which child to search next
		PageId childToTake;	
		nonleaf.locateChildPtr(searchKey, childToTake);

		printf("Following child pid %i\n", childToTake);
		
		//follow the child pointer
		locate_helper(searchKey, childToTake, cursor, currHeight - 1);
		return 0;
	}
	
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
	leaf.resetPage();
	leaf.read(cursor.pid, pf);

	//find associated key/rid	
	int entryno = cursor.eid; 
	int index = entryno*ENTRY_SIZE + META_SIZE;
	memcpy(&key, leaf.m_page + index, sizeof(int));
	memcpy(&(rid.pid), leaf.m_page + index + PID_OFFSET, sizeof(int));
	memcpy(&(rid.sid), leaf.m_page + index + SID_OFFSET, sizeof(int));
	
	//get the next record in line
	//case: if there is still one more key in this node left unread
	if(entryno < NUM_KEYS_PER_PAGE - 1) {
		cursor.eid = entryno + 1;
		printf("readForward: there are still %i keys left in this node!\n", NUM_KEYS_PER_PAGE - 1 - entryno);
		return 0;
	}
	else {
		cursor.pid = leaf.getNextNodePtr();
		cursor.eid = 0;
		printf("readForward: jumping to sibling child!\n");
		return 0;	
	}

    return 0;
}


void BTreeIndex::printTree() {
	BTNonLeafNode root;
	root.read(rootPid, pf);

	printf("-----ROOT!-----\n");
	printTreeHelper(root);
}

/* Print Tree */
void BTreeIndex::printTreeHelper(BTNonLeafNode nl) {

	nl.printPage();

	//print all children of nonleaf
	int numChild;
	for(numChild = 0; numChild < nl.getKeyCount(); numChild ++) {

		int height = nl.getHeight();
		bool childLeaf = (height == 1);
		int lchild = nl.lChild(numChild);
		int rchild = nl.rChild(numChild);

		//print left pointer	
		if(numChild == 0) {
			//if our child is a leaf
			if(childLeaf) {

				BTLeafNode child;
				printf("PID: %i\n", lchild); 
				child.read(nl.lChild(numChild), pf);		//get lchild
				child.printPage();	
			}
			else {
				BTNonLeafNode child;
				printf("PID: %i\n", lchild); 
				child.read(nl.lChild(numChild), pf);
				child.printPage();	
			}
		}

		//print right pointers
		if(childLeaf) {
			BTLeafNode child;
			printf("PID: %i\n", rchild); 
			child.read(nl.rChild(numChild), pf);
			child.printPage();
		}
		else {
			BTNonLeafNode child;
			printf("PID: %i\n", rchild); 
			child.read(nl.rChild(numChild), pf);
			child.printPage();
		}
	}
}

PageId BTreeIndex::getRoot() {
	return rootPid;
}

PageFile* BTreeIndex::getPf() {
	return &pf;
}

void BTreeIndex::printMeta() {

	//get meta data
	int height = treeHeight + 1;
	int num_pids_used = pf.endPid() - 1;

	printf("---METADATA----------------------------------------------------\n");
	printf("Tree Height: %i, Root PID: %i, Number of Pages Used: %i\n\n", height, rootPid, num_pids_used);
}

#include "BTreeNode.h"
#include <cstdio>
#include <iostream>
#include <fstream>

using namespace std;

/* sech */
/* HELPER FUNCTIONS */
void BTLeafNode::setKeyCount(int kc) {
	memcpy(m_page + KEY_COUNT_INDEX, &kc, sizeof(int));
}


RC BTLeafNode::shiftRight(int eid) {

	//note: we shift the keys starting from rightmost key to avoid mistaken overwrites
	printf("eid is %i\n", eid);

	if(eid > NUM_KEYS_PER_PAGE) {
		printf("shiftRight: invalid eid!\n");
		return -1;
	}

	//get the index of the eid specified 
	int endIndex = eid * ENTRY_SIZE + META_SIZE;

	if(getKeyCount() == 0)
		return 0;

	//get the index of the last key in the node
	int index = (getKeyCount()-1) * ENTRY_SIZE + META_SIZE;

	printf("endIndex is %i, index is %i\n", endIndex, index);

	//begin shifting
	while(index >= endIndex) {

		//the destination index of where we will write to
		int dest_index= index + ENTRY_SIZE;
		if(dest_index > TEMP_KEY_INDEX) {
			printf("shiftRight: shift error!\n");
			return -1;
		}
		
		//copy the entry
		memcpy(m_page + dest_index, m_page + index, ENTRY_SIZE);
		
		//update the index
		index -= ENTRY_SIZE;
	}

	//zero out the eid entry
	memset(m_page + endIndex, '-', ENTRY_SIZE);
	getKeyCount();

	return 0;
	
}

RC BTLeafNode::shiftLeft(int eid) {

}
/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{
	RC rc; 
	if((rc = pf.read(pid, m_page)) < 0)
		return rc;

	//remember which pid we are
	m_pid = pid;

	return 0; 
}
    
/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{
	RC rc;
	if((rc = pf.write(pid, m_page)) < 0)
		return rc;	
	 return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	int keycount;

	//find out the keycount by looking in our main memory page
	memcpy(&keycount, m_page + KEY_COUNT_INDEX, META_SIZE);
	
	return keycount;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{

	int eid;

/*
	//scan all keys, look for first key that is greater than our key
	for(eid=0; eid < getKeyCount(); eid++) {

		//index of the key comparing with 
		int index = (eid*ENTRY_SIZE) + META_SIZE;

		//get the comparison key
		int comp_key;	
		memcpy(&comp_key, m_page + index, sizeof(int));

		//if we found a key greater than our current key, break
		if(comp_key > key) 
			break;
	}	
*/
	locate(key, eid);

		//NOTE: may have to shift inside if statement below, return err on overflow
		//to let caller handle overflow

		//shift all keys to the right of eid (i) to the right	
		shiftRight(eid);
		int offset = eid * ENTRY_SIZE + META_SIZE;
	
		//copy the key, rid to the entry slot
		memcpy(m_page + offset, &key, sizeof(int)); 	
		memcpy(m_page + offset + PID_OFFSET, &(rid.pid), sizeof(int));
		memcpy(m_page + offset + SID_OFFSET, &(rid.sid), sizeof(int));



	//case: no overflow (82 max before inserting)
	if(getKeyCount() < NUM_KEYS_PER_PAGE) {

		//increment the key count
		setKeyCount(getKeyCount() + 1);

		//may need to write to pagefile
	}

	//case: overflow (insert past 83)
	else {

		setKeyCount(getKeyCount() + 1);

		//NOTE!: may let parent deal with insert and split so we don't have to worry about it
		//in this function
		//printf("overflow!\n");

		//create a new sibling leafnode
		//BTLeafNode sibling;
		//sibling.resetPage();	
		//int siblingkey;

		//pass sibling key to parent...
		//
	}

 return 0; 
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{ 
	//insert the key into our node
	//NOTE: this means we have stored a key into our TEMP key
	insert(key, rid);	

	if(getKeyCount() < NUM_KEYS_PER_PAGE + 1) {
		printf("Warning...splitting when node is not full!\n");
	}

	//find middle entry
	int midIndex = (getKeyCount() + 1)/2;
	printf("midIndex = %i\n", midIndex);

	//move all keys to the right (and including) midEid to sibling node (includes temp entry)
	
	//find the number of keys we are copying (should be 42 each time)
	int numKeys = getKeyCount()/2;
	printf("copying %i keys to sibling\n", numKeys);

	//the index from which we are copying the keys
	int from_index = midIndex * ENTRY_SIZE + META_SIZE;
	//the index to which we are copying the keys
	int to_index = META_SIZE;

	//copy the keys
	memcpy(sibling.m_page + to_index, m_page + from_index, numKeys*ENTRY_SIZE);

	//set sibling key count
	sibling.setKeyCount(numKeys);
	
	//set the keys we moved to be 0
	memset(m_page + from_index, 0, numKeys*ENTRY_SIZE);

	//the first key of the sibling node after split
	memcpy(&siblingKey, sibling.m_page + to_index, sizeof(int));

	return 0; 
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
	//scan all keys, look for first key that is greater than our key
	for(eid=0; eid < getKeyCount(); eid++) {

		//index of the key comparing with 
		int index = (eid*ENTRY_SIZE) + META_SIZE;

		//get the comparison key
		int comp_key;	
		memcpy(&comp_key, m_page + index, sizeof(int));

		//if we found a key greater than our current key, break
		if(comp_key >= searchKey) 
			break;
	}

	return 0; 
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{
/*
	//find the key value of the entry given 
	memcpy(&key, m_page + (eid*ENTRY_SIZE), sizeof(int)); 

	//find the page of the given entry	
	memcpy(&(rid.pid), m_page + (eid*ENTRY_SIZE) - KEY_SIZE, sizeof(int));

	//create temporary buffer to look for the correct entry
	char datapage[PageFile::PAGE_SIZE];
*/

	/*readEntry according to if our nodes store sids*/
	if(eid + 1> NUM_KEYS_PER_PAGE) {
		printf("EID too large!\n");
		return -1;	
	}

	//find location of key given entry
	int index = (eid + 1) * ENTRY_SIZE;

	//get key value
	memcpy(&key, m_page + index, sizeof(int));

	//set the rid pid
	memcpy(&(rid.pid), m_page + index + PID_OFFSET, sizeof(int)); 

	//set the rid sid
	memcpy(&(rid.sid), m_page + index + SID_OFFSET, sizeof(int)); 

	return 0; 
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	int nextPid;
	memcpy(&nextPid, m_page + RIGHT_PTR_INDEX, sizeof(int));

	if(nextPid < 0) {
		printf("getNextNodePtr error!\n");
		return RC_INVALID_PID;
	}
	return nextPid; 
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{	
	if(pid <= 0) {
		printf("setNextNodePtr error!\n");
		return RC_INVALID_PID;
	}	
	memcpy(m_page + RIGHT_PTR_INDEX, &pid, sizeof(int)); 
	return 0;
 }


/* sect */
/* Tester Function*/

//set m_page to have contents of page
void BTLeafNode::setPage(const char * page) {

	//set our m_page to whatever the buffer "page" is
	memcpy(m_page, page, PageFile::PAGE_SIZE);
}

//print contents of our page
void BTLeafNode::printPage() {
	printf("************* LEAF NODE *************\n");
	printf("m_page: | (entryno) [key, sid, pid] |\n" , m_page);
	
	int kc;
	memcpy(&kc, m_page, sizeof(int));
	printf("[%i]", kc);

	for(int i=META_SIZE; i < TEMP_KEY_INDEX + ENTRY_SIZE; i+=ENTRY_SIZE) {
		int entryno = (i-4)%12;

		if(entryno == 0 ) {
			printf(" |");
			if(i == TEMP_KEY_INDEX)	
				printf("(t) ");
			else if (i < TEMP_KEY_INDEX)
				printf("(%i)", (i-4)/12);
			else
				printf("(u)");
		}
				
		//print key, sid, pid
		int key;
		memcpy(&key, m_page + i, sizeof(int));	
		int sid;
		memcpy(&sid, m_page + i + SID_OFFSET, sizeof(int));
		int pid;
		memcpy(&pid, m_page + i + PID_OFFSET, sizeof(int));

		printf("%i,%i,%i", key, sid, pid);

	}

	//print right ptr
	int rptr = getNextNodePtr();
	
	printf(" |[r] %i", rptr);
	printf("|\n\n");
	
}


//set our page to be all 0s
void BTLeafNode::resetPage() {
	memset(m_page, 0, PageFile::PAGE_SIZE);
}

/*------------------------------------------------------------------ */
/*------------------------------------------------------------------ */
/*------------------------------------------------------------------ */
/* END OF LEAF NODE SECTION*/
/*------------------------------------------------------------------ */
/*------------------------------------------------------------------ */
/*------------------------------------------------------------------ */
/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{
	RC rc; 
	if((rc = pf.read(pid, m_page)) < 0)
		return rc;

	//remember which pid we are
	m_pid = pid;

	return 0;
}
    
/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ 
	RC rc; 
	if((rc = pf.write(pid, m_page)) < 0)
		return rc;
	return 0;
 }

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
	int keycount;

	//find out the keycount by looking in our main memory page
	memcpy(&keycount, m_page + KEY_COUNT_INDEX, META_SIZE);
	return keycount;
 }


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{

	//find location to insert new key (note: pid[in] is the RIGHT CHILD of key)	
	int i;
	int index;
	for(i=0; i < getKeyCount(); i++) {
		index = i*ENTRY_SIZE + META_SIZE;

		//get the comparison key
		int comp_key;
		memcpy(&comp_key, m_page + index, sizeof(int));

		if(key <= comp_key)
			break; 
	}	
	
	printf("Found searchKey <= compKey at eid %i\n", i);

	//shift over all entries to the RIGHT of the eid we found
	shiftRight(i+1);

	index = i*ENTRY_SIZE + META_SIZE;

	//shift the key over by one entry
	memcpy(m_page + index + ENTRY_SIZE, m_page + index, sizeof(int)); 
	
	//insert new key at slot (key is in current slot, ptr is in slot to the right)
	memcpy(m_page + index, &key, sizeof(int));
	setRchild(i, pid);	

	setKeyCount(getKeyCount() + 1);
	
	return 0; 
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ 
	//insert the key/pid into our current node (will cause overflow)
	insert(key, pid);	
	
	//printPage();

	//find eid of key from which to copy (should be eid 42)
	int copyId = getKeyCount()/2; 
	int copyIndex = copyId * ENTRY_SIZE + META_SIZE;
	printf("Key to copy is at eid %i\n", copyId);

	//get the middle key	
	int midIndex = (copyId-1) * ENTRY_SIZE + META_SIZE;
	memcpy(&midKey, m_page + midIndex, sizeof(int));
	int oldRptr;
	memcpy(&oldRptr, m_page + midIndex + PID_OFFSET, sizeof(int));
	printf("midKey is %i\n", midKey);
	printf("oldRptr is %i\n", oldRptr);

	//copy everything to the right (not including) of midIndex to sibling buffer
	int numEntries = (NUM_KEYS_PER_PAGE + 1) - copyId; 		
	printf("copying %i entries to sibling node!\n", numEntries);
	memcpy(sibling.m_page + META_SIZE, m_page + copyIndex, numEntries * ENTRY_SIZE);   	
	sibling.setKeyCount(numEntries);

	//copy the right pointer over as well (special case!)
	int rptr = rChild(NUM_KEYS_PER_PAGE);
	sibling.setRchild(numEntries - 1, rptr);

	//reset the right ptr
	setRchild(NUM_KEYS_PER_PAGE, 0);

	//remove all the keys that we have copied over (including key)
	memset(m_page + midIndex, 0, (numEntries+1) * ENTRY_SIZE);
	setRchild(copyId-2, oldRptr);
	setKeyCount(NUM_KEYS_PER_PAGE - numEntries - 1);
	
	return 0; 
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ 
	//if less than X, take left child
	//otherwise, take right child		
	
	//walk through all current keys in the node and find the key
	//corresponding to searchKey
	int i;	
	for(i=0; i < getKeyCount(); i++) {

		//get the index of the next key 
		int index = i*ENTRY_SIZE + META_SIZE;		

		int comp_key;
		memcpy(&comp_key, m_page + index, sizeof(int));

		//if search key is less than comp key, then take left ptr
		if(searchKey < comp_key) {
			//printf("Found search key at eid %i\n", i);
			pid = lChild(i);	
			return 0;
		}
	}

	//if we couldn't find a key that is >= search key, take right ptr	
	if(i >= getKeyCount()) {
		//printf("Getting key to the right of eid %i\n", i);
		pid = rChild(i-1);
		return 0;		
	}
	else
		return -1;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ 
	//assuming our node is the root node

	//set key 
	int rootIndex = 0;	
	memcpy(m_page + META_SIZE, &key, sizeof(int));

	//set pid1
	memcpy(m_page + META_SIZE + PID_OFFSET, &pid1, sizeof(int));

	//set pid2
	int nextIndex = (rootIndex + 1)*ENTRY_SIZE + META_SIZE + PID_OFFSET;
	memcpy(m_page + nextIndex, &pid2, sizeof(int));

	//set num keys
	setKeyCount(1);

	return 0; 
}

//print contents of our page
void BTNonLeafNode::printPage() {
	printf("*********** NON LEAF NODE ***********\n");
	printf("m_page: | (entryno) <pid> sid |\n" , m_page);
	
	int kc;
	memcpy(&kc, m_page, sizeof(int));
	printf("keyCount[%i] ptrCount[%i]\n", kc, kc+1);

	int lptr;
	memcpy(&lptr, m_page + META_SIZE + PID_OFFSET, sizeof(int));
	printf("[l] <%i>", lptr);

	int i;
	for(i=0; i < NUM_KEYS_PER_PAGE + 1; i++) {
		int index = i*ENTRY_SIZE + META_SIZE;
						
		//print key, sid, pid
		int rchild = rChild(i);
		int key;
		memcpy(&key, m_page + index, sizeof(int));	
		

		if(i == NUM_KEYS_PER_PAGE)
			printf(" |(t) %i <%i>", key, rchild);
		else
			printf(" |(%i) %i <%i>", i, key, rchild);
	}

	printf("|\n\n");

}

//set our page to be all 0s
void BTNonLeafNode::resetPage() {
	memset(m_page, 0, PageFile::PAGE_SIZE);
}

void BTNonLeafNode::setKeyCount(int kc) {
	memcpy(m_page + KEY_COUNT_INDEX, &kc, sizeof(int));
}


//set the right child of entry eid
void BTNonLeafNode::setRchild(int eid, PageId pid) {

	//if we are setting the right child of the temp index, set right_ptr_index
	if(eid >= NUM_KEYS_PER_PAGE) {
		memcpy(m_page + RIGHT_PTR_INDEX, &pid, sizeof(int));
	}

	//otherwise set the pid of the entry to the right of current entry
	else {
		int index = (eid + 1) * ENTRY_SIZE + META_SIZE + PID_OFFSET;
		memcpy(m_page + index, &pid, sizeof(int));
	}
}


//return pid of right child
PageId BTNonLeafNode::rChild(int eid) {
	
	int child_pid;

	//check for special case
	if(eid >= NUM_KEYS_PER_PAGE) {
		memcpy(&child_pid, m_page + RIGHT_PTR_INDEX, sizeof(int));
	}
	else {
		int index = (eid + 1) * ENTRY_SIZE + META_SIZE + PID_OFFSET;
		memcpy(&child_pid, m_page + index, sizeof(int));
	}
	return child_pid;
}  

//return pid of left child
PageId BTNonLeafNode::lChild(int eid) {
	int child_pid;
	int index = eid*ENTRY_SIZE + META_SIZE + PID_OFFSET;
	memcpy(&child_pid, m_page + index, sizeof(int));
	return child_pid;
}

RC BTNonLeafNode::shiftRight(int eid) {

	//note: we shift the keys starting from rightmost key to avoid mistaken overwrites
	printf("eid is %i\n", eid);

	if(eid > NUM_KEYS_PER_PAGE) {
		printf("shiftRight: invalid eid!\n");
		return -1;
	}

	char buf[PageFile::PAGE_SIZE];

	//get the index of the eid specified 
	int from_index = eid * ENTRY_SIZE + META_SIZE;

	//calculate the number of bytes to copy
	int numBytes = TEMP_KEY_INDEX - from_index;

	//copy the temp ptr to the right ptr
	memcpy(m_page + RIGHT_PTR_INDEX, m_page + TEMP_KEY_INDEX + PID_OFFSET, sizeof(int));

	//copy bytes to temporary buffer 
	memcpy(buf, m_page + from_index, numBytes);

	//calculate the index of the eid to copy to
	int to_index = (eid+1) * ENTRY_SIZE + META_SIZE;

	//shift right
	memcpy(m_page + to_index, buf, numBytes);	

/*	if(getKeyCount() == 0)
		return 0;

	//get the index of the last key in the node
	int index = (getKeyCount()) * ENTRY_SIZE + META_SIZE;

	printf("endIndex is %i, index is %i\n", endIndex, index);

	//begin shifting
	while(index >= endIndex) {

		//the destination index of where we will write to
		int dest_index= index + ENTRY_SIZE;
		if(dest_index > TEMP_KEY_INDEX) {
			printf("shiftRight: shift error!\n");
			return -1;
		}
		
		//copy the entry
		memcpy(m_page + dest_index, m_page + index, ENTRY_SIZE);
		
		//update the index
		index -= ENTRY_SIZE;
	}
*/

	//zero out the eid entry
	memset(m_page + from_index, 0, ENTRY_SIZE);
	//getKeyCount();

	return 0;
	
}

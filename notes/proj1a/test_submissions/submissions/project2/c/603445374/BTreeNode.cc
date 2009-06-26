#include "BTreeNode.h"
#include <iostream>

using namespace std;

//BTLeafNode constructor
BTLeafNode::BTLeafNode()
{
  	int count = 0;	//set key count to 0
	memcpy(&buffer, &count, sizeof(int));
}

/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf) {
	if (pid < 0) return RC_INVALID_PID; 
	RC rc;
	if (rc = pf.read(pid, &buffer) != 0) {
		return rc;
	}
	return 0;
}
    
/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf) {
	if (pid < 0) return RC_INVALID_PID;
	RC rc;
	if (rc = pf.write(pid, &buffer) != 0) {
		return rc;
	}
	return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount() {
	int count;
	memcpy(&count, &buffer, sizeof(int)); 	// the first four bytes of the node in buffer contains # of keys in the node
	return count;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid) {
	RC rc;
	if (getKeyCount() >= BTLeafNode::MAX_PAIRS) {	//if node is full -> insertAndSplit
		return RC_NODE_FULL;
		/*BTLeafNode sibNode; //create new siblingNode
		int sibKey;
		if (rc = insertAndSplit(key, rid, sibNode, sibKey) != 0) { //call insertAndSplit with siblingNode
			return rc;
		}*/
	}
	else { // if node isn't full
		int pos;
		if (rc = locate(key, pos) != 0) { //find location to insert
			return rc;
		}
		
		//if insertion point is at the end, no shifting needed
		if (pos == getKeyCount()) {
			memcpy(&(buffer[4+pos*12]), &key, sizeof(int));	//insert new (key, rid) pair at pos
			memcpy(&(buffer[4+(pos*12)+4]), &rid, sizeof(int)*2);
		}
		
		//if insertion point is in the middle or beginning
		else {
			for (int i=getKeyCount(); i >= pos; i--) {	//traverse from largest eid down to pos 
				memcpy(&(buffer[4+(i+1)*12]), &(buffer[4+(i*12)]), sizeof(int)*3);	//shift last element over one
			}
			memcpy(&(buffer[4+(pos*12)]), &key, sizeof(int));	//insert new (key, rid) pair at pos
			memcpy(&(buffer[4+(pos*12)+4]), &rid, sizeof(int)*2);
		}
		
		//increment key count
		int kc;
		memcpy(&kc, &buffer, sizeof(int));	//get current key count
		kc++;	//increment key count	
		memcpy(&buffer, &kc, sizeof(int));	//set new key count
		
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
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, BTLeafNode& sibling, int& siblingKey) {
	//assume node is full
	RC rc;
	
	//split original node in half
	for (int i=0; i < 42; i++) {
		//put last 42 entries from original node into first 42 entries of siblingNode
		memcpy(&(sibling.buffer[4+i*12]), &(buffer[4+((i+42)*12)]), sizeof(int)*3);
	}
	
	//set key counts of both nodes
	int kc = 42;
	memcpy(&buffer, &kc, sizeof(int));	//for original node
	memcpy(&sibling.buffer, &kc, sizeof(int));	//for sibling node
	
	//BEGIN KEY INSERTION
	//compare key to last entry of original node and first entry of siblingNode
	int last;
	memcpy(&last, &(buffer[4+41*12]), sizeof(int));	//get key of last entry in original node
	if (key <= last) {
		//key is less than or equal to the largest value of original node, so insert in original node
		if (rc = insert(key, rid) != 0) return rc;
		//insert() will automatically increase key count
	}
	else {
		//otherwise, key is greater than  the smallest value of sibling node, so insert in sibling node
		if (rc = sibling.insert(key, rid) != 0) return rc;
		//insert() will automatically increase key count
	}
	
	//set sibling key
	memcpy(&siblingKey, &(sibling.buffer[4]), sizeof(int));

	//BEGIN UPDATE OF LEAF NODE POINTERS
	PageId temp = getNextNodePtr();	//save the current next node ptr using getNextNodePtr() to a temporary variable
	PageId sibPid = (int)&(sibling.buffer);	//cast address of beginning of the sibling node as an int
	setNextNodePtr(sibPid);	//set the last pointer of the original node to its new sibling node using setNextNodePtr()
	sibling.setNextNodePtr(temp);	//set the last pointer of the sibling node to the original node's next pointer using setNextNodePtr() with the temp pointer from before
	
	return 0;
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalt to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid) { 
	RC rc;
	int compare, i;
	//this could only happen in test cases
	if (getKeyCount() == 0) {
		eid = 0;
		return 0;
	}
	for (i=0; i < getKeyCount(); i++) {
		memcpy(&compare, &(buffer[4+i*12]), sizeof(int));	//temporarily store key to be compared with
		if (searchKey <= compare) {	//if searchKey is larger than or equal to compared variable
			eid = i;	//save position as eid
			return 0;	//exit
		}
	}
	//if searchKey is never <= any keys in the node, it goes in the end
	eid = getKeyCount();
	return 0;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid) {
	RC rc;
	//calculate entry point
	int entry = 4+(eid*12);	//ex. if eid=0, then enter buffer at location 4 after first 4 bytes that store amt of pairs
	memcpy(&key, &(buffer[entry]), sizeof(int));
	memcpy(&rid, &(buffer[entry+4]), sizeof(int)*2);
	return 0; 
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr() {
	int next;
	//get last 4 bytes of pf since that is the pointer to the next node's ptr
	memcpy(&next, &(buffer[1012]), sizeof(int));	//use sizeof(int) because PageID is an int as defined in PageFile.h
	return next;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid) { 
	//go to last 4 bytes of pf and set as pid
	memcpy(&(buffer[1012]), &pid, sizeof(int));
	return 0;
}

void BTLeafNode::printNode() {
	cout << endl << "Contents of leafnode" << endl;
	int tempKey = -1;
	cout << "keycount: " << getKeyCount() << endl;
	for (int i=0; i<getKeyCount(); i++) {
		memcpy(&tempKey, &(buffer[4+i*12]), sizeof(int));
		cout << "entry: " << i+1;
		cout << "  value: " << tempKey << endl;
	}
}

//BTNonLeafNode Constructor
BTNonLeafNode::BTNonLeafNode()
{
  	int count = 0;	//set key count to 0
	memcpy(&buffer, &count, sizeof(int));
}


/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf) {
	if (pid < 0) return RC_INVALID_PID; 
	RC rc;
	if (rc = pf.read(pid, &buffer) != 0) {
		return rc;
	}
	return 0;
}
    
/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf) { 
	if (pid < 0) return RC_INVALID_PID; 
	RC rc;
	if (rc = pf.write(pid, &buffer) != 0) {
		return rc;
	}
	return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount() {
	int count;
	memcpy(&count, &buffer, sizeof(int)); 	// the first four bytes of the node in buffer contains # of keys in the node
	return count;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid) { 
	RC rc;
	cout << "gets into insert" << endl;
	if (getKeyCount() >= BTNonLeafNode::MAX_PAIRS) {	//if node is full -> insertAndSplit
		return RC_NODE_FULL;
		/*BTNonLeafNode sibNode; //create new siblingNode
		int midKey;
		if (rc = insertAndSplit(key, pid, sibNode, midKey) != 0) { //call insertAndSplit with siblingNode
			return rc;
		}*/
	}
	else { // if node isn't full
		cout << "knows node isn't full" << endl;
		int pos=-1, compKey;
		
		//BEGIN FINDING OF INSERTION POINT
		if (getKeyCount() == 0) {
			pos = 0;	//this could only happen in test cases
		}
		else {
			//find key that is larger than or equal to key
			for (int i=0; i < getKeyCount(); i++) {
				memcpy(&compKey, &(buffer[8+8*i]), sizeof(int));	//temporarily store key to be compared with
				if (key <= compKey) {	//if searchKey is larger than or equal to compared key
					pos = i;
					break;
				}
			}
			if (pos == -1) {	//key is at the end of the node
				pos = getKeyCount();
			}
		}
		
		//BEGIN INSERTION PROCESS
		//if insertion point is at the end, no shifting needed
		if (pos == getKeyCount()) {
			memcpy(&(buffer[8+pos*8]), &key, sizeof(int));	//insert new (key, pid) pair at pos
			memcpy(&(buffer[8+(pos*8)+4]), &pid, sizeof(int));
		}
		
		//if insertion point is at the middle or beginning, shifting needed
		else { 
			for (int i=getKeyCount(); i >= pos; i--) {	//traverse from furthest down
				memcpy(&(buffer[8+(i+1)*8]), &(buffer[8+(i*8)]), sizeof(int)*2);	//shift last element over one
			}
			memcpy(&(buffer[8+pos*8]), &key, sizeof(int));	//insert new (key, pid) pair at pos
			memcpy(&(buffer[8+(pos*8)+4]), &pid, sizeof(int));
		}
		
		//increment key count
		int kc;
		memcpy(&kc, &buffer, sizeof(int));
		kc++;
		memcpy(&buffer, &kc, sizeof(int));
	}
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
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey) { 
	//assume node is full, so 127 full pairs
	RC rc;
	
	//ALGORITHM
	// 1) first split the 127 entries half and half excluding exact middle key (63/63/1)
		//original node keeps entries 0-62 = 63 total entries
		//midkey is 63 entry
		//sibling node gets entries 64-126 = 63 total entries
	// 2) see which node to insert new key into by checking if key is
		//<= to current mid key
			//insert new key in original node
		//> than current mid key
			//insert new key into sibling node
	
	//split original node in half
	for (int i=0; i < 63; i++) {
		//copy last 62 entries from original node into first 62 entries of sibling node
		memcpy(&(sibling.buffer[8+i*8]), &(buffer[8+((i+64)*8)]), sizeof(int)*2);
	}

	//set key counts of both nodes
	int kc = 63;
	memcpy(&buffer, &kc, sizeof(int));	//for original node
	memcpy(&sibling.buffer, &kc, sizeof(int));	//for sibling node
	
	//set midKey
	memcpy(&midKey, &(buffer[8+(63*8)]), sizeof(int));	//copy over middle entry key
	
	//BEGIN KEY INSERTION
	//compare key to mid key
	if (key <= midKey) {
		//key is less than or equal to current mid key, so insert key into original node
		if (rc = insert(key, pid) != 0) return rc;
		//insert() will automatically increase key count
	}
	else {
		//otherwise, key is greater than mid key, so insert in sibling node
		if (rc = sibling.insert(key, pid) != 0) return rc;
		//insert() will automatically increase key count
	}
	
	//resultant nodes will either have 63/64 or 64/63 entries
	
	return 0;
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid) {
	int amt;
	memcpy(&amt, &buffer, sizeof(int));	//get amount of pairs
	int comp1, comp2;
	for (int i=0; i < (amt-1); i++) {
		memcpy(&comp1, &(buffer[8+i*8]), sizeof(int));	//1st key to compare with
		memcpy(&comp2, &(buffer[8+(i+1)*8]), sizeof(int));	//2nd key to compare with
		if (i==0 && searchKey < comp1) {
			memcpy(&pid, &(buffer[4]), sizeof(int));	//less than 1st key
			return 0;
		}
		if (i>0 && searchKey >= comp1 && searchKey <= comp2) {	//searchKey is between 2 compared keys
			if(searchKey == comp2) {
				memcpy(&pid, &(buffer[8+(i+1)*8+4]), sizeof(int)); //use pid after comp2
				return 0;
			}
			//otherwise, return pid after comp1;
			memcpy(&pid, &(buffer[8+i*8+4]), sizeof(int));	//otherwise, return pid after comp1;
			return 0;
		}
	}
	return 0; 
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2) {
	int amt = 1;
	memcpy(&buffer, &amt, sizeof(int));	//set the first 4 bytes to the number of keys = 1
	memcpy(&(buffer[4]), &pid1, sizeof(int));	//set pid1 as next 4 bytes
	memcpy(&(buffer[8]), &key, sizeof(int));	//set key as next 4 bytes
	memcpy(&(buffer[12]), &pid2, sizeof(int));	//set pid2 as next 4 bytes
	return 0; 
}

void BTNonLeafNode::printNode() {
	cout << endl << "Contents of non-leaf node" << endl;
	int tempKey = -1;
	cout << "keycount: " << getKeyCount() << endl;
	for (int i=0; i<getKeyCount(); i++) {
		memcpy(&tempKey, &(buffer[8+i*8]), sizeof(int));
		cout << "entry: " << i+1;
		cout << "  value: " << tempKey << endl;
	}
}

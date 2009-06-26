#include "BTreeNode.h"

using namespace std;

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
		BTLeafNode sibnode; //create new siblingNode
		int sibkey;
		int kc = 1; //temp var for setting sibling key count
		memcpy(&sibnode.buffer, &kc, sizeof(int));	//set sibling key count as 1
		if (rc = insertAndSplit(key, rid, sibnode, sibkey) != 0) { //call insertAndSplit with siblingNode
			return rc;
		}
	}
	else { // if node isn't full
		int pos;
		if (locate(key, pos) != 0) { //find location to insert
			return -1;
		}
		for (int i=getKeyCount(); i <= pos; i--) {	//traverse from largest eid to pos down
			memcpy(&(buffer[4+(i+1)*12]), &(buffer[4+(i*12)]), sizeof(int)*4);	//shift last element over one
		}
		memcpy(&(buffer[4+pos]), &key, sizeof(int));	//insert new (key, rid) pair at pos
		memcpy(&(buffer[4+pos+4]), &rid, sizeof(int)*2);
		
		//increment key count
		int kc;
		memcpy(&kc, &buffer, sizeof(int));
		kc++;
		memcpy(&buffer, &kc, sizeof(int));
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
	RC rc;
	//assume node is full
	int tempkey;
	RecordId temprid;
	if (rc = readEntry(83, tempkey, temprid) != 0) {	//check if key is larger than or equal to the last key of the 1st node using readEntry()
		return rc;
	}
	if (key >= tempkey) {	//if it is
		siblingKey = key;	//set siblingKey as key
		return 0;
	}
	
	//if not
	tempkey = key; //set the last key of the original node as siblingKey
	
	//insert new key into original node
	int pos;
	if (locate(key, pos) != 0) { //find location to insert
		return -1;
	}
	for (int i=82; i <= pos; i--) {	//shift all values over 1, replacing the last (key, rid)
			memcpy(&(buffer[4+(i+1)*12]), &(buffer[4+(i*12)]), sizeof(int)*3);
	}
	memcpy(&(buffer[4+pos]), &key, sizeof(int));	//insert new (key, rid) pair at pos
	memcpy(&(buffer[4+pos+4]), &rid, sizeof(int)*2);
	//begin update of the node ptrs
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
	int compare;
	for (int i=0; i < getKeyCount(); i++) {
		memcpy(&compare, &(buffer[4+8*i]), sizeof(int));	//temporarily store key to be compared with
		if (searchKey >= compare) {	//if searchKey is larger than or equal to compared variable
			eid = i;	//save position as eid
			return 0;	//exit
		}
	}
	return -1; //return error code
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
	int temp;
	//get last 4 bytes of pf since that is the pointer to the next node's ptr
	memcpy(&temp, &(buffer[1012]), sizeof(int));	//use sizeof(int) because PageID is an int as defined in PageFile.h
	return temp;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid) { 
	RC rc;
	//go to last 4 bytes of pf and set as pid
	//set the last 4 bytes as pid
	memcpy(&(buffer[1012]), &pid, sizeof(int));
	return 0;
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
	if (rc = pf.read(pid, &buffer) != 0) {
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
	if (getKeyCount() >= BTNonLeafNode::MAX_PAIRS) {	//if node is full -> insertAndSplit
		BTNonLeafNode sibnode; //create new siblingNode
		int midkey;
		int kc = 1; //temp var for setting sibling key count
		memcpy(&sibnode.buffer, &kc, sizeof(int));	//set sibling key count as 1
		if (rc = insertAndSplit(key, pid, sibnode, midkey) != 0) { //call insertAndSplit with siblingNode
			return rc;
		}
	}
	else { // if node isn't full
		int pos, compare;	//find key the is larger than or equal to key
		for (int i=0; i < getKeyCount(); i++) {
			memcpy(&compare, &(buffer[8+8*i]), sizeof(int));	//temporarily store key to be compared with
			if (key >= compare) {	//if searchKey is larger than or equal to compared variable
				pos = i;
				break;
			}
		}
		for (int i=getKeyCount(); i <= pos; i--) {	//traverse from furthest down
			memcpy(&(buffer[8+(i+1)*8]), &(buffer[8+(i*8)]), sizeof(int)*2);	//shift last element over one
		}
		memcpy(&(buffer[8+pos*8]), &key, sizeof(int));	//insert new (key, pid) pair at pos
		memcpy(&(buffer[8+pos*8+4]), &pid, sizeof(int));
		
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
	RC rc;
	//assume node is full, so 127 full pairs
	int tempkey;
	RecordId temprid;
	
	//split node half and half
	//1st node = 0-62, midkey = 63, 2nd node = 64-126
	int mid = 63; //middle position
	int pos, compare;	//find key the is larger than or equal to key
	for (int i=0; i < getKeyCount(); i++) {
		memcpy(&compare, &(buffer[8+8*i]), sizeof(int));	//temporarily store key to be compared with
		if (key >= compare) {	//if searchKey is larger than or equal to compared variable
			pos = i;
			break;
		}
	}
	//shift 2nd half of 1st node over to the 2nd node
	for (int i=127; i <= mid; i--) {	//shift all values over 1, replacing the last (key, rid)
			memcpy(&(sibling.buffer[4+(i+1-mid)*8]), &(buffer[4+(i*8)]), sizeof(int)*2);
	}

	int tempnum=64;
	memcpy(&(sibling.buffer), &tempnum, sizeof(int));	//update num of pairs in 2nd node to 64
	tempnum=62;
	memcpy(&buffer, &tempnum, sizeof(int));	//update num of pairs in 1st node to 62
	memcpy(&midKey, &(buffer[8+(mid*8)]), sizeof(int));	//set midkey as the 63rd entry
	//then insert (key, pid) into whichever half
	if (pos < 63) {	//insert into 1st node
		for (int i=62; i <= pos; i--) {	//shift all values over 1, replacing the last (key, rid)
			memcpy(&(buffer[4+(i+1)*8]), &(buffer[4+(i*8)]), sizeof(int)*2);
		}
		memcpy(&(buffer[4+pos]), &key, sizeof(int));	//insert new (key, rid) pair at pos
		memcpy(&(buffer[4+pos+4]), &pid, sizeof(int));
		int kc;	//increment key count
		memcpy(&kc, &buffer, sizeof(int));
		kc++;
		memcpy(&buffer, &kc, sizeof(int));
	}
	if (pos > 63) {	//insert into sibling node
		pos -= 127;
		for (int i=64; i <= pos; i--) {	//shift all values over 1, replacing the last (key, rid)
			memcpy(&(sibling.buffer[4+(i+1)*8]), &(sibling.buffer[4+(i*8)]), sizeof(int)*2);
		}
		memcpy(&(sibling.buffer[4+pos]), &key, sizeof(int));	//insert new (key, rid) pair at pos
		memcpy(&(sibling.buffer[4+pos+4]), &pid, sizeof(int));
		int kc;	//increment key count
		memcpy(&kc, &sibling.buffer, sizeof(int));
		kc++;
		memcpy(&sibling.buffer, &kc, sizeof(int));
	}
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
		memcpy(&comp1, &(buffer[8+8*i]), sizeof(int));	//1st key to compare with
		memcpy(&comp2, &(buffer[8+8*(i+1)]), sizeof(int));	//2nd key to compare with
		if (i==0 && searchKey < comp1) {
			memcpy(&pid, &(buffer[4]), sizeof(int));	//less than 1st key
			return 0;
		}
		if (searchKey >= comp1 && searchKey <= comp2) {	//searchKey is between 2 compared keys
			if(searchKey == comp2) {
				memcpy(&pid, &(buffer[8+8*(i+1)+4]), sizeof(int)); //use pid after comp2
				return 0;
			}
			//otherwise, return pid after comp1;
			memcpy(&pid, &(buffer[8+8*i+4]), sizeof(int));	//otherwise, return pid after comp1;
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
	RC rc;
	int amt = 1;
	memcpy(&buffer, &amt, sizeof(int));	//set the first 4 bytes to the number of keys = 1
	memcpy(&(buffer[4]), &pid1, sizeof(int));	//set pid1 as next 4 bytes
	memcpy(&(buffer[8]), &key, sizeof(int));	//set key as next 4 bytes
	memcpy(&(buffer[12]), &pid2, sizeof(int));	//set pid2 as next 4 bytes
	return 0; 
}

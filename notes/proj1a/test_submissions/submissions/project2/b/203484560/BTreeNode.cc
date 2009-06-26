#include "BTreeNode.h"
#include <iostream>

using namespace std;

/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ 
	//Note: Only reads first part of file, so the file must be flushed each time.
	char buffer[PageFile::PAGE_SIZE];
	RC rc = pf.read(pid, buffer);
	if (rc == 0) {
		memcpy((void*)&numKeys, (void*)buffer, sizeof(int));
		memcpy((void*)tuplePointers, (void*)(buffer + sizeof(int)), sizeof(RecordId)*MAX_ELEMENTS);
		memcpy((void*)searchKeys, (void*)(buffer + sizeof(int) + sizeof(RecordId)*MAX_ELEMENTS), sizeof(int)*MAX_ELEMENTS);
		memcpy((void*)&nextLeafNode, (void*)(buffer + sizeof(int) + sizeof(RecordId)*MAX_ELEMENTS + sizeof(int)*MAX_ELEMENTS), sizeof(PageId));
	}
	else
		fprintf(stderr, "Error %d in BTLeafNode::read- Read failed.\n", rc);
		
	return rc;
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
	char buffer[PageFile::PAGE_SIZE];
	char* cursor = buffer;		//character is one byte.  Keep track of pointer arithmetic

	//Copy numKeys
	memcpy((void*)buffer, (void*)&numKeys, sizeof(int));
	cursor += sizeof(int);

	//Copy tuplePointers
	memcpy((void*)cursor, (void*)(tuplePointers), sizeof(RecordId)*MAX_ELEMENTS);
	cursor += sizeof(RecordId)*MAX_ELEMENTS;
	
	//Copy searchKeys
	memcpy((void*)cursor, (void*)searchKeys, sizeof(int)*MAX_ELEMENTS);
	cursor += sizeof(int)*MAX_ELEMENTS;

	//Copy nextLeafNode
	memcpy((void*)cursor, (void*)&nextLeafNode, sizeof(PageId));
	cursor += sizeof(PageId);

	//8 bytes left

	RC rc = pf.write(pid, (void*)buffer);
	return rc;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
	return numKeys;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
	//This is based on binary sort algorithm
	//Perform binary search on sorted section.
	//Insert key at appropriate location.
	//Each insert keeps keys sorted.
	

	RC rc = 0;
	if (numKeys < MAX_ELEMENTS) {
		if (numKeys == 0) {
			//Inserting the first item
			searchKeys[0] = key;
			tuplePointers[0] = rid;
		}
		else {
			int eid;
			RC locateCondition =  locate(key, eid);
			
			//Check if the key to insert is greater than all keys
			if (locateCondition == RC_NO_SUCH_RECORD)
				eid = numKeys;
			else if (locateCondition != 0) {
				fprintf(stderr, "Error: in BTLeafNode::insert. Locate failed.\n");
				return locateCondition;
			}

			if (eid > -1) {
				shiftKeysRight(eid);
				searchKeys[eid] = key;
				tuplePointers[eid] = rid;
			}
			else {
				fprintf(stderr, "Error: in BTLeafNode::insert\n");
				return -1;
			}
		}
		numKeys++;
	}
	else {

		rc = RC_NODE_FULL;
	}
	
	return rc;
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
	//Note: If the key being inserted happens to be the middle key,
	//		the keys will be split up (in two different nodes)
	RC rc = 0;
	if (sibling.numKeys || numKeys != MAX_ELEMENTS)
		rc = RC_INVALID_ATTRIBUTE;
	else {
		//Loop initialization
		bool inLeft = true;		//the key belongs in the left (original) node
		int i = MAX_ELEMENTS/2;
		int j = 0;
		
		if (key > searchKeys[i]) {
			i++;
			inLeft = false;
		}
		numKeys = i;

		//Move to the sibling node
		while (i < MAX_ELEMENTS) {
			sibling.searchKeys[j] = searchKeys[i];
			sibling.tuplePointers[j] = tuplePointers[i];
		
			//Increment the loop
			i++;
			j++;
		}

		//Fix the state of sibling
		sibling.numKeys = j;

		if (inLeft)
			rc = insert(key, rid);
		else
			rc = sibling.insert(key, rid);

		//Set output parameter
		siblingKey = sibling.searchKeys[0];
	}
	return 0;
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * NOTE: RC_NO_SUCH_RECORD indicates that the saerchKey is greater than all keys
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
	RC rc = 0;
	int index = binarySearch(searchKey, 0, numKeys-1);
	if (index >= numKeys) {
		//Left as error, calling function should check.
		//	Don't want to automatically add to next leaf node,
		//	because unsure how last leaf node will be handled.
		rc = RC_NO_SUCH_RECORD;
	}
	else {
		do{index--;}while (index >= 0 && searchKeys[index] == searchKey);
		eid = index+1;
	}
	return rc;
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
	if (0 <= eid && eid < numKeys) {
		key = searchKeys[eid];
		rid = tuplePointers[eid];
		return 0;
	}
	return RC_INVALID_CURSOR;
}

/*
 * Output the pid of the next sibling node.
 * @return 0 if successful. Return an error code if there is an error.  -I don't think this applies.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	return nextLeafNode;
}

/*
 * Set the pid of the next sibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	RC rc = 0;
	if (pid < 0)
		rc = RC_INVALID_PID;
	nextLeafNode = pid;
	return rc;
}

/*
 * Find the index of the entry with its search key >= key (input)
 * NOTE: If a key is greater than all entries, returns numKeys (not valid)
 * @param key[IN] the key to find
 * @param lower[IN] the eid of the first entry
 * @param upper[IN] the eid of the last entry
 * @return the eid where the key is found, -1 if error
 */
int BTLeafNode::binarySearch(int key, int lower, int upper)
{
	int index;	//the eid where the key can be found
	if (lower > upper)
		index = -1;
	else if (key > searchKeys[upper])
		index = upper + 1;		//This will return past the end of the array
		//index = upper+1 < numKeys ? upper+1 : upper;	//Don't add if it exceeds numKeys
	else if (key < searchKeys[lower])
		index = lower;
	else {
		index = (lower+upper)/2;	//Integer division.
		if (key < searchKeys[index])
			index = BTLeafNode::binarySearch(key, lower, index-1);
		else if (key > searchKeys[index])
			index = BTLeafNode::binarySearch(key, index+1, upper);
	}
	return index;
}

/*
 * Shift all keys and pointers starting from eid to the right one space
 * NOTE: There must be space to move keys and pointers one space to the right.
 * @param eid[IN] the first key to move
 * @return 0 if success, error otherwise
 */
RC BTLeafNode::shiftKeysRight(int eid)
{
	RC rc = 0;
	if (numKeys == MAX_ELEMENTS)
		rc = RC_NODE_FULL;
	else
		for (int i = numKeys; i > eid; i--)
		{
			searchKeys[i] = searchKeys[i-1];
			tuplePointers[i] = tuplePointers[i-1];
		}
	return rc;
}



/*
 * Find the index of the first entry in keys[ ] larger than key.
 * @param key[IN] the key to find
 * @return the index in keys[ ] of the first entry larger than key, or if none exists, the number of elements filled in keys[ ]
 */
int BTNonLeafNode::binarySearch(int key)
{	
	if(numKeys < 1)
		return 0;
	
	int lower(0), upper(numKeys - 1);

	while(lower < upper)
	{
		if(key >= keys[lower + (upper - lower)/2])
			lower += 1 + (upper - lower)/2;
		else
			upper = lower + (upper - lower)/2;
	}
	
	if(key >= keys[lower])
		return (lower + 1);
	
	return lower;
}

 /*
 * Shift all entries in keys[ ] and pids[ ] that are in indexes >= index to one element greater
 * @param index[IN] the index to start the shifting at
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::shiftRight(int index)
{
	RC rc = 0;
	cerr << "index: " << index << endl;
	cerr << "numKeys: " << getKeyCount() << endl;
	if(numKeys < 1)
		return rc;
	if(numKeys >= MAX_KEYS)
	{
		rc = RC_NODE_FULL;
		fprintf(stderr, "Error %d in BTNonLeafNode::shiftRight -- Node full.\n", rc);
	}
	else if(index < 0)
	{
		rc = RC_NO_SUCH_RECORD;
		fprintf(stderr, "Error %d in BTNonLeafNode::shiftRight -- No such record.\n", rc);
	}
	else
	{
		int i = numKeys - 1;
		
		while(i >= index)
		{
			pids[i + 2] = pids[i + 1];			//want to move the pid located after the key
			keys[i + 1] = keys[i];
			i--;
		}
	}
	return rc;
}

 /*
 * Copy all entries in the NonLeafNode from the start index until the end into the sibling node.
 * The sibling node must be empty.
 * @param start[IN] the index to start the copying from
 * @return 0 if successful. Return an error code if sibling is not empty.
 */
RC BTNonLeafNode::copy(int start, BTNonLeafNode& sibling)
{	
	RC rc = 0;
	
	if(sibling.numKeys > 0)
	{
		rc = RC_INVALID_ATTRIBUTE;
		fprintf(stderr, "Error %d in BTNonLeafNode::copy -- Invalid attribute. Sibling node is not empty.\n", rc);
		return rc;
	}
	
	if(start < 0)
	{
		rc = RC_INVALID_ATTRIBUTE;
		fprintf(stderr, "Error %d in BTNonLeafNode::copy -- Invalid attribute. \nCannot copy negatively indexed elements.\n", rc);
		return rc;
	}
	
	for(sibling.numKeys = 0; (start + sibling.numKeys) < numKeys; sibling.numKeys++)
	{
		sibling.keys[sibling.numKeys] = keys[start + sibling.numKeys];
		sibling.pids[sibling.numKeys] = pids[start + sibling.numKeys];
	}
	
	sibling.pids[sibling.numKeys] = pids[numKeys];		//set last pid
	
	numKeys = start;	//set new numKeys
	
	return rc;
}
	
/*
 * Read the content of the page pid in 
 * the PageFile pf to the node.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ 
	RC rc = 0;
	
	int buffer[PageFile::PAGE_SIZE/sizeof(int)];
	int* cursor = buffer;
	
	//read the page to the buffer
	rc = pf.read(pid, buffer);
	
	if(rc == 0)			//if no error code
	{	
		//Get numKeys from buffer
		memcpy(&numKeys, cursor, sizeof(numKeys));
		cursor++;					//numKeys  must be size of int
		
		//Get pids[ ] from buffer
		memcpy(pids, cursor, MAX_PID);
		cursor += MAX_PID;
		
		//Get  keys[ ] from buffer
		memcpy(keys, cursor, MAX_KEYS);
		cursor += MAX_KEYS;
	}
	else
		fprintf(stderr, "Error %d in BTNonLeafNode::read.\n", rc);

	return rc; 
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
	RC rc = 0;
	
	int buffer[PageFile::PAGE_SIZE/sizeof(int)];
	int* cursor = buffer;
	
	//Copy numKeys to buffer
	memcpy(cursor, &numKeys, sizeof(numKeys));
	cursor++;					//numKeys  must be size of int
	
	//Copy pids[ ] to buffer
	memcpy(cursor, pids, MAX_PID);
	cursor += MAX_PID;
	
	//Copy keys[ ] to buffer
	memcpy(cursor, keys, MAX_KEYS);
	cursor += MAX_KEYS;
	
	//write the buffer to the disk
	rc = pf.write(pid, buffer);
	
	if(rc)			//if rc is an error
		fprintf(stderr, "Error %d in BTNonLeafNode::write.\n", rc);
	
	return rc; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
	return numKeys; 
}


/*
 * Insert the (key, pid) pair to the node.
 * Remember that all keys inside a B+tree node should be kept sorted.
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 
	RC rc = 0;
	
	if(numKeys < MAX_KEYS)			//if node has space
	{	
		int ip = binarySearch(key);		//find insertion point
		if(!(rc = shiftRight(ip)));				// search for node to insert into
		{
			pids[ip + 1] = pid;
			keys[ip] = key;
		}
		numKeys++;
	}
	else		//node is full
	{
		rc = RC_NODE_FULL;
		fprintf(stderr, "Error %d in BTNonLeafNode::insert -- Node full.\n", rc);
	}
	return rc; 
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
	RC rc =0;
	
	if(sibling.numKeys)			//if the sibling node is not empty 
	{
		rc = RC_INVALID_ATTRIBUTE;
		fprintf(stderr, "Error %d in BTNonLeafNode::insertAndSplit -- Invalid attribute. Sibling node is not empty.\n", rc);
		return rc;
	}
	
	if(numKeys < 2)			//if the current node has less than two elements
	{
		rc = RC_INVALID_ATTRIBUTE;
		fprintf(stderr, "Error %d in BTNonLeafNode::insertAndSplit -- Invalid attribute. Cannot split node with fewer than two elements.\n", rc);
		return rc;
	}
	
	int fromMiddle = binarySearch(key) - ((numKeys/2) + 1);	 	//do a search for it and see where insertion should be in relation to middle of node
	
	if(fromMiddle < 0)							//if new key goes to left of middle
	{
		midKey = keys[numKeys/2];				//set the midkey
		copy((numKeys/2) + 1, sibling);
		numKeys--;								//essentially delete middle node
		insert(key, pid);
	}
	else if(fromMiddle > 0)						//if new key goes to right of middle
	{
		midKey = keys[(numKeys/2) + 1];			//set the midkey
		copy((numKeys/2) + 2, sibling);
		numKeys--;								//essentially delete middle node
		sibling.insert(key, pid);
	}
	else if(fromMiddle == 0)					//if new key goes to middle
	{
		midKey = key;							//set the midkey
		copy((numKeys/2) + 1, sibling);
		sibling.pids[0] = pid;					//have to specially set first pid of sibling node for this case
	}
	
	return rc; 
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
	RC rc = 0;
	bool higher;
	
	if(numKeys < 1)			//if there are no keys
	{
		rc = RC_NO_SUCH_RECORD;
		fprintf(stderr, "Error %d in BTNonLeafNode::locateChildPtr -- No such record.\n", rc);
		return rc;
	}
	
	pid = pids[binarySearch(searchKey)];
	
	return rc; 
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
	RC rc = 0;
	
	if(numKeys > 0)
	{
		rc = RC_INVALID_ATTRIBUTE;
		fprintf(stderr, "Error %d in BTNonLeafNode::initializeRoot -- Invalid attribute. Node is not empty.\n", rc);
		return rc;
	}
	
	pids[0] = pid1;
	pids[1] = pid2;
	keys[0] = key;
	numKeys = 1;
	return 0;
}


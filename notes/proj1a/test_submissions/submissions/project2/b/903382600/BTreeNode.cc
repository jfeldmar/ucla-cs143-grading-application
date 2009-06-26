#include "BTreeNode.h"

using namespace std;

//constructor, initialize keycount to 0
BTLeafNode::BTLeafNode()
{
	keycount = 0;
}


//Debugging function
void BTLeafNode::debug()
{
	int counter;
	RecordId testrid;
	int testkey;
	char* pointer;
	pointer = content;
	cout << "keycount: " << keycount << endl;
	for (counter = 0; counter < keycount; counter++){
		memcpy(&testrid, pointer, sizeof(RecordId));
		pointer = pointer + sizeof(RecordId);
		memcpy(&testkey, pointer, sizeof(int));
		pointer = pointer + sizeof(int);
		cout << "Section " << counter << ":" << endl;
		cout << "rid.sid: " << testrid.sid << endl;
		cout << "rid.pid: " << testrid.pid << endl;
		cout << "key: " << testkey << endl;
	}
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
	int errcode;
	keycount = 0;
	
	errcode = pf.read(pid, content);

	if (errcode != 0)
	  return errcode;
	
	char* ptr;
	ptr = content;
	ptr = ptr + sizeof(RecordId);
	int counter = 0;
	int testint;
	memcpy(&testint, ptr, sizeof(int));	
	while(testint >= -999999999){
		keycount++;
		ptr = ptr + sizeof(RecordId) + sizeof(int);
		memcpy(&testint, ptr, sizeof(int));
   } 
	return errcode; 
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
	int errcode;
	errcode = pf.write(pid, content);
	return errcode;
}

/*
 * Recounts the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
	keycount = 0;
	char* ptr;
	char* endptr = content + PageFile::PAGE_SIZE;
	ptr = content;
	ptr = ptr + sizeof(RecordId);
	int counter = 0;
	int testint;
	memcpy(&testint, ptr, sizeof(int));	
	while(testint >= -999999999){
		keycount++;
		ptr = ptr + sizeof(RecordId) + sizeof(int);
		if (ptr > endptr)
		  return keycount;
		memcpy(&testint, ptr, sizeof(int));
    }
    
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
	char* pointer;
	pointer = content;
	 
	//check if full
	if (keycount == (MAX_LEAF_POINTER - 1))
	  return RC_NODE_FULL;
	
	//If no existing keys, insert into the beginning of the node
	if (keycount == 0) {
		memcpy(pointer, &rid, sizeof(RecordId));
		pointer = pointer + sizeof(RecordId);
		memcpy(pointer, &key, sizeof(int));
		keycount++;
		return 0;
	}
	
	if (keycount > 0) {
		int counter;
		int tempkey;
		char* temppointer;
		pointer = pointer + sizeof(RecordId);
		for (counter=0; counter < keycount; counter++) {
			memcpy(&tempkey, pointer, sizeof(int));
			if (key < tempkey) {
				temppointer = pointer - sizeof(RecordId);
				pointer = pointer + sizeof(int);
				memcpy(pointer, temppointer, ((keycount - counter) * 12));
				memcpy(temppointer, &rid, sizeof(RecordId));
				temppointer = temppointer + sizeof(RecordId);
				memcpy(temppointer, &key, sizeof(int));
				keycount++;
				return 0;
			}
			pointer = pointer + sizeof(int) + sizeof(RecordId);
		}
		pointer = pointer - sizeof(RecordId);
		memcpy(pointer, &rid, sizeof(RecordId));
		pointer = pointer + sizeof(RecordId);
		memcpy(pointer, &key, sizeof(int));
		keycount++;
		return 0;
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
	char* originalptr;
	char* siblingptr;
	
	originalptr = content + 504;
	siblingptr = sibling.content;
	
	memcpy(siblingptr, originalptr, PageFile::PAGE_SIZE - 504); //Copy the bigger portion to the sibling
	
	siblingptr = siblingptr + sizeof(RecordId);
	memcpy(&siblingKey, siblingptr, sizeof(int));
	sibling.getKeyCount();
	
	if(key < siblingKey)
	  insert(key, rid);
	else
	  sibling.insert(key, rid);
	  
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
	int counter;
	int refkey;
	char* ptr = content + sizeof(RecordId);
	memcpy(&refkey, ptr, sizeof(int));
	ptr = ptr + sizeof(RecordId) + sizeof(int);
	for (counter = 0; refkey < searchKey; counter++) {
		memcpy(&refkey, ptr, sizeof(int));
		ptr = ptr + sizeof(RecordId) + sizeof(int);
		if (counter > keycount){
			eid = -1;
			return RC_NO_SUCH_RECORD;
		}
	}
	eid = counter;
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
	if (eid < 0){
		return RC_NO_SUCH_RECORD;
	}
	
	char* ptr = content + ((sizeof(RecordId) + sizeof(int)) * eid);
	memcpy(&rid, ptr, sizeof(RecordId));
	ptr = ptr + sizeof(RecordId);
	memcpy(&key, ptr, sizeof(int));
	return 0;
}

/*
 * Output the pid of the next sibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	PageId sibling;
	char* ptr;
	ptr = content + PageFile::PAGE_SIZE - sizeof(int);
	memcpy(&sibling, ptr, sizeof(int));
	return sibling; 
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	char* ptr;
	ptr = content + PageFile::PAGE_SIZE - sizeof(int);
	memcpy(ptr, &pid, sizeof(int));
	return 0;
}

/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{   
	int errcode;
	keycount = 0;
	
	errcode = pf.read(pid, content);

	
	char* ptr;
	ptr = content;
	ptr = ptr + sizeof(int);
	int counter = 0;
	int testint;
	memcpy(&testint, ptr, sizeof(int));	
	while(testint >= -999999999){
		keycount++;
		ptr = ptr + sizeof(int) + sizeof(int);
		memcpy(&testint, ptr, sizeof(int));
   } 
	return errcode; 
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
	int errcode;
	errcode = pf.write(pid, content);
	return errcode;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
	keycount = 0;
	char* ptr;
	char* endptr = content + PageFile::PAGE_SIZE;
	ptr = content;
	ptr = ptr + sizeof(int);
	int counter = 0;
	int testint;
	memcpy(&testint, ptr, sizeof(int));	
	while(testint >= -999999999){
		keycount++;
		ptr = ptr + sizeof(int) + sizeof(int);
		if (ptr > endptr)
		  return keycount;
		memcpy(&testint, ptr, sizeof(int));
    }
    
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
	//Check if there's room
	if (keycount == (MAX_NONLEAF_POINTER - 1))
	  	return RC_NODE_FULL;
	
		char* pointer;
		pointer = content;
	 	int counter;
		int tempkey;
		char* temppointer;
		
		pointer = pointer + sizeof(int);  //Set the pointer to point at first key
		
		for (counter=0; counter < keycount; counter++) {
			memcpy(&tempkey, pointer, sizeof(int)); //Get the key
			
			if (key < tempkey) {  //If key to be inserted is less than current key
				//Shift the entire node forward to make room for key to be inserted
				temppointer = pointer;
				pointer = pointer + sizeof(int) + sizeof(int);
				memcpy(pointer, temppointer, ((keycount - counter) * 8));
				
				//Insert the key and pid
				memcpy(temppointer, &key, sizeof(int));
				temppointer = temppointer + sizeof(int);
				memcpy(temppointer, &pid, sizeof(int));
				keycount++;
				return 0;
			}
			pointer = pointer + sizeof(int) + sizeof(int);
		}
		
		memcpy(pointer, &key, sizeof(int));
		pointer = pointer + sizeof(int);
		memcpy(pointer, &pid, sizeof(int));
		keycount++;
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
	char* originalptr;
	char* siblingptr;
	
	originalptr = content + 512 - 4; //Divide the node in half, give the bigger half to the sibling node
	siblingptr = sibling.content;
	
	memcpy(siblingptr, originalptr, PageFile::PAGE_SIZE - 512 + 4);
	
	memcpy(&midKey, originalptr - 4, sizeof(int)); //The middle key is the last key of the original node
	sibling.getKeyCount(); //Initialize the keycount for sibling node
	
	if(key > midKey) //If key is greater than mid key, insert it into sibling node
	  sibling.insert(key, pid);
	else
	  insert(key, pid);
	  
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
	int counter;
	int refkey;
	char* ptr = content + sizeof(int);
	
	for (counter = 0; counter < keycount; counter++) { //Search through the keys
		memcpy(&refkey, ptr, sizeof(int));
		
		if (searchKey < refkey){  //If the search key is less than the node, go down left pointer
			ptr = ptr - sizeof(int);
			memcpy(&pid, ptr, sizeof(int));
			return 0;
		}	
		
		ptr = ptr + sizeof(int) + sizeof(int);
	}
	
	//If the search key is greater than all nodes, go down the last pointer
	ptr = ptr - sizeof(int);
	memcpy(&pid, ptr, sizeof(int));
	
	return 0; 
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
	char* ptr = content;
	memcpy(ptr, &pid1, sizeof(int));
	ptr = ptr + sizeof(int);
	memcpy(ptr, &key, sizeof(int));
	ptr = ptr + sizeof(int);
	memcpy(ptr, &pid2, sizeof(int));
	return 0;
}

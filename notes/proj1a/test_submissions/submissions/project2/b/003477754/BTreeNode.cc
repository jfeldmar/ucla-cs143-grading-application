#include "BTreeNode.h"

#include <iostream>
#include <string>

using namespace std;

//##### START OF PROXY FUNCTIONS #####

BTLeafNode::BTLeafNode()
{
	//Initialize entry count to 0
	int entryCount = 0;
	memcpy(page+4, &entryCount, sizeof(int));
	
	//Initialize node type to 1 for leafNode
	char nodeType = 1;
	memcpy(page, &nodeType, sizeof(char));
	
}

BTNonLeafNode::BTNonLeafNode()
{
	//Initialize entry count to 0
	int entryCount = 0;
	memcpy(page+4, &entryCount, sizeof(int));
	
	//Initialize node type to 0 for nonLeafNode
	char nodeType = 0;
	memcpy(page, &nodeType, sizeof(char));
	
}

void BTLeafNode::printContent()
{
	cout << "start printContent()" << endl;
	
	int n = 0;
	int keyValue = 0;

	char *ptr = page + FIRST_ENTRY_OFFSET;
	
	cout << "getKeyCount: " << getKeyCount() << endl;
	
	while( n < getKeyCount() )
	{
		memcpy(&keyValue, ptr+8, sizeof(int));
		cout << "key: " << keyValue << endl;
		
		ptr+=12;
		n++;
	}
	
	cout << "done with printContent()" << endl;
}

void BTNonLeafNode::printContent()
{
	cout << "start printContent() for NonLeafNode" << endl;
	
	int n = 0;
	int keyValue = 0;
	int pidValue = 0;
	
	//set ptr to leftmost pid
	char *ptr = page + FIRST_ENTRY_OFFSET;
	
	cout << "getKeyCount: " << getKeyCount() << endl;
	
	//output the leftmost pid
	memcpy(&pidValue, ptr, sizeof(int));
	cout << "leftmost pid: " << pidValue << endl;
	
	//move ptr to point to the first (key, pid) pair
	ptr+=4;
	
	while( n < getKeyCount() )
	{
		memcpy(&keyValue, ptr, sizeof(int));
		memcpy(&pidValue, ptr+4, sizeof(int)); 
		cout << "entry # " << n << " key value: " << keyValue << " \t pid : " << pidValue << endl;
		
		ptr+=8;
		n++;
	}
	
	cout << "done with printContent() for NonLeafNode" << endl;
}

char* BTLeafNode::get_buffer(){ return page;}
char* BTNonLeafNode::get_buffer(){ return page;}

PageId BTLeafNode::getParentPtr()
{
	PageId ptr;
	memcpy(&ptr, page+8, sizeof(int));
	return ptr;
}

PageId BTLeafNode::getSelfPtr()
{
	PageId ptr;
	memcpy(&ptr, page+12, sizeof(int));
	return ptr;
}

void BTLeafNode::setParentPtr(PageId ptr){ memcpy(page+8, &ptr, sizeof(int)); }
void BTLeafNode::setSelfPtr(PageId ptr){ memcpy(page+12, &ptr, sizeof(int)); }

PageId BTNonLeafNode::getParentPtr()
{
	PageId ptr;
	memcpy(&ptr, page+8, sizeof(int));
	return ptr;
}

PageId BTNonLeafNode::getSelfPtr()
{
	PageId ptr;
	memcpy(&ptr, page+12, sizeof(int));
	return ptr;
}

void BTNonLeafNode::setParentPtr(PageId ptr){ memcpy(page+8, &ptr, sizeof(int)); }
void BTNonLeafNode::setSelfPtr(PageId ptr){ memcpy(page+12, &ptr, sizeof(int)); }

//##### END OF PROXY FUNCTIONS #####

/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ 
	RC   rc;
	if ((rc = pf.read(pid, page)) < 0) 	{return rc;}	
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
	RC 	rc;
	if ((rc = pf.write(pid, page)) < 0) {return rc;}
	return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
	int count;
	memcpy(&count, page+4, sizeof(int));
	return count; 
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 

	int count = getKeyCount();
	//If the node is full
	if(count == MAX_ENTRY) { return RC_NODE_FULL; }
	
	//Find the correct entry location
	int i = 0;
	int value;

	//Points to the first entry in a node
	char *ptr = page + FIRST_ENTRY_OFFSET;
	
	while( i < count )
	{
		memcpy(&value, ptr+8, sizeof(int));
		
		//If the value is the same as key
		if( value == key) { cout << "invalid_attribute: " << value << endl;
							return RC_INVALID_ATTRIBUTE; }
		//Entry location found
		else if (key < value)
		{
			//set pointer to point to the last entry
			ptr = page + FIRST_ENTRY_OFFSET + ((count-1) * 12);
			
			while( i < count)
			{
				memcpy(ptr+12, ptr, 8);	
				memcpy(ptr+20, ptr+8, sizeof(int));
				
				//set pointer to the earlier entry
				ptr-=12;
				i++;
			}

		}
		
		//Pointer goes to the next entry
		ptr+= 12;
		//Increment eid counter
		i++;
	}

	//Insert rid and key
	memcpy(ptr, &rid, 8);
	memcpy(ptr+8, &key, sizeof(int));
	
	//update entryCount and store the updated value
	count++;
	memcpy(page+4, &count, sizeof(int));
	
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
	
	int count = getKeyCount();
	
	//Find the correct entry location
	int i = 0;
	int value;

	//Points to the first entry in a node
	char *ptr = page + FIRST_ENTRY_OFFSET;
	
	while( i < count )
	{
		memcpy(&value, ptr+8, sizeof(int));
		
		//If the value is the same as key
		if( value == key) { return RC_INVALID_ATTRIBUTE; }
		//Entry location found
		else if (key < value)
		{
			//set pointer to point to the last entry
			ptr = page + FIRST_ENTRY_OFFSET + ((count-1) * 12);
			
			while( i < count)
			{
				memcpy(ptr+12, ptr, 8);	
				memcpy(ptr+20, ptr+8, sizeof(int));
				
				//set pointer to the earlier entry
				ptr-=12;
				i++;
			}

		}
		
		//Pointer goes to the next entry
		ptr+= 12;
		//Increment eid counter
		i++;
	}
	
	//Insert rid and key
	memcpy(ptr, &rid, 8);
	memcpy(ptr+8, &key, sizeof(int));
	
	//update entryCount and store the updated value
	count++;
	memcpy(page+4, &count, sizeof(int));
		
	/////
	// Split the node
	/////
	
	int splitCount = count / 2;	
	int siblingCount = 0;
	int siblingEntry = splitCount;

	//set ptr to point to the first entry to be copied to sibling
	//first sibling key is located at eid siblingEntry 
	ptr = page + FIRST_ENTRY_OFFSET + ((siblingEntry) * 12);
	//set pointer to set to sibling's buffer
	char *siblingPtr = sibling.page + FIRST_ENTRY_OFFSET;
	
	//set the return value siblingKey 
	memcpy(&siblingKey, ptr+8, sizeof(int));
	
	while(siblingEntry < count)
	{
		//Copy rid and key value to sibling
		memcpy(siblingPtr, ptr, 8);
		memcpy(siblingPtr+8, ptr+8, sizeof(int));
		
		//Pointer goes to the next entry
		ptr+=12;
		siblingPtr+=12;
		siblingEntry++;
		siblingCount++;
	}
	
	//----- update entryCount for both nodes ----	
	memcpy(page+4, &splitCount, sizeof(int));
	memcpy(sibling.page+4, &siblingCount, sizeof(int));

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
	//Get the number of entries in the current node
	int count = getKeyCount();
	
	//If no entry is found in the node
	if( count <= 0 ){return RC_NO_SUCH_RECORD;}
	
	//Points to the first entry in a node
	char *ptr = page + FIRST_ENTRY_OFFSET;
	
	//eid (entry id) counter
	int i 	  = 0;
	//variable to hold key value
	int value;
	
	while( i < count )
	{
		memcpy(&value, ptr+8, sizeof(int));
	
		//Returns the eid
		if( value  >= searchKey )
		{
			eid = i;
			return 0;
		}
		
		//Pointer goes to the next entry
		ptr+= 12;
		//Increment eid counter
		i++;
	}
	
	//Otherwise, return the last pointer
	eid = -1;
	return RC_NO_SUCH_RECORD; 
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

	if( eid >= getKeyCount()){ return RC_NO_SUCH_RECORD; }

	//Add 20 bytes to point to the first entry
	//For every entry number, add 12 bytes
	char *ptr = page + FIRST_ENTRY_OFFSET + (eid*12);

	//Copy the key value
	memcpy(&key, ptr+8, sizeof(int));

	//Copy the rid value
	memcpy(&rid, ptr, 8);
	
	return 0; 
	
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	PageId	pid; 
	memcpy(&pid, page+1021, sizeof(int));
	return pid; 
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 	
	memcpy(page+1021, &pid, sizeof(int));
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
	RC   rc;
	if ((rc = pf.read(pid, page)) < 0) 	{return rc;}	
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
	RC 	rc;
	if ((rc = pf.write(pid, page)) < 0) {return rc;}
	return 0;  
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
	int count;
	memcpy(&count, page+4, sizeof(int));
	return count; 
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 

	int count = getKeyCount();
	//If the node is full
	if(count == MAX_ENTRY) { return RC_NODE_FULL; }
		
	//Find the correct entry location
	int i = 0;
	int value;
	
	//Points key values 
	char *ptr = page + FIRST_ENTRY_OFFSET + 4;

	while( i < count )
	{
		memcpy(&value, ptr, sizeof(int));
		
		if( key == value) { return RC_INVALID_ATTRIBUTE; }
		
		else if( key < value )
		{
			//set pointer to point to the last (key, pointer) pair
			ptr = page + FIRST_ENTRY_OFFSET + 4 + ((count-1) * 8);
			
			//shift(key, pointer) one position to the right
			while( i < count)
			{
				//copy the key
				memcpy(ptr+8, ptr, sizeof(int));	
				//copy the pointer
				memcpy(ptr+12, ptr+4, sizeof(int));
				
				//set pointer to the earlier entry
				ptr-=8;
				i++;
			}
			
		}
	
		//Pointer goes to the next (key, pointer) pair
		ptr+= 8;
		//Increment eid counter
		i++;
	
	}
	
	//Insert key and pid 
	memcpy(ptr, &key, sizeof(int));
	memcpy(ptr+4, &pid, sizeof(int));
	
	//store the updated value
	count++;
	memcpy(page+4, &count, sizeof(int));
		
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

	int count = getKeyCount();
		
	//Find the correct entry location
	int i = 0;
	int value;
	
	//Points key values 
	char *ptr = page + FIRST_ENTRY_OFFSET + 4;

	while( i < count )
	{
		memcpy(&value, ptr, sizeof(int));
		
		if( key == value) { return RC_INVALID_ATTRIBUTE; }
		
		else if( key < value )
		{
			//set pointer to point to the last (key, pointer) pair
			ptr = page + FIRST_ENTRY_OFFSET + 4 + ((count-1) * 8);
			
			//shift(key, pointer) one position to the right
			while( i < count)
			{
				//copy the key
				memcpy(ptr+8, ptr, sizeof(int));	
				//copy the pointer
				memcpy(ptr+12, ptr+4, sizeof(int));
				
				//set pointer to the earlier entry
				ptr-=8;
				i++;
			}
			
		}
	
		//Pointer goes to the next (key, pointer) pair
		ptr+= 8;
		//Increment eid counter
		i++;
	
	}
	
	//Insert key and pid 
	memcpy(ptr, &key, sizeof(int));
	memcpy(ptr+4, &pid, sizeof(int));
	
	//store the updated entryCount value
	count++;
	memcpy(page+4, &count, sizeof(int));

	/////
	// Split the node
	/////
	
	int splitCount 			= count / 2;	
	int siblingCount 		= 0;
	//add 1 to skip the midEntry for return
	int siblingEntry 		= splitCount+1;
	int siblingFirstPid 	= 0;
	
	//set ptr to point to the midKey 
	ptr = page + FIRST_ENTRY_OFFSET + 4 + ((siblingEntry-1) * 8);		
	//set the return value siblingKey 
	memcpy(&midKey, ptr, sizeof(int));
	//copy the pointer for midKey, to be copied as sibling's leftmost pid pointer
	memcpy(&siblingFirstPid, ptr+4, sizeof(pid));
	
	//set ptr to point to the first entry to be copied to sibling
	//first sibling key is located at eid siblingEntry 
	ptr+= 8;
	//set pointer to point to buffer  
	char *siblingPtr = sibling.page + 4 + FIRST_ENTRY_OFFSET;
	//copy pointer for midKey as sibling's leftmost pid pointer
	memcpy(siblingPtr-4, &siblingFirstPid, sizeof(int));
	
	while(siblingEntry < count)
	{
		//Copy (key, entry) pair to sibling
		memcpy(siblingPtr, ptr, sizeof(int));
		memcpy(siblingPtr+4, ptr+4, sizeof(int));
		
		//Pointer goes to the next entry
		ptr+=8;
		siblingPtr+=8;
		siblingEntry++;
		siblingCount++;
	}
	
	//----- update entryCount for both nodes ----	
	memcpy(page+4, &splitCount, sizeof(int));
	memcpy(sibling.page+4, &siblingCount, sizeof(int));
	
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

	//Get the number of entries in the current node
	int count = getKeyCount();
	
	//If no entry is found in the node
	if( count <= 0 ){return RC_NO_SUCH_RECORD;}
	
	//Points to the first entry in a node
	char *ptr = page + FIRST_ENTRY_OFFSET;
	//variable to hold key value
	int value;
	//index for iteration
	int i = 0;
	
	while( i < count )
	{
		memcpy(&value, ptr+4, sizeof(int));
		
		//Returns pointer to child to follow
		if( searchKey < value )
		{
			memcpy(&pid, ptr, sizeof(int));
			return 0;
		}
		
		//Pointer goes to the next entry
		ptr+= 8;
		//Increment eid counter
		i++;
	}
	
	//Otherwise, return the rightmost pointer
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
	//Points to the first entry in a node
	char *ptr = page + FIRST_ENTRY_OFFSET;
	
	//Store the left pointer 
	memcpy(ptr, &pid1, sizeof(int));
	//Store the key value
	memcpy(ptr+4, &key, sizeof(int));
	//Store the right pointer 
	memcpy(ptr+8, &pid2, sizeof(int));
	
	//update entryCount
	int entryCount = 1;
	memcpy(page+4, &entryCount, sizeof(int));
	
	return 0; 
}

#include "BTreeNode.h"

using namespace std;

BTLeafNode::BTLeafNode() //Constructor
{
	//Initialize all pids to -1
	int initial = -1;
	for (int i=0; i<N_LEAF; i++)
		memcpy(leafArray + (i * 12), &initial, sizeof(int));
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
	char * temp = (char*) malloc(pf.PAGE_SIZE);
	if (!pf.read(pid, temp)) {
		memcpy(leafArray, temp, pf.PAGE_SIZE);
		free(temp);
		return 0;
	}
	else {
		free(temp);
		return RC_FILE_READ_FAILED;
	}
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
	char * temp = (char*) malloc(pf.PAGE_SIZE);
	memcpy(temp, leafArray, pf.PAGE_SIZE);
	if (!pf.write(pid, temp)) {
		free(temp);
		return 0;
	}
	else {
		free(temp);
		return RC_FILE_WRITE_FAILED;
	}
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
	int i = 0;
    for (i; i<N_LEAF; i++) {
        if (getRecordPid(i) == -1)
            break;
    }
    return i;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
	//Get the number of keys (entries) in the node:
    int usedEntries;
	usedEntries = getKeyCount();
    
    //Check the node is not full, it it is not, continue.
    if (usedEntries > N_LEAF)
        return RC_NODE_FULL;
    
    //Find the index  (insert position) where the new pair should go
    int insertPos;
    locate(key, insertPos);
   
	// Move (linearly) all nodes one position to the right to make space
	// for new pair to be inserted
    for (int i = usedEntries - 1; i >= insertPos ; i--) 
	{
		setRecordPid(i+1, getRecordPid(i));
		setRecordSid(i+1, getRecordSid(i));
		setKey(i+1, getKey(i));
    }
	// Insert new pair of recordId and key
    setRecordPid(insertPos, rid.pid);
	setRecordSid(insertPos, rid.sid);
	setKey(insertPos, key);
	
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
	// CHECK sibling is empty
	if (sibling.getKeyCount() != 0)
		return RC_INVALID_ATTRIBUTE;

	int insertPos;
	
	// find out where the new key should go, if in the old node or the new (sibling)
	locate(key, insertPos);
	
	if(insertPos <= 42)
	{
		// Copy the last 43 elements into the new array
		memcpy(sibling.leafArray, leafArray + ((N_LEAF/2/*42*/) * 12), (N_LEAF/2 + 1) * 12);
		// Insert the new pair into the old array
		insert(key, rid);
	}
	else
	{
		// Copy only the last 42 elements into the new array
		memcpy(sibling.leafArray, leafArray + ((N_LEAF/2 + 1/*43*/) * 12), (N_LEAF/2) * 12);
		
		// Insert the new pair into the new array
		sibling.insert(key, rid);
	}

	//Reset pid for empty entries in the old node and sibling
	int initial = -1;
	for (int i=N_LEAF/2 + 1/*start at pos 43*/; i<N_LEAF; i++)
		memcpy(leafArray + (i * 12), &initial, sizeof(int));

	for (int i=N_LEAF/2 + 1/*start at pos 43*/; i<N_LEAF; i++)
		memcpy(sibling.leafArray + (i * 12), &initial, sizeof(int));

	siblingKey = sibling.getKey(0);
		
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
	int insertPos = 0;
	while (searchKey > getKey(insertPos) && insertPos != N_LEAF && insertPos < getKeyCount())
		insertPos++;

	//if (insertPos > N_LEAF)
	//	return RC_NODE_FULL;
	eid = insertPos;
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
	if(eid < 0 || eid >=N_LEAF)
		return RC_NO_SUCH_RECORD;

	rid.pid = this->getRecordPid(eid);
	rid.sid = this->getRecordSid(eid);

	key = this->getKey(eid);
	return 0; 
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	PageId temp;

	memcpy(&temp, this->leafArray + (PageFile::PAGE_SIZE - 4), sizeof(PageId));

	return (PageId)temp;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	memcpy(this->leafArray + (PageFile::PAGE_SIZE - 4), &pid, sizeof(PageId));    
	return 0; 
}


/*
 * Constructor for BTNonLeafNode, initialize pids with (-1)
 */
BTNonLeafNode::BTNonLeafNode()
{
	int i;
	int initial = -1;
	for (i=0; i<N_NONLEAF; i++)
		memcpy(nonLeafArray + (i * 8),&initial,sizeof(int));
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
	char * temp = (char*) malloc(pf.PAGE_SIZE);
	if (!pf.read(pid, temp)) {
		memcpy(nonLeafArray, temp, pf.PAGE_SIZE);
		free(temp);
		return 0;
	}
	else {
		free(temp);
		return RC_FILE_READ_FAILED;
	}
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
	char * temp = (char*) malloc(pf.PAGE_SIZE);
	memcpy(temp, nonLeafArray, pf.PAGE_SIZE);
	if (!pf.write(pid, temp)) {
		free(temp);
		return 0;
	}
	else {
		free(temp);
		return RC_FILE_WRITE_FAILED;
	}
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
	int i = 0;
	for (i; i<N_NONLEAF; i++) {
		if (getPid(i) == -1)
			break;
	}
	return i;
}

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 
	//Get the number of keys (entries) in the node:
	int usedEntries;
	usedEntries = getKeyCount();
	
	//Check the node is not full, it it is not, continue.
	if (usedEntries == N_NONLEAF)
		return RC_NODE_FULL;
	
	//Find the index  (insert position) where the new pair should go
	int insertPos;
	insertPos=0;
	while(key > getKey(insertPos) && insertPos != usedEntries-1)
		insertPos++;
	
	int i;
	if (key <= getKey(insertPos)){
		for (i = usedEntries - 1; i >= insertPos; i--) {
			setKey(i+1, getKey(i));
			setPid(i+1, getPid(i));
		}
		setKey(insertPos, key);
		setPid(insertPos, pid);
	}
	else {
		setKey(usedEntries, key);
		setPid(usedEntries, pid);
	}
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
	if (sibling.getKeyCount() != 0)
		return RC_INVALID_ATTRIBUTE;
		
	//Find the index  (insert position) where the new pair should go
	int insertPos;
	insertPos=0;
	while(key > getKey(insertPos) && insertPos != N_NONLEAF/2)
		insertPos++;
	
	//If insertpos == N_NONLEAF/2 (or 63), insert the new key in the sibling, else in the  caller.
	if (insertPos == N_NONLEAF/2) {
		memcpy(sibling.nonLeafArray, nonLeafArray + (((N_NONLEAF+1)/2) * 8),((N_NONLEAF)/2) * 8);
		//Reset pid for empty entries in caller node (from N_NONLEAF + 1 on wards)
		int i;
		int initial = -1;
		for (i=(N_NONLEAF+1)/2; i<N_NONLEAF; i++)
			memcpy(nonLeafArray + (i * 8),&initial,sizeof(int));		
		sibling.insert(key, pid);
	}
	else {
		memcpy(sibling.nonLeafArray, nonLeafArray + ((N_NONLEAF/2) * 8),((N_NONLEAF+1)/2) * 8);
		//Reset pid for empty entries in caller node (from N_NONLEAF onwards)
		int i;
		int initial = -1;
		for (i=N_NONLEAF/2; i<N_NONLEAF; i++)
			memcpy(nonLeafArray + (i * 8),&initial,sizeof(int));		
		insert(key,pid);
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
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ 
	//Find the index  where the searchKey would be located
	
	if (pid < 0)
		return RC_INVALID_ATTRIBUTE;
	
	int location;
	location=0;
	while(searchKey > getKey(location) && location != N_NONLEAF)
		location++;
		
	if (searchKey < getKey(location))
		pid = getPid(location);
	else
		pid = getPid(location+1); 
	
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
	if (pid1<0 || pid2<0)
		return RC_INVALID_ATTRIBUTE;

	setPid(0, pid1);
	setKey(0, key);
	setPid(1, pid2);
	return 0; 
}

/*
 * Return the Pid of the "index" entry in the array.
 * @param index[IN] the index of the entry.
 * @return the PageID of the entry if successful. 
 * Return an error code if there is an error.
 */
PageId BTNonLeafNode::getPid(int index)
{
	// Check index value
	if(index < 0 || index >= N_NONLEAF)
		return RC_INVALID_ATTRIBUTE;

	PageId temp;
	memcpy(&temp, nonLeafArray + (index*8), sizeof(PageId));
	return (PageId)temp;
}


/*
* Set the Pid of the "index" entry in the array.
* @param index[IN] the index of the entry.
* @param pid[IN] the PageId to set.
* Return an error code if there is an error, 0 if succesful.
*/
	RC BTNonLeafNode::setPid(int index, PageId pid)
	{
		// Check index value
		if(index < 0 || index >= N_NONLEAF)
			return RC_INVALID_ATTRIBUTE;
			
		memcpy(nonLeafArray + (index*8), &pid, sizeof(PageId));	
		return 0;
	}


/*
 * Return the Key of the "index" entry in the array.
 * @param index[IN] the index of the entry.
 * @return the Key of the entry if successful. 
 * Return an error code if there is an error.
 */
int BTNonLeafNode::getKey(int index)
{
	// Check index value
	if(index < 0 || index >= N_NONLEAF)
		return RC_INVALID_ATTRIBUTE;

	int temp;
	memcpy(&temp, nonLeafArray + (index*8 + 4), sizeof(int));
	return (int)temp;
}


/*
* Set the Key of the "index" entry in the array.
* @param index[IN] the index of the entry.
* @param key[IN] the Key to set.
* Return an error code if there is an error, 0 if succesful.
*/
int BTNonLeafNode::setKey(int index, int key)
{
	// Check index value
	if(index < 0 || index >= N_NONLEAF)
		return RC_INVALID_ATTRIBUTE;
		
	memcpy(nonLeafArray + (index*8 + 4), &key, sizeof(int));
	return 0;
}

PageId BTLeafNode::getRecordPid(int index)
{
	// Check index value
	if(index < 0 || index >= N_LEAF)
		return RC_INVALID_ATTRIBUTE;

	PageId temp;

	memcpy(&temp, this->leafArray + (index*12), sizeof(PageId));

	return (PageId)temp;
}

int BTLeafNode::getRecordSid(int index)
{
	// Check index value
	if(index < 0 || index >= N_LEAF)
		return RC_INVALID_ATTRIBUTE;

	int temp;

	memcpy(&temp, this->leafArray + (index*12 + 4), sizeof(int));

	return (int)temp;
}

int BTLeafNode::getKey(int index)
{
	// Check index value
	if(index < 0 || index >= N_LEAF)
		return RC_INVALID_ATTRIBUTE;

	int temp;

	memcpy(&temp, this->leafArray + (index*12 + 8), sizeof(int));

	return (int)temp;
}

RC BTLeafNode::setRecordPid(int index, PageId pid)
{
	// Check index value
    if(index < 0 || index >= N_LEAF)
		return RC_INVALID_ATTRIBUTE;
            
    memcpy(leafArray + (index*12), &pid, sizeof(PageId));    
    return 0;
}

RC BTLeafNode::setRecordSid(int index, int sid)
{
	// Check index value
    if(index < 0 || index >= N_LEAF)
		return RC_INVALID_ATTRIBUTE;
            
    memcpy(leafArray + (index*12 + 4), &sid, sizeof(int));    
    return 0;
}

int BTLeafNode::setKey(int index, int key)
{
	// Check index value
    if(index < 0 || index >= N_LEAF)
        return RC_INVALID_ATTRIBUTE;
        
    memcpy(leafArray + (index*12 + 8), &key, sizeof(int));
    return 0;
}
#include "BTreeNode.h"

#include <iostream>

using namespace std;


BTLeafNode::BTLeafNode(PageFile& pf)
{
	/*if (pf.open("Random.pf", 'w') != 0)
		int i = RC_FILE_OPEN_FAILED;*/
	RecordId r;
	r.pid = -1;
	r.sid = -1;
	for (int i = 0; i < BTLeafNode_N; i++)
	{
		this->nodeKey[i] = -1;
		this->nodeRid[i] = r;
	}

	this->self = pf.endPid();
	this->parent = -1;
	this->nodeLast = -1;
	keyCount = 0;

	write(pf.endPid(),pf);

	//if(pf.close() != 0)
	//	int i = RC_FILE_CLOSE_FAILED;
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
char	buffer[1024];
	int		offset=0;

	// Read into buffer
	if (pf.read(pid,buffer) != 0)
		return RC_FILE_READ_FAILED;

		//Keys
	memcpy(this->nodeKey, buffer + offset, BTLeafNode_N*4);
	offset += BTLeafNode_N*4;

		//Rid
	memcpy(this->nodeRid, buffer + offset, BTLeafNode_N*8);
	offset += BTLeafNode_N*8;

		//Last
	memcpy(&(this->nodeLast), buffer + offset, 4);
	offset += 4;

		//Keycount
	memcpy(&(this->keyCount), buffer + offset, 4);
	offset += 4;

		//Parent
	memcpy(&(this->parent), buffer + offset, 4);
	offset += 4;

		//Self
	memcpy(&(this->self), buffer + offset, 4);
	offset += 4; 



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
	char	buffer[1024];
	int		offset=0;

	// Read into buffer

		//Keys
	memcpy(buffer + offset, this->nodeKey, BTLeafNode_N*4);
	offset += BTLeafNode_N*4;

		//Rids
	memcpy(buffer + offset, this->nodeRid, BTLeafNode_N*8);
	offset += BTLeafNode_N*8;

		//Last
	memcpy(buffer + offset, &(this->nodeLast), 4);
	offset += 4;

		//Keycount
	memcpy(buffer + offset, &(this->keyCount), 4);
	offset += 4;
	
		//Parent
	memcpy(buffer + offset, &(this->parent), 4);
	offset += 4;

		//Self
	memcpy(buffer + offset, &(this->self), 4);
	offset += 4; 

	if (pf.write(pid,buffer) != 0)
		return RC_FILE_WRITE_FAILED;

	return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
	return keyCount;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
	//////////// Normal insert
	//////////if(keyCount < BTLeafNode_N)
	//////////{
	//////////	int eid;
	//////////	int callLocate = locate(key, eid);

	//////////	for(int i = getKeyCount(); i > eid; i--)
	//////////	{
	//////////		nodeKey[i] = nodeKey[i-1];
	//////////		nodeRid[i] = nodeRid[i-1];
	//////////	}

	//////////	nodeKey[eid] = key;
	//////////	nodeRid[eid] = rid;
	//////////	
	//////////	keyCount++;
	//////////}

	//////////// Overflow at leaf node
	//////////else
	//////////{
	//////////	BTLeafNode newNode(p);

	//////////	newNode.parent = this->parent;

	//////////	int	siblingKey;

	//////////	this->insertAndSplit(key, rid, newNode, siblingKey);
	//////////}

	// If full
	
	if (keyCount >= BTLeafNode_N)
		return RC_NODE_FULL;

	int eid;
	int callLocate = locate(key, eid);

	for(int i = getKeyCount(); i > eid; i--)
	{
		nodeKey[i] = nodeKey[i-1];
		nodeRid[i] = nodeRid[i-1];
	}

	nodeKey[eid] = key;
	nodeRid[eid] = rid;
	
	keyCount++;

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
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, BTLeafNode& sibling, int& siblingKey)
{ 
	//copy array contents to sibling node
	for (int i = getKeyCount()/2; i < getKeyCount(); i++)
	{
		sibling.insert(nodeKey[i],nodeRid[i]);
	
	}
	sibling.keyCount = getKeyCount()-getKeyCount()/2;
	keyCount = getKeyCount()/2;
	
	if (key < sibling.nodeKey[0])
		this->insert(key, rid);
	else
		sibling.insert(key,rid);

	// Set the sibling's parent
	sibling.setParent(this->parent);

	// Set next pointers
	sibling.setNextNodePtr(nodeLast);
	nodeLast = sibling.self;


	// [OUT] key
	siblingKey = sibling.nodeKey[0];
	

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
	int i = 0;
	
	while(i < getKeyCount() &&  searchKey > nodeKey[i])
	{
		i++;
	}

	eid = i;
	//cout << "LOCATED! EID: " << eid << endl;
	//if (searchKey > nodeKey[keyCount-1])
	//	return RC_END_OF_TREE;
	

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
	if (nodeRid[eid].pid == -1 || nodeRid[eid].sid == -1)
		return -1;


	rid.pid = nodeRid[eid].pid;
	rid.sid = nodeRid[eid].sid;
	key = nodeKey[eid];

	return 0;
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	return nodeLast;
}

/*
 * Set the pid of the next sibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	nodeLast = pid;
	
	return 0; 
}


void BTLeafNode::setParent(PageId pid)
{
	parent = pid;
}


PageId BTLeafNode::getParentPid()
{
	return this->parent;
}

PageId BTLeafNode::getPid()
{
	return this->self;
}


BTLeafNode* BTLeafNode::BTLeafNodeLocator(PageId pid)
{
	
	
	return NULL;
}

RecordId	BTLeafNode::ridFinder(int index)
{
	return this->nodeRid[index];
}










































BTNonLeafNode::BTNonLeafNode(PageFile& pf)
{
	/*if (pf.open("Random.pf", 'w') != 0)
		int i = RC_FILE_OPEN_FAILED;*/

	for (int i = 0; i < BTNonLeafNode_N; i++)
	{
		this->nodeKey[i] = -1;
		this->nodePid[i] = -1;
	}
	this->nodePid[BTNonLeafNode_N] = -1;

	this->self = pf.endPid();
	this->parent = -1;
	write(pf.endPid(),pf);
	keyCount = 0;

	//if(pf.close() != 0)
	//	int i = RC_FILE_CLOSE_FAILED;
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
	char	buffer[1024];
	int		offset=0;

	// Read into buffer
	if (pf.read(pid,buffer) != 0)
		return RC_FILE_READ_FAILED;

		//Pids
	memcpy(this->nodeKey, buffer + offset, BTNonLeafNode_N*4);
	offset += BTNonLeafNode_N*4;

		//Keys
	memcpy(this->nodePid, buffer + offset, (BTNonLeafNode_N+1)*4);
	offset += (BTNonLeafNode_N+1)*4;

		//Keycount, Parent, Self
	memcpy(&(this->keyCount), buffer + offset, 4);
	offset += 4;

		//Parent
	memcpy(&(this->parent), buffer + offset, 4);
	offset += 4;

		//Self
	memcpy(&(this->self), buffer + offset, 4);
	offset += 4; 


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
	char	buffer[1024];
	int		offset=0;

	// Read into buffer

		//Pids
	memcpy(buffer + offset, this->nodeKey, BTNonLeafNode_N*4);
	offset += BTNonLeafNode_N*4;

		//Keys
	memcpy(buffer + offset, this->nodePid, (BTNonLeafNode_N+1)*4);
	offset += (BTNonLeafNode_N+1)*4;

		//Keycount, Parent, Self
	memcpy(buffer + offset, &(this->keyCount), 4);
	offset += 4;

		//Parent
	memcpy(buffer + offset, &(this->parent), 4);
	offset += 4;

		//Self
	memcpy(buffer + offset, &(this->self), 4);
	offset += 4; 

	if (pf.write(pid,buffer) != 0)
		return RC_FILE_WRITE_FAILED;

	return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
	return keyCount;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 
	// If full
	if (keyCount >= BTLeafNode_N)
		return RC_NODE_FULL;

	int eid;

	int callLocate = locate(key, eid);

	for(int i = getKeyCount(); i > eid; i--)
	{
		nodeKey[i] = nodeKey[i-1];
		nodePid[i+1] = nodePid[i];
	}

	nodeKey[eid] = key;
	nodePid[eid+1] = pid;

	keyCount++;

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
	// NOTE THAT PID AND KEY in ARRAY ELEMENT ARE DIRECTLY ASSOCIATED WITH EACHOTHER!!!
	// Create a temp key array that will calculate the midkey
	int		tempKeyArray[BTNonLeafNode_N+1];
	PageId	tempPidArray[BTNonLeafNode_N+2];

	for(int i=0; i < BTNonLeafNode_N; i++)
	{
		tempKeyArray[i] = nodeKey[i];
		tempPidArray[i] = nodePid[i+1];
	}

	// Insert the midkey and Pid and sort it
	int i=0;

	while (key >= tempKeyArray[i] && i < BTNonLeafNode_N)
	{
		i++;
	}

	int k = BTNonLeafNode_N;
	while (k > i)
	{
		tempKeyArray[k] = tempKeyArray[k-1];
		tempPidArray[k] = tempPidArray[k-1];
		k--;
	}

	tempKeyArray[i] = key;
	tempPidArray[i] = pid;

	//[OUT] midkey value
	midKey = tempKeyArray[(BTNonLeafNode_N+1)/2];

	// Set first element of sibling to point to midkey's pid
	sibling.nodePid[0] = tempPidArray[(BTNonLeafNode_N+1)/2];

	// copy array contents to sibling node
	for (int i = (BTNonLeafNode_N+1)/2 + 1; i < getKeyCount()+1; i++)
	{
		sibling.insert(tempKeyArray[i],tempPidArray[i]);
	}

	// Set new keycount size
	keyCount = (BTNonLeafNode_N+1)/2;
	
	// copy new array into member variables
	for (int k = 0; k < (BTNonLeafNode_N+1)/2; k++)
	{
		nodePid[k+1] = tempPidArray[k];
		nodeKey[k] = tempKeyArray[k];
	}

	// Set the sibling's parent
	sibling.setParent(this->parent);
	
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
RC BTNonLeafNode::locate(int searchKey, int& eid)
{ 
	int i = 0;
	
	while(i < getKeyCount() && searchKey > nodeKey[i])
	{
		i++;
	}

	eid = i;
	

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
	int i=0;

	while (i < keyCount && searchKey >= nodeKey[i])
	{
		i++;
	}

	pid = nodePid[i];

	return 0;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)  ////MAYBE WE HAVE TO INSERT THE VALUE INTO LEAF??? NO RID!!!
{ 
	keyCount = 1;

	// Set up the B+ Tree
	nodePid[0] = pid1;
	nodePid[1] = pid2;
	nodeKey[0] = key;

	for (int i=2; i < BTNonLeafNode_N; i++)
		nodePid[i] = -1;

	//BTLeafNode leaf1(pid1);
	//BTLeafNode leaf2(pid2);

	///*leaf1.setParent(this->self);
	//leaf2.setParent(this->self);

	//leaf1.setNextNodePtr(pid2);*/



	// CALL NODELOCATOR FUCNTION


	return 0;
}

void BTNonLeafNode::setParent(PageId pid)
{
	parent = pid;
}

PageId BTNonLeafNode::getPid()
{
	return this->self;
}

PageId BTNonLeafNode::getParentPid()
{
	return this->parent;
}



PageId BTNonLeafNode::pidFinder(int index)
{
	return this->nodePid[index];
}

void BTNonLeafNode::printNode()
{
	cout << "   ~~~ BTNonLeafNode ~~~ keycount:" << keyCount << "     selfpid: " << self << endl;

	int i=0;
	while (i <= getKeyCount())
	{
		cout << "   Pid:" << nodePid[i] << "       Key:" << nodeKey[i] << endl;
		i++;
	}

	cout << endl;
	return;
}


void BTLeafNode::printNode()
{
	cout << "   ~~~ BTLeafNode ~~~ keycount:" << keyCount << "     selfpid: " << self << endl;

	int i=0;
	while (i <= getKeyCount())
	{
		cout << "    Rid:" << nodeRid[i].pid << "," << nodeRid[i].sid << "     Key:" << nodeKey[i] << endl;
		i++;
	}

	cout << "     -   NODELAST:" << this->nodeLast << endl;

	cout << endl;
	return;
}

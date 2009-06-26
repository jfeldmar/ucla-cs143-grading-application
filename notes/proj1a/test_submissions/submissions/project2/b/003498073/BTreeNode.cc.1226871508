#include "BTreeNode.h"

#include <iostream>

using namespace std;


BTLeafNode::BTLeafNode(PageFile pf)
{
	if (pf.open("Random.pf", 'w') != 0)
		int i = RC_FILE_OPEN_FAILED;

	this->self = pf.endPid();
	write(pf.endPid(),pf);
	keyCount = 0;
	cout << "~~~ BTLeafNode Constructor called!! :: pid:" << this->self << endl;

	if(pf.close() != 0)
		int i = RC_FILE_CLOSE_FAILED;
		
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
	if (pf.read(pid,buffer) != 0)
		return RC_FILE_READ_FAILED;
	
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
		cout << "INSIDE insertNSplit: nodekey[i], nodeRid[i] = " << nodeKey[i] << "," << nodeRid[i].pid << " " << nodeRid[i].sid << endl;
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
	
	while(i < getKeyCount() && searchKey > nodeKey[i])
	{
		i++;
	}

	eid = i;
	

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


PageId BTLeafNode::getPid()
{
	return this->self;
}

BTLeafNode* BTLeafNode::BTLeafNodeLocator(PageId pid)
{
	
	
	return NULL;
}










































BTNonLeafNode::BTNonLeafNode(PageFile pf)
{
	if (pf.open("Random.pf", 'w') != 0)
		int i = RC_FILE_OPEN_FAILED;

	this->self = pf.endPid();
	write(pf.endPid(),pf);
	keyCount = 0;
	cout << "~~~ BTLeafNode Constructor called!! :: pid:" << this->self << endl;

	if(pf.close() != 0)
		int i = RC_FILE_CLOSE_FAILED;
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
	if (pf.read(pid,buffer) != 0)
		return RC_FILE_READ_FAILED;
	
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
	//[OUT] midkey value
	midKey = nodeKey[keyCount/2];

	// Find index of midKey
	int eid;
	locate(key, eid);

	// CONSIDER THAT THE ROOT IS FULL OR SOMETHING


	//copy array contents to sibling node
	for (int i = getKeyCount()/2 + 1; i < getKeyCount(); i++)
	{
		cout << "INSIDE insertNSplit: nodekey[i], nodeRid[i] = " << nodeKey[i] << "," << nodePid[i+1] << endl;
		sibling.insert(nodeKey[i],nodePid[i+1]);
	
	}
	sibling.keyCount = getKeyCount()-getKeyCount()/2-1;
	keyCount = getKeyCount()/2;

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


void BTNonLeafNode::printNode()
{
	cout << endl << "BTNonLeafNode keycount: " << keyCount << endl;

	int i=0;
	while (i <= getKeyCount())
	{
		cout << "/t Pid:" << nodePid[i] << "  Key:" << nodeKey[i] << endl;
		i++;
	}

	cout << endl << endl;
	return;
}


void BTLeafNode::printNode()
{
	cout << endl << "BTLeafNode keycount: " << keyCount << endl;

	int i=0;
	while (i <= getKeyCount())
	{
		cout << "/t Rid:" << nodeRid[i].pid << "," << nodeRid[i].sid << "  Key:" << nodeKey[i] << endl;
		i++;
	}

	cout << endl << endl;
	return;
}

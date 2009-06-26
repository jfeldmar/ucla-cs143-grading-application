#include "BTreeNode.h"

#include <iostream>

using namespace std;

/************************************************
 ****************B+Tree Leaf Node****************
 ************************************************/

Leaf * BTLeafNode::getLeafPtr()
{
	return pLeaf;
}

BTLeafNode::BTLeafNode()
{
	pLeaf = (Leaf *) buffer;

	pLeaf->keyCount = 0;
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
	return pf.read(pid, buffer);
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
	return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
	return pLeaf->keyCount; 
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
	if(getKeyCount() >= (MAX_NUM_LEAF_PTRS)-1)
		return RC_NODE_FULL;
	if(rid.pid < 0 || rid.sid < 0)
		return RC_INVALID_RID;
	
	int eid;
	locate(key, eid);

	for(int i = getKeyCount(); i > eid; i--)
	{
		pLeaf->ridArr[i] = pLeaf->ridArr[i-1];
		pLeaf->keyArr[i] = pLeaf->keyArr[i-1];
	}

	pLeaf->ridArr[eid] = rid;
	pLeaf->keyArr[eid] = key;

	pLeaf->keyCount++;

	return 0; 
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the KEY TO INSERT.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{
	if(rid.pid < 0 || rid.sid < 0)
		return RC_INVALID_RID;

	int tempKeyArr[MAX_NUM_LEAF_PTRS];

	bool located = false;
	int kIndex = 0;
	for(int tIndex = 0; tIndex <= MAX_NUM_LEAF_PTRS; tIndex++)
	{
		if(pLeaf->keyArr[kIndex] >= key && !located)
		{
			tempKeyArr[tIndex] = key;
			located = true;
		}
		else
		{
			tempKeyArr[tIndex] = pLeaf->keyArr[kIndex];
			kIndex++;
		}
	}

	int medianValue;

	if(MAX_NUM_LEAF_PTRS % 2 == 0)
		medianValue = tempKeyArr[MAX_NUM_LEAF_PTRS / 2];
	else
		medianValue = tempKeyArr[(MAX_NUM_LEAF_PTRS / 2) + 1];

	int eid;
	locate(medianValue, eid);

	Leaf * pSibLeaf = sibling.getLeafPtr();

	pSibLeaf->keyCount = 0;

	for(int i = eid; i < (MAX_NUM_LEAF_PTRS - 1); i++)
	{
		sibling.insert(pLeaf->keyArr[i], pLeaf->ridArr[i]);
		pLeaf->keyCount--;
	}

	if(key < medianValue)
		insert(key, rid);
	else
		sibling.insert(key, rid);

	sibling.setNextNodePtr(getNextNodePtr());

	siblingKey = medianValue;

	return 0; 
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remember that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
	if(getKeyCount() == 0)
		eid = 0;
	else
	{
		int i;
		for(i = 0; i < getKeyCount(); i++)
		{
			if(pLeaf->keyArr[i] >= searchKey)
				break;
		}
		eid = i;
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
	rid = pLeaf->ridArr[eid];
	key = pLeaf->keyArr[eid];

	return 0; 
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{  
	return pLeaf->nextLeafNode; 
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	pLeaf->nextLeafNode = pid;

	return 0; 
}

RC BTLeafNode::printNode()
{
	int i;
	for(i = 0; i < getKeyCount(); i++)
	{
		RecordId rid = pLeaf->ridArr[i];
		cout << "|" << rid.pid << "," << rid.sid <<
			"| " << pLeaf->keyArr[i] << " ";
	}
	PageId pid = getNextNodePtr();
	cout << "|" << pid << "|" << endl;
}


/************************************************
 **************B+Tree Non-Leaf Node**************
 ************************************************/

NonLeaf * BTNonLeafNode::getNonLeafPtr()
{
	return pNonLeaf;
}

BTNonLeafNode::BTNonLeafNode()
{
	pNonLeaf = (NonLeaf *) buffer;

	pNonLeaf->keyCount = 0;
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
	return pf.read(pid, buffer);
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
	return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
	return pNonLeaf->keyCount;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{  
	if(getKeyCount() >= (MAX_NUM_NONLEAF_PTRS - 1))
		return RC_NODE_FULL;
	if(pid < 0)
		return RC_INVALID_PID;

	int eid;
	locate(key, eid);

	for(int i = getKeyCount(); i > eid; i--)
	{
		pNonLeaf->pidArr[i+1] = pNonLeaf->pidArr[i];
		pNonLeaf->keyArr[i] = pNonLeaf->keyArr[i-1];
	}

	pNonLeaf->pidArr[eid+1] = pid;
	pNonLeaf->keyArr[eid] = key;

	pNonLeaf->keyCount++;

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
	if(pid < 0)
		return RC_INVALID_PID;

	int tempKeyArr[MAX_NUM_NONLEAF_PTRS];

	bool located = false;
	int kIndex = 0;
	for(int tIndex = 0; tIndex <= MAX_NUM_NONLEAF_PTRS; tIndex++)
	{
		if(pNonLeaf->keyArr[kIndex] >= key && !located)
		{
			tempKeyArr[tIndex] = key;
			located = true;
		}
		else
		{
			tempKeyArr[tIndex] = pNonLeaf->keyArr[kIndex];
			kIndex++;
		}
	}

	int medianValue;

	if(MAX_NUM_NONLEAF_PTRS % 2 == 0)
		medianValue = tempKeyArr[MAX_NUM_NONLEAF_PTRS / 2];
	else
		medianValue = tempKeyArr[(MAX_NUM_NONLEAF_PTRS / 2) + 1];

	midKey = medianValue;

	int eid;
	locate(medianValue, eid);

	if(key != medianValue)
	{
		for(int i = eid; i < (MAX_NUM_NONLEAF_PTRS - 1); i++)
		{
			pNonLeaf->keyArr[i] = pNonLeaf->keyArr[i+1];
			pNonLeaf->pidArr[i+1] = pNonLeaf->pidArr[i+2];
		}
		pNonLeaf->keyCount--;
		insert(key, pid);
	}

	NonLeaf * pSibNonLeaf = sibling.getNonLeafPtr();

	pSibNonLeaf->keyCount = 0;
	pSibNonLeaf->pidArr[0] = pid;
	for(int i = eid; i < (MAX_NUM_NONLEAF_PTRS - 1); i++)
	{
		sibling.insert(pNonLeaf->keyArr[i], pNonLeaf->pidArr[i+1]);
		pNonLeaf->keyCount--;
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
	if(searchKey < pNonLeaf->keyArr[0])
		pid = pNonLeaf->pidArr[0];
	else
	{
		int i;
		for(i = getKeyCount(); i > 0; i--)
		{
			if(searchKey >= pNonLeaf->keyArr[i-1])
				break;
		}
		pid = pNonLeaf->pidArr[i];
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
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{
	if(pid1 < 0 || pid2 < 0)
		return RC_INVALID_PID;

	pNonLeaf->pidArr[0] = pid1;
	pNonLeaf->keyArr[0] = key;
	pNonLeaf->pidArr[1] = pid2;
	
	pNonLeaf->keyCount = 1;
 
	return 0; 
}

RC BTNonLeafNode::locate(int searchKey, int& eid)
{
	int i;
	for(i = 0; i < getKeyCount(); i++)
	{
		if(pNonLeaf->keyArr[i] >= searchKey)
			break;
	}
	eid = i;

	return 0;
}

RC BTNonLeafNode::printNode()
{
	cout << "|" << pNonLeaf->pidArr[0] << "| ";
	for(int i = 0; i < getKeyCount(); i++)
	{
		cout << pNonLeaf->keyArr[i] << " ";
		cout << "|" << pNonLeaf->pidArr[i+1] << "| ";
	}
	cout << endl;	

	return 0;
}

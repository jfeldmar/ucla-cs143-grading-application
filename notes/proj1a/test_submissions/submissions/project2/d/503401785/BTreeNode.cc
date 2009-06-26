#include "BTreeNode.h"

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
	return pf.read(pid,data);
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
	return pf.write(pid,(const void*)data);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	return (int)data[1022];
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
	if (getKeyCount() >= 50)
		return RC_NODE_FULL;

	LeafEntry *ent = (LeafEntry*)data;

	if (getKeyCount() > 0)
		ent += getKeyCount() - 1;

	for (int i = getKeyCount(); i > 0; i--)
	{
		if (key > ent->key)
		{
			(ent + 1)->key = key;
			(ent + 1)->rid = rid;
			data[1022]++;
			return 0;
		}

		else
		{
			(ent + 1)->key = ent->key;
			(ent + 1)->rid = ent->rid;
			if (i > 1)
				ent--;
		}
	}
	
	ent->rid = rid;
	ent->key = key;
	data[1022]++;
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
	if (getNextNodePtr() != -1)
		sibling.setNextNodePtr(getNextNodePtr());	

	setNextNodePtr((PageId)sibling.data[1023]);

	LeafEntry *ent = (LeafEntry*)data;
	ent += 25;
	
	siblingKey = ent->key;
	int thisLeaf = 26;

	if (key < siblingKey)
	{
		siblingKey = (ent - 1)->key;
		thisLeaf = 25;
	}

	while(thisLeaf <= 50)
	{
		sibling.insert(ent->key,ent->rid);
		thisLeaf++;
		data[1022]--;
		ent++;
	}

	if (key < siblingKey)
		insert(key,rid);
	else
		sibling.insert(key,rid);

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
	LeafEntry *ent = (LeafEntry*)data;
	
	for(int i = 0; i < getKeyCount(); i++)
	{
		if (ent->key >= searchKey)
		{
			eid = i + 1;
			return 0;
		}
		ent++;
	}

	return RC_END_OF_TREE;
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
	LeafEntry *ent = (LeafEntry*)data;

	if (eid > getKeyCount())
		return RC_NO_SUCH_RECORD;
	
	for(int i = 0; i < eid - 1; i++)
		ent++;

	key = (int)ent->key;
	rid = (RecordId)ent->rid;

	return 0;
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
	if ((PageId)data[1021] != (PageId)data[1023])
		return (PageId)data[1021];

	else
		return -1;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	data[1021] = pid;
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
	return pf.read(pid,data);
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
	return pf.write(pid,(const void*)data);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	return (int)data[1022];
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
	if (getKeyCount() >= 50)
		return RC_NODE_FULL;

	NonLeafEntry *ent = (NonLeafEntry*)data;

	if (getKeyCount() > 0)
		ent += getKeyCount() - 1;

	for (int i = getKeyCount(); i > 0; i--)
	{
		if (key > ent->key)
		{
			(ent + 1)->key = key;
			(ent + 1)->pid = pid;
			data[1022]++;
			return 0;
		}

		else
		{
			(ent + 1)->key = ent->key;
			(ent + 1)->pid = ent->pid;
			if (i > 1)
				ent--;
		}
	}
	
	ent->pid = pid;
	ent->key = key;
	data[1022]++;
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
	NonLeafEntry *ent = (NonLeafEntry*)data;
	ent += 25;
	
	midKey = ent->key;
	int thisNonLeaf = 26;

	if (key < midKey)
	{
		midKey = (ent - 1)->key;
		thisNonLeaf = 25;
	}

	while(thisNonLeaf < 50)
	{
		if (ent->key != midKey)
		{
			sibling.insert(ent->key,ent->pid);
			thisNonLeaf++;
			data[1022]--;
			ent++;
		}
	}

	if (key < midKey)
		insert(key,pid);
	else if (key != midKey)
		sibling.insert(key,pid);

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
	NonLeafEntry *ent = (NonLeafEntry*)data;
	
	for(int i = 0; i < getKeyCount(); i++)
	{
		if (ent->key > searchKey)
		{
			if (i == 0)
				pid = (PageId)data[1021];
			else
				pid = (ent - 1)->pid;
			return 0;
		}

		if (i < getKeyCount() - 1)
			ent++;
	}

	pid = (PageId)ent->pid;
	return 0;
	//return RC_NO_SUCH_RECORD;
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
	NonLeafEntry *ent = (NonLeafEntry*)data;

	data[1021] = pid1;
	data[1022] = 1;
	
	ent->key = key;
	ent->pid = pid2;

	return 0;
}

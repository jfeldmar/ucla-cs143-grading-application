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
	char *buffer = new char[PageFile::PAGE_SIZE];

	RC err;
	if(err = pf.read(pid, buffer))
	{
		delete [] buffer;
		return err;
	}

	memcpy(&header, buffer, sizeof(BTLeafNodeHeader));

	// read in actual nodes
	memcpy(data, buffer + sizeof(BTLeafNodeHeader), header.nKeys * sizeof(BTLeafNodeData));

	delete [] buffer;
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
	char *buffer = new char[PageFile::PAGE_SIZE];

	memcpy(buffer, &header, sizeof(BTLeafNodeHeader));
	memcpy(buffer + sizeof(BTLeafNodeHeader), data, header.nKeys * sizeof(BTLeafNodeData));

	RC err = pf.write(pid, buffer);

	delete [] buffer;
	return err;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	return header.nKeys;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
	if(header.nKeys >= MAX_KEYS)
		return RC_NODE_FULL;

	int i;
	for(i = 0; i < header.nKeys; i++)
		if(data[i].key > key)
			break;

	for(int j = header.nKeys; j > i; j--)
	{
		data[j].key = data[j-1].key;
		data[j].recordID = data[j-1].recordID;
	}

	data[i].key = key;
	data[i].recordID = rid;

	header.nKeys++;

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
	for(int i = 0, j = MAX_KEYS / 2; j < MAX_KEYS; i++, j++)
	{
		sibling.data[i].key      = data[j].key;
		sibling.data[i].recordID = data[j].recordID;
	}

	sibling.header.nKeys = MAX_KEYS - MAX_KEYS / 2; // this isn't quite stupid, since MAX_KEYS may be odd
	header.nKeys         = MAX_KEYS / 2;

	if(key > data[header.nKeys - 1].key)
		sibling.insert(key, rid);
	else
		insert(key, rid);

	siblingKey = sibling.data[0].key;
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
	for(int i = 0; i < header.nKeys; i++)
	{
		if(data[i].key >= searchKey)
		{
			eid = i;
			return 0;
		}
	}

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
	if(eid >= header.nKeys)
		return RC_NO_SUCH_RECORD;

	key = data[eid].key;
	rid = data[eid].recordID;

	return 0;
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
	return header.nextSibling;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	header.nextSibling = pid;
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
	char *buffer = new char[PageFile::PAGE_SIZE];

	RC err;
	if(err = pf.read(pid, buffer))
	{
		delete [] buffer;
		return err;
	}

	memcpy(&header, buffer, sizeof(BTNonLeafNodeHeader));

	// read in actual nodes
	memcpy(data, buffer + sizeof(BTNonLeafNodeHeader), header.nKeys * sizeof(BTNonLeafNodeData));

	delete [] buffer;
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
	char *buffer = new char[PageFile::PAGE_SIZE];

	memcpy(buffer, &header, sizeof(BTNonLeafNodeHeader));
	memcpy(buffer + sizeof(BTNonLeafNodeHeader), data, header.nKeys * sizeof(BTNonLeafNodeData));

	RC err = pf.write(pid, buffer);

	delete [] buffer;
	return err;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	return header.nKeys;
}

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
	if(header.nKeys >= MAX_KEYS)
		return RC_NODE_FULL;

	/*if(key < data[0].key)
	{
		insert(data[0].key, header.firstChild); // bump it up
		data[0].key = key;
		header.firstChild = pid;
		return 0;
	}*/

	int i;
	for(i = 0; i < header.nKeys; i++)
		if(data[i].key > key)
			break;

	for(int j = header.nKeys; j > i; j--)
	{
		data[j].key = data[j-1].key;
		data[j].pageID = data[j-1].pageID;
	}

	data[i].key = key;
	data[i].pageID = pid;

	header.nKeys++;

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
	// insert as usual, but we know there'll be overflow, so save it
	int    overflowKey;
	PageId overflowPID;

	// the overflow is either going to be the new data, or whatever USED to be last
	if(key < data[header.nKeys-1].key)
	{
		overflowKey = data[header.nKeys-1].key;
		overflowPID = data[header.nKeys-1].key;

		header.nKeys--;
		insert(key, pid);
	}
	else
	{
		overflowKey = key;
		overflowPID = pid;
	}

	midKey = data[(MAX_KEYS+1) / 2].key; // only need key value because BTree will handle the pointers

	for(int i = 0, j = MAX_KEYS / 2 + 1; j < MAX_KEYS; i++, j++)
	{
		sibling.data[i].key      = data[j].key;
		sibling.data[i].pageID   = data[j].pageID;
	}

	sibling.header.nKeys      = MAX_KEYS - (MAX_KEYS / 2 + 1); // this isn't quite stupid, since MAX_KEYS may be odd
	sibling.header.firstChild = data[MAX_KEYS / 2].pageID;
	header.nKeys              = MAX_KEYS / 2;

	sibling.insert(overflowKey, overflowPID);

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
	if(data[0].key > searchKey)
	{
		pid = header.firstChild;
		return 0;
	}

	for(int i = 1; i < header.nKeys; i++)
	{
		if(data[i].key > searchKey)
		{
			pid = data[i-1].pageID;
			return 0;
		}
	}

	pid = data[header.nKeys-1].pageID;
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
	header.firstChild = pid1;
	header.nKeys = 1;
	data[0].key = key;
	data[0].pageID = pid2;
	return 0;
}

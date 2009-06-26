#include "BTreeNode.h"

using namespace std;
	BTLeafNode::BTLeafNode()
	{
		node.nextPid = -1;
		node.numOfData = 0;
		node.size = (int) ((PageFile::PAGE_SIZE - sizeof(PageId) - sizeof(int) - sizeof(int)) / sizeof(Data));

		for (int i = 0; i < node.size; i++)
		{
			node.data[i].key = -1;
		}
	}

	BTNonLeafNode::BTNonLeafNode()
	{
		node.firstPid = -1;
		node.numOfData = 0;
		node.size = (int) ((PageFile::PAGE_SIZE - sizeof(PageId) - sizeof(int) - sizeof(int)) / sizeof(Data));

		for (int i = 0; i < node.size; i++)
		{
			node.data[i].key = -1;
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
	char buf[PageFile::PAGE_SIZE];

	if (pf.read(pid, buf) < 0) return RC_FILE_READ_FAILED;

	memcpy(&node, buf, 1024);

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
	return pf.write(pid, &node);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	return node.numOfData;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
	  int i = 0;

	  if (node.numOfData == node.size) return RC_NODE_FULL;

	  for (; i < node.size; i++)
	  {
		  if (key < node.data[i].key || node.data[i].key == -1)
		  {
			  break;
		  }
	  }

	  if (node.data[i].key == -1)
	  {
		  node.data[i].key = key;
		  node.data[i].rid = rid;
	  }
	  else
	  {
		  int tempKey1 = key;
		  int tempKey2 = 0;
		  RecordId tempRid1 = rid;
		  RecordId tempRid2 = rid;

		  for (; node.data[i].key != -1; i++)
		  {
			  tempKey2 = node.data[i].key;
			  tempRid2 = node.data[i].rid;
			  node.data[i].key = tempKey1;
			  node.data[i].rid = tempRid1;
			  tempKey1 = tempKey2;
			  tempRid1 = tempRid2;
		  }

		  node.data[i].key = tempKey1;
		  node.data[i].rid = tempRid1;
	  }

	  node.numOfData++;

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
	if (key < 0) return RC_INVALID_ATTRIBUTE;
	int half = (int) ((node.size / 2) + 1);
	int i = 0;

	int tempKey = 0;
	RecordId tempRid = rid;

	if (key < node.data[node.size-1].key)
	{
		tempKey = node.data[node.size-1].key;
		tempRid = node.data[node.size-1].rid;
		node.data[node.size-1].key = -1;
		node.numOfData--;
		insert(key, rid);
	}
	else
	{
		tempKey = key;
		tempRid = rid;
	}

	siblingKey = node.data[half].key;

	for (i = half; i < node.size; i++)
	{
		sibling.insert(node.data[i].key, node.data[i].rid);
		node.data[i].key = -1;
	}

	sibling.insert(tempKey, tempRid);
	node.numOfData = half;

	return 0;
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equal to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{
	int i = 0;

	for(; i < node.numOfData; i++)
	{
		if (searchKey <= node.data[i].key)
		{
			eid = i;
			return 0;
		}
	}

	return RC_FILE_SEEK_FAILED;
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
	if (node.data[eid].key == -1) return RC_INVALID_CURSOR;

	key = node.data[eid].key;
	rid = node.data[eid].rid;

	return 0;
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
	return node.nextPid;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	node.nextPid = pid;

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
	char buf[PageFile::PAGE_SIZE];

	if (pf.read(pid, buf) < 0) return RC_FILE_READ_FAILED;

	memcpy(&node, buf, 1024);

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
	return pf.write(pid, &node);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	return node.numOfData;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
	int i = 0;

	if (node.numOfData == node.size) return RC_NODE_FULL;

	for (; i < node.size; i++)
	{
		if (key < node.data[i].key || node.data[i].key == -1)
		{
			break;
		}
	}

	if (node.data[i].key == -1)
	{
		node.data[i].key = key;
		node.data[i].pid = pid;
	}
	else
	{
		int tempKey1 = key;
		int tempKey2 = 0;
		PageId tempPid1 = pid;
		PageId tempPid2 = pid;

		for (; node.data[i].key != -1; i++)
		{
			tempKey2 = node.data[i].key;
			tempPid2 = node.data[i].pid;
			node.data[i].key = tempKey1;
			node.data[i].pid = tempPid1;
			tempKey1 = tempKey2;
			tempPid1 = tempPid2;
		}

		node.data[i].key = tempKey1;
		node.data[i].pid = tempPid1;
	}

	node.numOfData++;

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
	if (key < 0) return RC_INVALID_ATTRIBUTE;

	int half = 0;
	int i = 0;

	int tempKey = 0;
	PageId tempPid = pid;

	if (node.size % 2 == 0)
	{
		half = (int) ((node.size / 2));
	}
	else
	{
		half = (int) ((node.size / 2) + 1);
	}

	if (key < node.data[node.size-1].key)
	{
		tempKey = node.data[node.size-1].key;
		tempPid = node.data[node.size-1].pid;
		node.data[node.size-1].key = -1;
		node.numOfData--;
		insert(key, pid);
	}
	else
	{
		tempKey = key;
		tempPid = pid;
	}

	midKey = node.data[half].key;

	sibling.initializeRoot(node.data[half].pid, node.data[half].key, node.data[half].pid);

	node.data[half].key = -1;
	node.data[half+1].key = -1;

	for (i = half + 2; i < node.size; i++)
	{
		sibling.insert(node.data[i].key, node.data[i].pid);
		node.data[i].key = -1;
	}

	sibling.insert(tempKey, tempPid);
	node.numOfData = half;

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
	int i = 0;

	for(; i < node.numOfData; i++)
	{
		if (searchKey <= node.data[i].key)
		{
			if (i == 0)
				pid = node.firstPid;
			else
				pid = node.data[i-1].pid;//printf("%d %d %d %d\n", searchKey, node.data[i].key, pid, node.data[i-1].key);fflush(stdout);
			return 0;
		}
	}

	pid = node.data[i-1].pid;

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
	node.firstPid = pid1;
	node.data[0].key = key;
	node.data[0].pid = pid2;

	node.numOfData = 1;

	return 0;
}

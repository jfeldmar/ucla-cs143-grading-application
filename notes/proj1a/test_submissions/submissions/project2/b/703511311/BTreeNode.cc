#include "BTreeNode.h"

using namespace std;
	BTLeafNode::BTLeafNode()
	{
		node = new Node;
		isLeafNode = true;
		node->nextPid = -1;
		node->full = false;
		node->numOfData = (int) ((PageFile::PAGE_SIZE - sizeof(PageId) - sizeof(int) - sizeof(bool)) / sizeof(Data));

		for (int i = 0; i < node->numOfData; i++)
		{
			node->data[i].key = -1;
		}
	}

	BTNonLeafNode::BTNonLeafNode()
	{
		node = new Node;
		isLeafNode = false;
		node->pid = -1;
		node->full = false;
		node->numOfData = (int) ((PageFile::PAGE_SIZE - sizeof(PageId) - sizeof(int) - sizeof(bool)) / sizeof(Data));

		for (int i = 0; i < node->numOfData; i++)
		{
			node->data[i].key = -1;
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
	char *buf;

	if (pf.read(pid, buf) < 0) return RC_FILE_READ_FAILED;

	node = (struct Node *) buf;

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
	if (pf.write(pid, (char *) node) < 0) return RC_FILE_WRITE_FAILED;

	return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	int i = 0;

	for(; i < node->numOfData; i++)
	{
		if (node->data[i].key == -1)
		{
			break;
		}
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
	  int i = 0;

	  if (node->full) return RC_NODE_FULL;

	  for (; i < node->numOfData; i++)
	  {
		  if (key < node->data[i].key || node->data[i].key == -1)
		  {
			  break;
		  }
	  }

	  if (node->data[i].key == -1)
	  {
		  node->data[i].key = key;
		  node->data[i].rid = rid;
		  i++;
	  }
	  else
	  {
		  int tempKey1 = key;
		  int tempKey2 = 0;
		  RecordId tempRid1 = rid;
		  RecordId tempRid2 = rid;

		  for (; i < node->numOfData || tempKey1 != -1; i++)
		  {
			  tempKey2 = node->data[i].key;
			  tempRid2 = node->data[i].rid;
			  node->data[i].key = tempKey1;
			  node->data[i].rid = tempRid1;
			  tempKey1 = tempKey2;
			  tempRid1 = tempRid2;
		  }
	  }

	  if (i == node->numOfData)
	  {
		  node->full = true;
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
	if (key < 0) return RC_INVALID_ATTRIBUTE;
	int half = (int) ((node->numOfData / 2) + 1);
	int i = 0;

	int tempKey = 0;
	RecordId tempRid = rid;

	if (key < node->data[node->numOfData-1].key)
	{
		tempKey = node->data[node->numOfData-1].key;
		tempRid = node->data[node->numOfData-1].rid;
		node->data[node->numOfData-1].key = -1;
		insert(key, rid);
	}
	else
	{
		tempKey = key;
		tempRid = rid;
	}

	siblingKey = node->data[half].key;

	for (i = half; i < node->numOfData != -1; i++)
	{
		sibling.insert(node->data[i].key, node->data[i].rid);
		node->data[i].key = -1;
	}

	sibling.insert(tempKey, tempRid);

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

	for(; i < node->numOfData; i++)
	{
		if (searchKey <= node->data[i].key)
		{
			eid = i;
			break;
		}
	}

	if (i == node->numOfData) return RC_FILE_SEEK_FAILED;

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
	if (node->data[eid].key == -1) return RC_INVALID_CURSOR;

	key = node->data[eid].key;
	rid = node->data[eid].rid;

	return 0;
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
	PageId temp = node->nextPid;

	if (temp == -1) return RC_NO_SUCH_RECORD;

	return temp;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	node->nextPid = pid;

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
	char *buf;

	if (pf.read(pid, buf) < 0) return RC_FILE_READ_FAILED;

	node = (struct Node *) buf;

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
	if (pf.write(pid, (char *) node) < 0) return RC_FILE_WRITE_FAILED;

	return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	int i = 0;

	for(; i < node->numOfData; i++)
	{
		if (node->data[i].key == -1)
		{
			break;
		}
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
	int i = 0;

	if (node->full) return RC_NODE_FULL;

	for (; i < node->numOfData; i++)
	{
		if (key < node->data[i].key || node->data[i].key == -1)
		{
			break;
		}
	}

	if (node->data[i].key == -1)
	{
		node->data[i].key = key;
		node->data[i].pid = pid;
		i++;
	}
	else
	{
		int tempKey1 = key;
		int tempKey2 = 0;
		PageId tempPid1 = pid;
		PageId tempPid2 = pid;

		for (; i < node->numOfData || tempKey1 != -1; i++)
		{
			tempKey2 = node->data[i].key;
			tempPid2 = node->data[i].pid;
			node->data[i].key = tempKey1;
			node->data[i].pid = tempPid1;
			tempKey1 = tempKey2;
			tempPid1 = tempPid2;
		}
	}

	if (i == node->numOfData)
	{
		node->full = true;
	}

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

	if (node->numOfData % 2 == 0)
	{
		half = (int) ((node->numOfData / 2));
	}
	else
	{
		half = (int) ((node->numOfData / 2) + 1);
	}

	if (key < node->data[node->numOfData-1].key)
	{
		tempKey = node->data[node->numOfData-1].key;
		tempPid = node->data[node->numOfData-1].pid;
		node->data[node->numOfData-1].key = -1;
		insert(key, pid);
	}
	else
	{
		tempKey = key;
		tempPid = pid;
	}

	midKey = node->data[half].key;

	for (i = half + 1; i < node->numOfData != -1; i++)
	{
		sibling.insert(node->data[i].key, node->data[i].pid);
		node->data[i].key = -1;
	}

	sibling.insert(tempKey, tempPid);

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

	for(; i < node->numOfData; i++)
	{
		if (searchKey <= node->data[i].key)
		{
			pid = node->data[i].pid;
			break;
		}
	}

	if (i == node->numOfData)
	{
		pid = node->pid;
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
	node->data[0].key = key;
	node->data[0].pid = pid1;
	node->pid = pid2;

	return 0;
}

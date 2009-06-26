#include "BTreeNode.h"
#include <algorithm>
using namespace std;

/**
* Default constructor for BTLeafNode
*/
BTLeafNode::BTLeafNode()
{
	//Note that a buffer in reality has 2 more LeafPairs than the size
	//in order to store the size, count, and next_pid
	size = MAX_NODE_SIZE+1;
	count = 0;
	next_pid = -1;
	buffer = new LeafPair[MAX_NODE_SIZE+1];
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
	RC rc = pf.read(pid,buffer);
	if (rc != 0)
		return rc;
	count = buffer[0].key;
	size = buffer[0].rid.pid;
	next_pid = buffer[0].rid.sid;
	return rc;
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
	buffer[0].key = count;
	buffer[0].rid.pid = size;
	buffer[0].rid.sid = next_pid;
	return pf.write(pid,buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
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
	//Check if node is full
	//(1*3*4) + (84*3*4) is close to 1024 as a LeafPair struct has 3 integers
	//We also need 3 ints for the first entry, so
	if (count == MAX_NODE_SIZE)	{
		return RC_NODE_FULL;
	}

	//Insert the pair at the end
	LeafPair newpair;
	newpair.key = key;
	newpair.rid = rid;
	buffer[count + 1] = newpair;

	//Increment count
	count++;

	//Sort the buffer using STL sort excluding first 2
	sort(buffer+1, buffer+1+count);

	//Return
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
	//Figuring out the count of the new nodes
	int new_count1;
	int new_count2;
	if (count%2 == 0){
		new_count1 = count/2;
		new_count2 = count/2;
	}
	else {
		new_count1 = count/2 + 1;
		new_count2 = count/2;
	}

	//Inserting the nodes in the sibling
	for (int i = 0; i < new_count2; ++i)	{
		sibling.insert(buffer[new_count1+1+i].key, buffer[new_count1+1+i].rid);
	}

	//Set next_pid of the sibling
	sibling.setNextNodePtr(next_pid);

	//Set new count of this node
	//Count of sibling node set automatically
	count = new_count1;

	int rc;	//return code

	//Set siblingKey
	RecordId id;
	rc = sibling.readEntry(0,siblingKey,id);	if (rc != 0)	return rc;


	//Insert key pair in the node
	if (key >= siblingKey)	{
		rc = sibling.insert(key,rid);
	}
	else	{
		rc = insert(key,rid);
	}

	return rc;



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
	//Find key value larger than or equal to
	for (int i = 0; i < count; ++i)	{
		if (searchKey <= buffer[i + 1].key) {
			eid = i;
			return 0;
		}
	}
	//If not found
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
	if (eid >= count)
		return RC_NO_SUCH_RECORD;

	key = buffer[eid + 1].key;
	rid = buffer[eid + 1].rid;

	return 0;
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
	if (next_pid == -1)	return RC_END_OF_TREE;
	return next_pid;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	next_pid = pid;
	return 0;
}

/*
 * Default constructor for BTNonLeafNode
 */
BTNonLeafNode::BTNonLeafNode()
{
	size = MAX_NODE_SIZE + 2;
	count = 0;
	buffer = new NonLeafPair[MAX_NODE_SIZE+2];
	first_pid = -1;
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
	RC rc = pf.read(pid,buffer);
	if (rc != 0)
		return rc;
	count = buffer[0].key;
	size = buffer[0].pid;
	//skip buffer[1].key
	first_pid = buffer[1].pid;
	return rc;
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
	buffer[0].key = count;
	buffer[0].pid = size;
	buffer[1].key = 0;
	buffer[1].pid = first_pid;
	return pf.write(pid,buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
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
	//Check if node is full
	if (count == MAX_NODE_SIZE)	{
		return RC_NODE_FULL;
	}
	
	//Insert the pair at the end
	NonLeafPair newpair;
	newpair.key = key;
	newpair.pid = pid;
	buffer[count + 2] = newpair;

	//Increment count
	count++;

	//Sort the buffer using STL sort excluding first 2
	sort(buffer+2, buffer+2+count);

	//Return
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
	//Figuring out the count of the new nodes
	int new_count1;
	int new_count2;
	int mid;
	if (count%2 == 0)	{
		new_count1 = count/2;
		new_count2 = count/2 - 1;
		mid = count/2;
	}
	else {
		new_count1 = count/2;
		new_count2 = count/2;
		mid = count/2;
	}

	//Inserting the nodes in the sibling
	for (int i = 0; i < new_count2; ++i)	{
		sibling.insert(buffer[new_count1+2+1+i].key, buffer[new_count1+2+1+i].pid);
	}

	//Retrieving midkey's key and setting it to midkey
	midKey = buffer[mid + 2].key;

	//Inserting midkey's pointer to beginning of sibling
	sibling.set_first_pid(buffer[mid + 2].pid);

	//Set new count of this node
	count = new_count1;

	//Insert key pair in the node
	if (key >= midKey)
		return sibling.insert(key,pid);
	else
		return insert(key,pid);
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
	//If searchKey less than all the keys
	if (searchKey < buffer[2].key)	{
		pid = first_pid;
		return 0;
	}

	//If searchKey greater or equal to one of the keys
	for (int i = 1; i < count; ++i)	{
		if (searchKey < buffer[i+2].key)	{
			pid = buffer[i-1+2].pid;
			return 0;
		}
	}

	//If searchKey greater than all the keys
	if (searchKey >= buffer[count-1+2].key)	{
		pid = buffer[count-1+2].pid;
		return 0;
	}

	//If not found
	return RC_NO_SUCH_RECORD;
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
	first_pid = pid1;
	return insert(key,pid2);
}

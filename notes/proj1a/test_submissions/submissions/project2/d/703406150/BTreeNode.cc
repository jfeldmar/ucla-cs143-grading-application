#include "BTreeNode.h"
#include <algorithm>
#include <iostream>

using namespace std;

//#define	DEBUG

// to use the built-in array sort function
bool operator<(const leafTuple& a, const leafTuple& b)
{
	// sort by key
    return a.key < b.key;
}

// sort the array
void BTLeafNode::sorter()
{
	// use the built-in sort function
	sort(entry, entry + keyCount);
	return;
}

// to use the built-in array sort function
bool operator<(const nonLeafTuple& a, const nonLeafTuple& b)
{
	// sort by key
    return a.key < b.key;
}

// sort the array
void BTNonLeafNode::sorter()
{
	// use the built-in sort function
	sort(entry, entry + keyCount);
	return;
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
	#ifdef DEBUG
	cout << "BTLeafNode::read" << endl;
	#endif
	RC err;
	// check for invalid pid
	if (pid < 1) return RC_INVALID_PID;
	
	// buffer to read pagefile page into
	int* buff = new int[256];
	// create a leaf tuple
	leafTuple mon;
	
	// read a page from the pagefile into the buffer
	err = pf.read(pid, buff);
	// check for error
	if (err != 0) return err;
	
	// get data members
	leaf = buff[0];
	keyCount = buff[1];
	sibPid = buff[2];
	parPid = buff[255];
	
	// get the array elements
	for (int i = 0; i < keyCount; i++)
	{
		// get the key
		mon.key = buff[(i*3)+3];
		// get the pid
		mon.pid = buff[(i*3)+4];
		// get the sid
		mon.sid = buff[(i*3)+5];
		// set the tuple
		entry[i] = mon;
	}
	// clean up memory
	delete [] buff;
	// return result
	return err;
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
	#ifdef DEBUG
	cout << "BTLeafNode::write" << endl;
	#endif
	RC err;
	// check for invalid pid
	if (pid < 1) return RC_INVALID_PID;
	
	// buffer to read pagefile page into
	int* buff = new int[256];
	// create a leaf tuple
	leafTuple mon;
	
	// set the node members
	buff[0] = 1;
	buff[1] = keyCount;
	buff[2] = sibPid;
	buff[255] = parPid;
	
	// set the array elements
	for (int i = 0; i < keyCount; i++)
	{
		// get the tuple
		mon = entry[i];
		// set the key
		buff[(i*3)+3] = mon.key;
		// set the pid
		buff[(i*3)+4] = mon.pid;
		// set the sid
		buff[(i*3)+5] = mon.sid;
	}
	// write a page to the pagefile from the buffer
	err = pf.write(pid, buff);
	// clean up memory
	delete [] buff;
	// return result
	return err;
}

// returns the number of keys in the node
int BTLeafNode::getKeyCount()
{
	// return member variable
	return keyCount;
}

// return leaf status
bool BTLeafNode::isLeaf()
{
	return leaf;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
	#ifdef DEBUG
	cout << "BTLeafNode::insert" << endl;
	#endif
	// check if node is full
	if (keyCount == MAX_TUPLES) return -1010;
	
	// create a new leaf tuple
	leafTuple stu;
	// set the members
	stu.pid = rid.pid;
	stu.sid = rid.sid;
	stu.key = key;
	// add the new leaf tuple to the node
	entry[keyCount] = stu;
	
	// increment the key count
	keyCount += 1;
	// sort the array
	sorter();
	// return
	return 0;
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param midKey[OUT] the first key in the sibling node after the split
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
									BTLeafNode& sibling, int& midKey)
{
	#ifdef DEBUG
	cout << "BTLeafNode::insertAndSplit" << endl;
	#endif
	RC err;
	// initialize a leaf tuple to the 'split' entry
	// this will be the first entry in the new node after the split
	leafTuple mon = entry[BREAK_POINT];
	
	// copy the last half of the entries into the new node
	for (int i = BREAK_POINT; i < MAX_TUPLES; i++)
	{
		sibling.entry[i-BREAK_POINT] = entry[i];
	}
	// set the new key counts
	keyCount = BREAK_POINT;
	sibling.keyCount = BREAK_POINT;
	
	// check which node the new entry sould go in
	// if the new key is greater than the first key in the new node
	if (key > mon.key)
		// put it in the new node
		err = sibling.insert(key, rid);
	else
		// put it in the old node
		err = insert(key, rid);
	
	// set the mid key
	midKey = mon.key;
	
	// return success
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
	#ifdef DEBUG
	cout << "BTLeafNode::locate" << endl;
	#endif
	// create leaf tuple
	leafTuple mon;
	
	// get the first entry
	mon = entry[0];
	// check if we have something like key > -5000
	if (searchKey < mon.key)
	{
		// set eid to 0
		eid = 0;
		return 0;
	}
	
	// check each entry for the search key
	for (int i = 0; i < keyCount; i++)
	{
		// get the current entry
		mon = entry[i];
		// if we've found the key we're looking for
		if (mon.key >= searchKey)
		{
			// set the entry id to the current entry number
			eid = i;
			// return success
			return 0;
		}
	}
	// return
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
	#ifdef DEBUG
	cout << "BTLeafNode::readEntry" << endl;
	#endif
	// if the eid is out of bounds
	if ((eid < 0) || (eid > keyCount)) return -1;
		
	// initialize a leaf tuple
	leafTuple mon = entry[eid];
	// set the return values
	key = mon.key;
	rid.pid = mon.pid;
	rid.sid = mon.sid;
	// return success
	return 0;
}

/*
 * Output the pid of the next sibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getSibPid()
{
	// check for invalid sibPid
	if (sibPid == -1) return -37;
	// return the member variable
	return sibPid;
}

/*
 * Set the pid of the next sibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setSibPid(PageId pid)
{
	// set the member variable
	sibPid = pid;
	// return success
	return 0;
}

// set parPid
void BTLeafNode::setParPid(int pid)
{
	parPid = pid;
	return;
}

// get parPid
int BTLeafNode::getParPid()
{
	// who's your daddy?
	return parPid;
}

////////////////////////////////////////////////////////////////////////////////
//			Non Leaf Node
////////////////////////////////////////////////////////////////////////////////

/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{
	#ifdef DEBUG
	cout << "BTNonLeafNode::read" << endl;
	#endif
	RC err;
	// check for invalid pid
	if (pid < 1) return RC_INVALID_PID;
	
	// buffer to read pagefile page into
	int* buff = new int[256];
	// create a non leaf tuple
	nonLeafTuple mon;
	
	// read a page from the pagefile into the buffer
	err = pf.read(pid, buff);
	// check for error
	if (err != 0) return err;
	
	// get data members
	leaf = buff[0];
	keyCount = buff[1];
	firstChildPid = buff[2];
	parPid = buff[255];
	
	// get the array elements
	for (int i = 0; i < keyCount; i++)
	{
		// get the key
		mon.key = buff[(i*2)+3];
		// get the pid
		mon.pid = buff[(i*2)+4];
		// set the tuple
		entry[i] = mon;
	}
	// clean up memory
	delete [] buff;
	// return result
	return err;
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
	#ifdef DEBUG
	cout << "BTNonLeafNode::write" << endl;
	#endif
	RC err;
	// check for invalid pid
	if (pid < 1) return RC_INVALID_PID;
	
	// buffer to read pagefile page into
	int* buff = new int[256];
	// create a non leaf tuple
	nonLeafTuple mon;
	
	// set the node members
	buff[0] = 0;
	buff[1] = keyCount;
	buff[2] = firstChildPid;
	buff[255] = parPid;
	
	// set the array elements
	for (int i = 0; i < keyCount; i++)
	{
		// get the tuple
		mon = entry[i];
		// set the key
		buff[(i*2)+3] = mon.key;
		// set the pid
		buff[(i*2)+4] = mon.pid;
	}
	// write a page to the pagefile from the buffer
	err = pf.write(pid, buff);
	// clean up memory
	delete [] buff;
	// return result
	return err;
}

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
	#ifdef DEBUG
	cout << "BTNonLeafNode::insert" << endl;
	#endif
	// check if node is full
	if (keyCount == MAX_TUPLES) return -1010;
	
	// create a new non leaf tuple
	nonLeafTuple stu;
	// set the members
	stu.pid = pid;
	stu.key = key;
	// add the new leaf tuple to the node
	entry[keyCount] = stu;
	
	// increment the key count
	keyCount += 1;
	// sort the array
	sorter(); 
	// return success
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
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling,
								 int& midKey)
{
	#ifdef DEBUG
	cout << "BTNonLeafNode::insertAndSplit" << endl;
	#endif
	RC err;
	// create a tuple
	nonLeafTuple mon;
	
	// copy the last half of the entries into the new node
	for (int i = BREAK_POINT; i < MAX_TUPLES; i++)
	{
		sibling.entry[i-BREAK_POINT] = entry[i];
	}
	// set the new key counts
	keyCount = BREAK_POINT;
	sibling.keyCount = BREAK_POINT;
	
	// check which node the new key sould go in
	// if the new key is greater than all the keys in the old node
	if (key > mon.key)
		// put it in the new node
		err = sibling.insert(key, pid);
	else
		// put it in the old node
		err = insert(key, pid);
	
	// "remove" the first entry in the new node
	mon = sibling.entry[0];
	sibling.entry[0] = sibling.entry[keyCount-1];
	sibling.keyCount -= 1;
	// set the new node's first child
	sibling.firstChildPid = mon.pid;
	// set the midkey
	midKey = mon.key;
	// return result
	return err;
	// need to call insert into parent after this call in B+treeindex
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
	#ifdef DEBUG
	cout << "BTNonLeafNode::locateChildPtr" << endl;
	#endif
	// instancaiate non leaf tuple
	nonLeafTuple mon;
	
	// check if we need to follow first child pid
	mon = entry[0];
	// if we should follow the first child pid
	if (searchKey < mon.key)
	{
		// set pid to last child pid
		pid = firstChildPid;
		// return success
		return 0;
	}
	
	// find the key that is > the search key, from large to small
	for (int i = (keyCount-1); i >= 0; i--)
	{
		// get the tuple data
		mon = entry[i];
		// if we found the entry we're looking for
		if (searchKey >= mon.key)
		{
			// set the pid
			pid = mon.pid;
			// return success
			return 0;
		}
	}
	// if we didn't find it, return failure, shouldn't happen
	return -1;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first child pid
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{
	#ifdef DEBUG
	cout << "BTNonLeafNode::initializeRoot" << endl;
	#endif
	RC err;
	// set the first child pid
	firstChildPid = pid1;
	// insert the key pid
	err = insert(key, pid2);
	// return
	return err;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	// return the member variable
	return keyCount;
}

// return leaf status
bool BTNonLeafNode::isLeaf()
{
	return leaf;
}

// set the first child pid
void BTNonLeafNode::setFirstChildPid(int pid)
{
	firstChildPid = pid;
}

// get the first child pid
int BTNonLeafNode::getFirstChildPid()
{
	return firstChildPid;
}

// set parPid
void BTNonLeafNode::setParPid(int pid)
{
	parPid = pid;
	return;
}

// get parPid
int BTNonLeafNode::getParPid()
{
	// who's your daddy?
	return parPid;
}

// set all the children nodes' parent pids to pid
// called when there is non leaf overflow
RC BTNonLeafNode::setChildrenParPids(int pid, PageFile& pf)
{
	#ifdef DEBUG
	cout << "BTNonLeafNode::setChildrenParPids" << endl;
	#endif
	RC err;
	// variables
	BTNonLeafNode node;
	BTLeafNode edon;
	nonLeafTuple stu;
	int k, childType = -1;
	
	// for each child of this node
	for (int i = 0; i < keyCount; i++)
	{
		// get the entry
		stu = entry[i];
		// get the pid
		k = stu.pid;
		
		// if this is the first child looked at
		if (childType == -1)
		{
			// get the node
			err = node.read(k, pf);
			if (err != 0) return err;
			
			// check if the node is a leaf
			if (node.isLeaf() == 1) childType = 1;
			// check if the node is a non leaf
			else if (node.isLeaf() == 0) childType = 0;
			// should never happen
			else return -1;
		}
		
		// check if it's a leaf node
		if (childType == 1)
		{
			// read the node as a leaf
			err = edon.read(k, pf);
			if (err != 0) return err;
			
			// set the parent pid
			edon.setParPid(pid);
			
			// save the node
			err = edon.write(k, pf);
			if (err != 0) return err;
		}
		// if it's a non leaf
		else if (childType == 0)
		{
			// read the node as a non leaf
			err = node.read(k, pf);
			if (err != 0) return err;
			
			// set the parent pid
			node.setParPid(pid);
			
			// save the node
			err = node.write(k, pf);
			if (err != 0) return err;
		}
		// should never happen
		else return -1;
	}
	// everything's ok if we've gotten this far
	return 0;
}


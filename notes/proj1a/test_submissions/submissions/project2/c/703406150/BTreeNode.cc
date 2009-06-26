#include "BTreeNode.h"
#include <algorithm>
#include <iostream>

using namespace std;

#define	DEBUG

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
	if (pid < 0) 
		return RC_INVALID_PID;
	// read a page from the pagefile into the node
	err = pf.read(pid, this);
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
	if (pid < 0) 
		return RC_INVALID_PID;
	// write a page to the pagefile from the node
	err = pf.write(pid, this);
	// return result
	return err;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	// return member variable
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
	#ifdef DEBUG
	cout << "BTLeafNode::insert" << endl;
	#endif
	// check if node is full
	if (keyCount == MAX_TUPLES)
		// return failure
		return -1010;
	// create a new leaf tuple
	leafTuple foo;
	// set the members
	foo.pid = rid.pid;
	foo.sid = rid.sid;
	foo.key = key;
	// add the new leaf tuple to the node
	entry[keyCount] = foo;
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
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{
	#ifdef DEBUG
	cout << "BTLeafNode::insertAndSplit" << endl;
	#endif
	RC err;
	// instanciate a leaf tuple
	leafTuple bar;
	
	// copy half the entries into the new node
	for (int i = BREAK_POINT; i < MAX_TUPLES; i++)
	{
		sibling.entry[i-BREAK_POINT] = entry[i];
	}
	// set the new key counts
	keyCount = BREAK_POINT;
	sibling.keyCount = BREAK_POINT;
	
	// set bar to the first entry in the new node
	bar = sibling.entry[0];
	// set the sibling key
	siblingKey = bar.key;
	
	// check which node the new entry sould go in
	// if the new key is greater than all the keys in the old node
	if (key > bar.key)
		// put it in the new node
		err = sibling.insert(key, rid);
	else
		// put it in the old node
		err = insert(key, rid);
		
	// set the sibPid of old node to sibId of new node
	sibling.sibPid = sibPid;
	// set the sibPid of old node to pid of new node
// this will be done in the btree index implementation
// just call set sibling node
		
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
	// instanciate leaf tuple
	leafTuple foo;
	// check the entries for the search key
	for (int i = 0; i < keyCount; i++)
	{
		// get the current entry
		foo = entry[i];
		// if we've found the key we're looking for
		if (foo.key >= searchKey)
		{
			// set the entry id to the current entry number
			eid = i;
			// return success
			return 0;
		}
	}
	// if we somehow made a mistake, return failure
	return -1;
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
	if ((eid < 0) || (eid > keyCount))
		// return failure
		return -1;
		
	// initialize a leaf tuple
	leafTuple bar = entry[eid];
	// set the return values
	key = bar.key;
	rid.pid = bar.pid;
	rid.sid = bar.sid;
	// return success
	return 0;
}

/*
 * Output the pid of the next sibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
	// return the member variable
	return sibPid;
}

/*
 * Set the pid of the next sibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	// set the member variable
	sibPid = pid;
	// return success
	return 0;
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
	if (pid < 0) 
		return RC_INVALID_PID;
	// read a page from the pagefile to the node
	err = pf.read(pid, this);
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
	if (pid < 0) 
		return RC_INVALID_PID;
	// write a page to the pagefile from the node
	err = pf.write(pid, this);
	// return result
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
	if (keyCount == MAX_TUPLES)
		return -1010;
	// create a new non leaf tuple
	nonLeafTuple foo;
	// set the members
	foo.pid = pid;
	foo.key = key;
	// add the new leaf tuple to the node
	entry[keyCount] = foo;
	// increment the key count
	keyCount += 1;
	// sort the array
	sorter(); 
	// return
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
	// initialize the "split" tuple
	nonLeafTuple bar = entry[BREAK_POINT];
	
	// copy half the entries into the new node
	for (int i = BREAK_POINT; i < MAX_TUPLES; i++)
	{
		sibling.entry[i-BREAK_POINT] = entry[i];
	}
	// set the new key counts
	keyCount = BREAK_POINT;
	sibling.keyCount = BREAK_POINT;
	
	// set the new last child pid
	sibling.lastChildPid = lastChildPid;
	
	// check which node the new key sould go in
	// if the new key is greater than all the keys in the old node
	if (key > bar.key)
		// put it in the new node
		err = sibling.insert(key, pid);
	else
		// put it in the old node
		err = insert(key, pid);
	
	// update old node last child pid
	lastChildPid = bar.pid;
	// update the first entry's pid in new node
	bar = sibling.entry[0];
	bar.pid = pid;
	sibling.entry[0] = bar;
	
	// get the last entry in old node
	bar = entry[keyCount-1];
	// set the mid key
	midKey = bar.key;
	// remove the last entry in old node by decrementing key count
	keyCount -= 1;
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
	// non leaf tuple to check
	nonLeafTuple foo;
	
	// find the key that is > the search key
	for (int i = 0; i < keyCount; i++)
	{
		// get the tuple data
		foo = entry[i];
		// if we found the entry we're looking for
		if (foo.key > searchKey)
		{
			// set the pid
			pid = foo.pid;
			// return success
			return 0;
		}
	}
	// check if we need to follow last child pid
	foo = entry[keyCount-1];
	// if we should follow the last child pid
	if (searchKey >= foo.key)
	{
		// set pid to last child pid
		pid = lastChildPid;
		// return success
		return 0;
	}
	// if we didn't find it, return failure, shouldn't happen
	return -1;
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
	#ifdef DEBUG
	cout << "BTNonLeafNode::initializeRoot" << endl;
	#endif
	RC err;
	// insert the key pid
	err = insert(key, pid1);
	// set the last child pid
	lastChildPid = pid2;
	// return
	return err;
}


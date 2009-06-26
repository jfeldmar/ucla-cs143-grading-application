#include <iostream>
#include "BTreeNode.h"

using namespace std;

BTLeafNode::BTLeafNode()
: nkeys(0)
{
  buffer = new bool[PAGE_SIZE];
  resetBuffer();
}

BTLeafNode::~BTLeafNode()
{
  delete buffer;
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
  RC rc = pf.read(pid, buffer);
  nkeys = btod(1, 8);
  //printf("Reading: nkeys is %d\n", nkeys);
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
  //printf("Writing: nkeys is %d\n", nkeys);
  //writenkeys();
  RC rc = pf.write(pid, buffer);
  return rc;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
  return nkeys;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
  if (isFull())
  {
	//printf("Node is full\n");
    return RC_NODE_FULL;
  }
  
  if (nkeys == 0)
  {
	dtob(getPos(0), getPos(0)+32, key);
	dtob(getPos(0)+32, getPos(0)+64, rid.pid);
	dtob(getPos(0)+64, getPos(0)+96, rid.sid);
  }
  else
  {
    int eid = 0;
    locate(key, eid);
	if (eid < nkeys)
		shift(eid);
	dtob(getPos(eid), getPos(eid)+32, key);
	dtob(getPos(eid)+32, getPos(eid)+64, rid.pid);
	dtob(getPos(eid)+64, getPos(eid)+96, rid.sid);
  }
  
  nkeys++;
  writenkeys();
  
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
  int tkey;
  RecordId trid;
  int newnkeys = nkeys;
  
  // Insert all keys and records after halfway point into sibling node
  for (int i = nkeys/2; i < nkeys; i++)
  {
    readEntry(i, tkey, trid);
	if (sibling.insert(tkey, trid) != 0)
      return RC_NODE_FULL;
	
    // Zero out corresponding key and record id
	dtob(getPos(i), getPos(i)+32, 0);
	dtob(getPos(i)+32, getPos(i)+64, 0);
	dtob(getPos(i)+64, getPos(i)+96, 0);
	
	// Remove a key
	newnkeys--;
  }
  
  nkeys = newnkeys;
  writenkeys();
  
  // Insert the key
  insert(key, rid);
  
  sibling.readEntry(0, siblingKey, trid);
  
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
  int k = 0;
  for (int i = 0; i <= nkeys; i++)
  {
    if (i == nkeys)
	{
	  eid = i;
	  return 0;
	}
	else
	{
	    k = getKey(i);
	    if (k >= searchKey)
		{
		  eid = i;
		  return 0;
		}
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
  if (eid > TOTAL_LEAF_KEYS || eid < 0)
    return RC_NO_SUCH_RECORD;
  int p = getPos(eid);
  key = getKey(eid);
  rid.pid = btod(getPos(eid)+32, getPos(eid)+64);
  rid.sid = btod(getPos(eid)+64, getPos(eid)+96);
  return 0;
}

/*
 * Output the pid of the next sibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
  int d = btod(PAGE_SIZE-32, PAGE_SIZE);
  if (d != 0)
    return d;
  else
    return RC_NO_SUCH_RECORD;
}

/*
 * Set the pid of the next sibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
  if (pid >= 0)
  {
    dtob(PAGE_SIZE-32, PAGE_SIZE, pid);
	return 0;
  }
  else
    return RC_INVALID_PID;
}



/* Start of Auxiliary Functions */

// Returns true if the node is full
bool BTLeafNode::isFull()
{
  return (nkeys >= TOTAL_LEAF_KEYS) ? true : false;
}

// Write the number of keys into the buffer
void BTLeafNode::writenkeys()
{
  dtob(1, 8, nkeys);
}

// Get the key of an entry given the entry id
int BTLeafNode::getKey(int eid)
{
  return btod(getPos(eid), getPos(eid)+32);
}

// Get the position in the buffer given an entry id
int BTLeafNode::getPos(int eid)
{
  return 8 + eid * 96;
}

// Move entry ids over to the right
void BTLeafNode::shift(int eid)
{
  int last = nkeys - 1;
  while (last >= eid)
  {
    copy(last, last + 1);
	last--;
  }
}

// Copy an entry id into another entry id
void BTLeafNode::copy(int eid, int newEid)
{
  int newPtr = getPos(newEid);
  int oldPtr = getPos(eid);
  for (int i = 0; i < 96; i++)
  {
    buffer[newPtr + i] = buffer[oldPtr + i];
  }
}

// Convert eights bits into decimal starting at position pos
// and return it
int BTLeafNode::btod(unsigned int pos, unsigned int endPos)
{
  unsigned int div = 1;
  for (int i = 0; i < endPos - pos - 1; i++)
    div *= 2;
  int total = 0;
  for (pos; pos < endPos; pos++)
  {
    total += div * buffer[pos];
	div = div>>1;
  }
  return total;  
}

// Convert decimal to binary and put it in the eight bits
// starting at position pos
void BTLeafNode::dtob(unsigned int pos, unsigned int endPos, int n)
{
  unsigned int div = 1;
  for (int i = 0; i < endPos - pos - 1; i++)
    div *= 2;
  for (int i = pos; i < endPos; i++)
  {
    //cout << div << endl;
    if (n >= div)
	{
	  buffer[i] = 1;
	  n -= div;
	} 
	else
	  buffer[i] = 0;
	  
	div = div>>1;
  }
}

// Print out the buffer
void BTLeafNode::debug()
{
  std::cout << endl << endl;
  for (int i = 0; i < PAGE_SIZE; i++)
  {
    if (buffer[i] == 0)
		std::cout << "0";
	else
	    std::cout << "1";
    if (((i + 1) % 8) == 0)
	  std::cout << " ";
  }
}

// Reset the buffer to zeros
void BTLeafNode::resetBuffer()
{
  nkeys = 0;
  for (int i = 0; i < PAGE_SIZE; i++)
    buffer[i] = 0;
}

/* End of Auxiliary Functions */



/*
 *
 *	NON-LEAF NODES
 *
*/




BTNonLeafNode::BTNonLeafNode()
: nkeys(0)
{
  buffer = new bool[PAGE_SIZE];
  resetBuffer();
}

BTNonLeafNode::~BTNonLeafNode()
{
  delete buffer;
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
  RC rs = pf.read(pid, buffer);
  
  // Read the number of keys
  nkeys = btod(1, 8);

  return rs;
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
  // Write the number of keys
  //writenkeys();
  
  RC rs = pf.write(pid, buffer);
  return rs;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
  return nkeys;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
  if (isFull())
    return RC_NODE_FULL;
  
  if (nkeys == 0)
  {
	dtob(getPos(0), getPos(0)+32, key);
	dtob(getPos(0)+32, getPos(0)+64, pid);
  }
  else
  {
    int eid = 0;
    locate(key, eid);
	if (eid < nkeys)
		shift(eid);
	dtob(getPos(eid), getPos(eid)+32, key);
	dtob(getPos(eid)+32, getPos(eid)+64, pid);	
  }
  
  nkeys++;
  writenkeys();
  
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
  int tkey;
  PageId tpid;
  int newnkeys = nkeys;
  
  // Insert all keys and records after halfway point into sibling node
  for (int i = nkeys/2; i < nkeys; i++)
  {
    readEntry(i, tkey, tpid);
	if (i != nkeys/2)
	{
		if (sibling.insert(tkey, tpid) != 0)
			return RC_NODE_FULL;
	}
	else
		midKey = tkey;
	
    // Zero out corresponding key and record id
	dtob(getPos(i), getPos(i)+32, 0);
	dtob(getPos(i)+32, getPos(i)+64, 0);
	
	// Remove a key
	newnkeys--;
  }
  
  nkeys = newnkeys;
  writenkeys();
  
  // Insert the key
  insert(key, pid);
  
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
  int eid = 0;
  locate(searchKey, eid);
  //printf("eid is %d\ngetPos(eid) is %d", eid, getPos(eid));
  //debug();
  pid = btod(getPos(eid)-32, getPos(eid));
  //printf("locateChildPtr returned %d\n", pid);
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
  buffer[0] = 1;
  dtob(8, 40, pid1);
  if(insert(key, pid2))
    return RC_INVALID_CURSOR;
  return 0;
}



/* Start of Auxiliary Functions */

// Returns true if the node is full
bool BTNonLeafNode::isFull()
{
  int n = nkeys;
  return (n >= TOTAL_NONLEAF_KEYS) ? true : false;
}

// Write the number of keys into the buffer
void BTNonLeafNode::writenkeys()
{
  dtob(1, 8, nkeys);
}

// Get the key of an entry given the entry id
int BTNonLeafNode::getKey(int eid)
{
  return btod(getPos(eid), getPos(eid)+32);
}

// Get the position in the buffer given an entry id
int BTNonLeafNode::getPos(int eid)
{
  return 40 + eid * 64;
}

// Move entry ids over to the right
void BTNonLeafNode::shift(int eid)
{
  int last = nkeys - 1;
  while (last >= eid)
  {
    copy(last, last + 1);
	last--;
  }
}

// Copy an entry id into another entry id
void BTNonLeafNode::copy(int eid, int newEid)
{
  int newPtr = getPos(newEid);
  int oldPtr = getPos(eid);
  for (int i = 0; i < 64; i++)
  {
    buffer[newPtr + i] = buffer[oldPtr + i];
  }
}

// Convert eights bits into decimal starting at position pos
// and return it
int BTNonLeafNode::btod(unsigned int pos, unsigned int endPos)
{
  unsigned int div = 1;
  for (int i = 0; i < endPos - pos - 1; i++)
    div *= 2;
  int total = 0;
  for (pos; pos < endPos; pos++)
  {
	//printf("buffer[pos] is %d\n", buffer[pos]);
    total += div * buffer[pos];
	div = div>>1;
  }
  return total;  
}

// Convert decimal to binary and put it in the eight bits
// starting at position pos
void BTNonLeafNode::dtob(unsigned int pos, unsigned int endPos, int n)
{
  unsigned int div = 1;
  for (int i = 0; i < endPos - pos - 1; i++)
    div *= 2;
  for (int i = pos; i < endPos; i++)
  {
    if (n >= div)
	{
	  buffer[i] = 1;
	  n -= div;
	} 
	else
	  buffer[i] = 0;
	  
	div = div>>1;
  }
}

// Read the key and pid given an eid
RC BTNonLeafNode::readEntry(int eid, int& key, PageId& pid)
{
  //int p = getPos(eid);
  key = getKey(eid);
  pid = btod(getPos(eid)+32, getPos(eid)+64);
  return 0;
}

// Find the search key in the page
RC BTNonLeafNode::locate(int searchKey, int& eid)
{
  int k = 0;
  for (int i = 0; i <= nkeys; i++)
  {  
    if (i == nkeys)
	{
	  eid = i;
	  return 0;
	}
	else
	{
	    k = getKey(i);
	    if (k > searchKey)
		{
		  eid = i;
		  return 0;
		}
	}
  }
  return RC_NO_SUCH_RECORD;
}

// Reset the buffer to zeros
void BTNonLeafNode::resetBuffer()
{
  nkeys = 0;
  for (int i = 0; i < PAGE_SIZE; i++)
    buffer[i] = 0;
}

// Print out the buffer
void BTNonLeafNode::debug()
{
  std::cout << endl << endl;
  for (int i = 0; i < PAGE_SIZE; i++)
  {
    if (buffer[i] == 0)
		std::cout << "0";
	else
	    std::cout << "1";
    if (((i + 1) % 8) == 0)
	  std::cout << " ";
  }
}

bool BTNonLeafNode::isRoot()
{
  return buffer[0] ? true : false;
}

/* End of Auxiliary Functions */

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
{ //XXX: note that this has been kinda tested
  geneprintf("in BTLeafNode::read(pid==%d, pf)\n", pid);
  RC rc = 0;

  // Fill buffer with INT_MIN value
  int buffer[PageFile::PAGE_SIZE/sizeof(int)];
  for (int z = 0; z < PageFile::PAGE_SIZE/sizeof(int); ++z) {
    buffer[z] = INT_MIN;
  }

  // Read page from disk into buffer
  bool is_old_node = true;
  if ((rc = pf.read(pid, buffer)) < 0 && rc == RC_INVALID_PID) {
    geneprintf("BTLeafNode::read()\tCannot read page %d from disk into buffer\trc==%d\n\t=> Assuming is new node\n",pid,rc);
    node.clear();
    is_old_node = false;
    rc = 0;
  } else if (rc < 0) { return rc; }
  node_pid = pid;

#if verbose>1
  for (int z = 0; z < PageFile::PAGE_SIZE/sizeof(int); ++z)
    geneprintf("%d ",buffer[z]);
  geneprintf("\n");
#endif

  /* Nodes stored in page look like:
   *  +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
   *  | pid | sid | key | pid | sid | key | ... | pid | sid | key | pageid |
   *  +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
   * rec_id is type RecordId and key is type int
   *
   * Create vector/list that (after sorting by key, ascending) looks like:
   *  +-----------+-----+-----+-----------+-----+-----------+----------+
   *  | record_id | key | ... | record_id | key | dummy_rec | dummykey |
   *  +-----------+-----+-----+-----------+-----+-----------+----------+
   */

  // Insert record-key pair. Last RecordId is use for PageID to sibling
  node.clear();
  int i = 0;
  int limit = PageFile::PAGE_SIZE/sizeof(int); 
  do { // do-while b/c need to create at least the pair of dummies ONLY
    if (is_old_node && buffer[i] == INT_MIN) // if node is new
      break;
    RecordId record_id;
    record_id.pid = buffer[i] == INT_MIN ? -1 : buffer[i];
    // do not run over end of buffer
    record_id.sid = i+1 >= limit || buffer[i+1] == INT_MIN ? -1 : buffer[i+1];
    // do not run over end of buffer
    int key = i+2 >= limit || buffer[i+2] == INT_MIN ? INT_MAX : buffer[i+2];
    node.insert(pair<int, RecordId>(key, record_id));
    //XXX: Assume RecordId cannot be 0. Assume Key cannot be 0.
    i += 3;
  } while (buffer[i] != INT_MIN && i < limit);

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
{ //XXX: note that this has not been tested
  geneprintf("in BTLeafNode::write(pid==%d, pf)\n", pid);
  RC rc = 0;
  // Zero out buffer first
  int buffer[PageFile::PAGE_SIZE/sizeof(int)];
  for (int z = 0; z < PageFile::PAGE_SIZE/sizeof(int); ++z)
    buffer[z] = INT_MIN;

  /* Map from begin() to end() looks like:
   *  +-----------+-----+-----+-----------+-----+-----------+----------+
   *  | record_id | key | ... | record_id | key | dummy_rec | dummykey |
   *  +-----------+-----+-----+-----------+-----+-----------+----------+
   *
   * Buffer to be written should look like this:
   *  +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
   *  | pid | sid | key | pid | sid | key | ... | pid | sid | key | pageid |
   *  +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--------+
   * Assume pageid, sid and key all of type int
   */

  int i = 0;
  int limit = PageFile::PAGE_SIZE/sizeof(int); 
  //map<int, RecordId>::iterator itr_limit = --node.end();
  // Unpack record_id (This is C++ not C, so cannot be sure variables in struct
  // are represented in the same order in machine code)
  for (map<int, RecordId>::iterator itr = node.begin();
       i < limit && itr != node.end(); i+=3, ++itr) {
    buffer[i] = (itr->second).pid; // PageId
    // Ignore dummy sid but not associated pageid (copied above)
    //if (i+1 < limit && itr != itr_limit)
      buffer[i+1] = (itr->second).sid; // slot id
    // Ignore dummy key but not associated pageid (copied above)
    //if (i+2 < limit && itr != itr_limit)
      buffer[i+2] = itr->first;
  }
  geneprintf("i/3 == %d\n",i/3);
//buffer[0] = 1;
//buffer[1] = 0;
#ifdef debug
for (int z = 0; z < PageFile::PAGE_SIZE/sizeof(int); ++z) {
  geneprintf("%x ",buffer[z]);
}
geneprintf("\n");
#endif

  // Write page from buffer into disk
  if ((rc = pf.write(pid, buffer)) < 0) {
    geneprintf("BTLeafNode::write()\tCannot write page %d from into buffer disk\trc==%d", pid, rc);
    return rc;
  }


  geneprintf("BTLeafNode::write() returns\tleafnode written\n\n");

  return rc;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
  return node.size()-1; 
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
  geneprintf("BTLeafNode::insert(key==%d, rid)\tgetKeyCount()==%d\tBTLEAF_N-1==%d\n",key,getKeyCount(),BTLEAF_N-1); 
  if (getKeyCount() >= BTLEAF_N - 1) {
    return RC_NODE_FULL;
  }
  if (!((node.insert(pair<int, RecordId>(key, rid))).second)) {
    geneprintf("BTLeafNode::insert(key==%d, rid)\tIGNORED DUPLICATE\n",key);
  } else
    geneprintf("BTLeafNode::insert(key==%d, rid)\tInserted\n",key);
  map<int, RecordId>::iterator itr = node.begin();
  for (; itr != node.end(); ++itr)
    geneprintf("1..%d 2..%d 3..%d\n", itr->second.pid, itr->second.sid, itr->first);
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
	node.insert(pair<int, RecordId>(key, rid));
	map<int, RecordId>::iterator tempIt,it = node.begin();
	//move the iterator to the halfway point
	for(int i = 1; i < getKeyCount()/2+1; i++)
		it++;
	siblingKey = it->first;
	//move values starting from halfway, from old node to new node
	for(int i = 0; i < node.size()-1; i++) {
		int tempKey = it->first;
		RecordId tempRid = it->second;
		sibling.insert(tempKey, tempRid);	//insert values into new node
		tempIt = it;
		it++;
		node.erase(tempIt);	//delete values from old node
	}
/*dealing with this in BTreeIndex //set the current next node ptr to the sibling and set siblings next node ptr to whatever this was pointing at
	PageId tempNextPtr = getNextNodePtr();
	//this next line might be slightly confusing but all it is is getting the pid? check if this line is right? or else how are we pointing?
	setNextNodePtr(sibling.node_pid);
	sibling.setNextNodePtr(tempNextPtr);*/
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
  map<int, RecordId>::iterator it;
  eid = 0;
  for (it = node.begin(); it != node.end(); ++it, ++eid) { // eid is 0-indexed
    if (it->first >= searchKey)
      break;
  }
  if (it == node.end() || eid == getKeyCount()) // Don't hand out eids to dummy
    return RC_NO_SUCH_RECORD;
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
  if(eid == getKeyCount()) { gene2printf("\nreadEntry is trying to read dummy!\n\n");}
  map<int, RecordId>::iterator it = node.begin();
  for(int i = 0; i < eid; i++) {
    if(it == node.end())
      return RC_INVALID_ATTRIBUTE;
    it++;
  }
  key = it->first;
  rid = it->second;
  return 0;
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ return (--node.end())->second.pid; }

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	if(pid < 0)
		return RC_INVALID_PID;
	(--node.end())->second.pid = pid;
	return 0;
}

/*
BTNonLeafNode::BTNonLeafNode()
{
  node.clear();
}
*/

/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 *
 * Gene: read() is also used to initialise a BTNonLeafNode
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ //XXX: note that this is untested
  geneprintf("in BTNonLeafNode::read(pid==%d, pf)\n", pid);
  RC rc = 0;
  // Zero out buffer first
  int buffer[PageFile::PAGE_SIZE/sizeof(int)];
  for (int z = 0; z < PageFile::PAGE_SIZE/sizeof(int); ++z)
    buffer[z] = INT_MAX;
  // Read page from disk into buffer
  if ((rc = pf.read(pid, buffer)) < 0 && rc == RC_INVALID_PID) {
    geneprintf("BTNonLeafNode::read() Cannot read page %d from disk into buffer. rc==%d\n  => Assuming is new node\n", pid, rc);
  } else if (rc < 0) { return rc; }
  node_pid = pid;

  /* Nodes stored in page look like:
   *  +--------+-----+--------+-----+-----+--------+-----+--------+
   *  | pageid | key | pageid | key | ... | pageid | key | pageid |
   *  +--------+-----+--------+-----+-----+--------+-----+--------+
   * Assume pageid and key both of type int
   *
   * Create vector/list that (after sorting by key, ascending) looks like:
   *  +----------+--------+-----+--------+-----+--------+-----+-----+--------+
   *  | dummykey | pageid | key | pageid | key | pageid | ... | key | pageid |
   *  +----------+--------+-----+--------+-----+--------+-----+-----+--------+
   */

  // Insert into tree key-PageId pairs. Last PageId has dummy key associated
  node.clear();
  int i = 0;
  int limit = PageFile::PAGE_SIZE/sizeof(int); 
  do { // do-while b/c need to create at least one dummy key
    PageId pid = buffer[i] == INT_MAX ? -1 : buffer[i];
    // do not run over end of buffer
    int key = i+1 >= limit || buffer[i+1] == INT_MAX ? INT_MIN : buffer[i+1];
    node.insert(pair<int, PageId>(key, pid));
    //XXX: Assume RecordId cannot be 0. Assume Key cannot be 0.
    i += 2;
  } while (buffer[i] != INT_MAX && i < limit);

  geneprintf("BTNonLeafNode::read() returns\tnonleafnode \"initialised\"\n");

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
{ //XXX: note that this has not been tested
  RC rc = 0;
  // Zero out buffer first
  int buffer[PageFile::PAGE_SIZE/sizeof(int)];
  for (int z = 0; z < PageFile::PAGE_SIZE/sizeof(int); ++z)
    buffer[z] = INT_MAX;

  /* Map from begin() to end() looks like:
   *  +--------+----------+--------+-----+--------+-----+-----+--------------+
   *  | pageid | dummykey | pageid | key | pageid | key | ... | pageid | key | 
   *  +--------+----------+--------+-----+--------+-----+-----+--------------+
   *
   * Buffer to be written should look like this:
   *  +--------+-----+--------+-----+-----+--------+-----+--------+
   *  | pageid | key | pageid | key | ... | pageid | key | pageid |
   *  +--------+-----+--------+-----+-----+--------+-----+--------+
   * Assume pageid and key both of type int
   */

  // Turn page into list of pageid-key pairs
  int i = 0;
  int limit = PageFile::PAGE_SIZE/sizeof(int); 
  //map<int, PageId>::iterator itr_limit = --node.end();
  for (map<int, PageId>::iterator itr = node.begin();
       i < limit && itr != node.end(); i+=2, ++itr) {
    buffer[i] = itr->second;
    //donot Ignore dummy key but not associated pageid
    //if (i+1 < limit && itr != itr_limit)
    buffer[i+1] = itr->first;
  }

  // Write page from buffer into disk
  if ((rc = pf.write(pid, buffer)) < 0) {
    geneprintf("BTNonLeafNode::write()\tCannot write page %d from buffer into disk. rc==%d\n", pid, rc);
    return rc;
  }
  
  geneprintf("BTNonLeafNode::write()\treturns\tnonleafnode written\n");

  return rc;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
  return node.size()-1; // -1 for dummy key
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
  if (getKeyCount() >= BTNONLEAF_N - 1)
    return RC_NODE_FULL;
  node.insert(pair<int, PageId>(key, pid));
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
  geneprintf("in BTNonLeafNode::insertAndSplit(%d, %d, sibling, midKey)\n", key, pid); 
  node.insert(pair<int, PageId>(key, pid));
  map<int, PageId>::iterator half_point, itr = node.begin();
  // Start at half_point of this->node
  for (int i = getKeyCount() / 2; i >= 0; --i) // ugly
    ++itr;
  half_point = itr; // for later to remove right half of this->node
  ++itr; // Don't copy middle key. It is MOVED to parent for nonleaf overflows

  // Need to initialise sibling's dummy pair
  sibling.initializeRoot(half_point->second, itr->first, itr->second);
  ++itr;

  // Copy to sibling nodes from half_point+1 to end
  for (; itr != node.end(); itr++)
    sibling.insert(itr->first, itr->second);

  // Remove copied nodes and middle key from this->node
  node.erase(half_point, node.end());

  midKey = half_point->first;
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
  map<int, PageId>::iterator itr;
  itr = node.upper_bound(searchKey);
  /*
   * if searchKey==5, itr will point to 7, decrement to get 4's pointer.
   * if searchKey==7, itr will point to end(), decrement to get 7's pointer.
   *
   * dummy|  3|  4|  7|
   *      V   V   V   V
   */
  itr--; // decrement b/c pointers are right of keys
  pid = itr->second;
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
  node[INT_MIN] = pid1; // Dummy is always there. No need to check size
  return insert(key, pid2);
}

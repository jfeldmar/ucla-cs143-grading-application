#include "BTreeNode.h"
#include <cstdio>
#include <iostream>

using namespace std;

/*
 * Default constructor for BTLeafNode
 */
BTLeafNode::BTLeafNode(){
  d.info = LEAF;
  d.next = RC_END_OF_TREE;
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
  RC rc;
  rc = pf.read(pid,(void*)(&d)); 
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
  RC rc;
  rc = pf.write(pid,(void*)(&d)); 
  return rc;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
  return NODE_COUNT(d.info);
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
  if(NODE_COUNT(d.info) >= LEAF_PTRS)
    return RC_NODE_FULL;
  
  lrec rec;
  rec.key = key;
  rec.rid = rid;
  int index = -1;
  index = bsearch(d.entries,0,NODE_COUNT(d.info)-1,rec);
  if(index < 0){
    if(NODE_COUNT(d.info) != 0)
      return RC_INVALID_ATTRIBUTE;
    else
      index = 0;
  }
  else if(index == NODE_COUNT(d.info)-1
	  && d.entries[index] < rec)
    index = NODE_COUNT(d.info);

  lrec aux1,aux2;
  aux1 = rec;
  for(int i = index; i < NODE_COUNT(d.info)+1; i++){
    aux2 = d.entries[i];
    d.entries[i] = aux1;
    aux1 = aux2;
  }
  d.info = INCREMENT(d.info);
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
  if(NODE_COUNT(sibling.d.info) != 0)
    return RC_INVALID_ATTRIBUTE;
  
  int middle = (NODE_COUNT(d.info)) / 2;

  if(key > d.entries[middle].key)
    middle++;

  int count = 0;

  for(int i = middle, j = 0; i < NODE_COUNT(d.info); i++, j++){
    sibling.d.entries[j] = d.entries[i];
    count++;
    sibling.d.info = (sibling.d.info & FLAGS) | count;
  }
  d.info = ADD_COUNT(d.info,-1*count);

  if(key > d.entries[NODE_COUNT(d.info)-1].key)
    sibling.insert(key,rid);
  else
    insert(key,rid);

  sibling.d.next = d.next;
  siblingKey = sibling.d.entries[0].key;
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
  eid = -1;
  lrec r;
  r.key = searchKey;
  eid = bsearch(d.entries,0,NODE_COUNT(d.info)-1,r);

  if(eid == -1)
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
  if(eid >= NODE_COUNT(d.info) || eid < 0)
    return RC_INVALID_CURSOR;
  
  key = d.entries[eid].key;
  rid = d.entries[eid].rid;

  return 0;
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
  return d.next;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
  return d.next = pid;
}

RC BTLeafNode::printData(int mode)
{
  cout << "Keys:\n";
  for(int i = 0; i < NODE_COUNT(d.info); i++){
    int key = d.entries[i].key;
    cout << ((i > 0) ? ", " : "")  << key;
  }
  if(NODE_COUNT(d.info) == 0)
    cout << "Leaf is empty";
  cout << "\n";
  return 0;
}

/*
 * Default constructor for BTNonLeafNode
 */
BTNonLeafNode::BTNonLeafNode(){
  d.info = NONLEAF;
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
  RC rc;
  rc = pf.read(pid,(void*)(&d)); 
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
  RC rc;
  rc = pf.write(pid,(void*)(&d)); 
  return rc;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
  return NODE_COUNT(d.info); 
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 
  if(NODE_COUNT(d.info) >= NONLEAF_PTRS)
     return RC_NODE_FULL;
  
  nlrec rec;
  rec.key = key;
  rec.pid = pid;
  int index = -1;
  index = bsearch(d.entries,0,NODE_COUNT(d.info)-1,rec);
  if(index < 0)
    return RC_INVALID_ATTRIBUTE;
  else if(index == NODE_COUNT(d.info)-1
	  && d.entries[index] < rec)
    index = NODE_COUNT(d.info);

  nlrec aux1,aux2;
  aux1 = rec;
  for(int i = index; i < NODE_COUNT(d.info)+1; i++){
    aux2 = d.entries[i];
    d.entries[i] = aux1;
    aux1 = aux2;
  }
  d.info = INCREMENT(d.info);
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
  if(NODE_COUNT(sibling.d.info) != 0)
    return RC_INVALID_ATTRIBUTE;
  
  int middle = (NODE_COUNT(d.info)+1) / 2;

  if(key > d.entries[middle].key)
    middle++;

  int count = 0;

  sibling.d.left = d.entries[middle].pid;
  for(int i = middle, j = 0; i < NODE_COUNT(d.info); i++, j++){
    sibling.d.entries[j] = d.entries[i];
    count++;
  }
  sibling.d.info = ADD_COUNT(sibling.d.info,count);
  d.info = ADD_COUNT(d.info,-1*count);

  if(key > d.entries[NODE_COUNT(d.info)].key)
    sibling.insert(key,pid);
  else
    insert(key,pid);

  midKey = d.entries[NODE_COUNT(d.info)-1].key;
  sibling.d.left = d.entries[NODE_COUNT(d.info)-1].pid;
  d.info = DECREMENT(d.info);
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
  pid = -1;
  nlrec r;
  r.key = searchKey;
  pid = bsearch(d.entries,0,NODE_COUNT(d.info)-1,r);
  if(pid == -1)
    return RC_NO_SUCH_RECORD;
  
  if(pid == 0 & searchKey < d.entries[pid].key)
    pid = d.left;
  else{
    if(searchKey < d.entries[pid].key)
      pid = d.entries[pid-1].pid;
    else 
      pid = d.entries[pid].pid;
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
  if(NODE_COUNT(d.info) != 0)
    return RC_INVALID_ATTRIBUTE;

  d.info |= ROOT;
  d.entries[0].pid = pid2;
  d.entries[0].key = key;
  d.left = pid1;

  d.info = INCREMENT(d.info);
  
  return 0;
}

/*
 * Insert the height of the tree into the root node
 * @param height[IN] the height to be stored into the root
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertHeight(int height){
  if(height & HEIGHT != 0
     || d.info & ROOT == 0)
    return RC_INVALID_ATTRIBUTE;
  d.info = (height << 16) | d.info;
  return 0;
}

/*
 * Read the height stored int he root of the tree
 * @return height if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::getHeight(){
  if(d.info & ROOT == 0)
    return RC_INVALID_ATTRIBUTE;
  return (d.info & HEIGHT) >> 16;
}

RC BTNonLeafNode::printData(int mode){
  cout << "Node Data:\n";
  cout << "Keys:\n";
  for(int i = 0; i < NODE_COUNT(d.info); i++){
    int key = d.entries[i].key;
    cout << ((i > 0) ? ", " : "")  << key;
  }
  if(NODE_COUNT(d.info) == 0)
    cout << "Node is empty";
  cout << "\n";
  return 0;
}

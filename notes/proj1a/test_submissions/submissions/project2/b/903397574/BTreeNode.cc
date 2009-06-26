#include "BTreeNode.h"
#include <cstdio>
#include <iostream>

using namespace std;

/*
 * Default constructor for BTLeafNode
 */
BTLeafNode::BTLeafNode(){
  memset((void*)(&data),-1,sizeof(data));
  data.fc = 0;
  data.fc = 0 | BTREE_LEAF;
  data.next = RC_END_OF_TREE;
  printf("Leaf sizeof(data) = %d & N_LEAF_PAIRS = %d\n",sizeof(data),N_LEAF_PAIRS);
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
  return pf.read(pid,(void*)(&data)); 
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
  return pf.write(pid,(void*)(&data)); 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
  return KV_COUNT;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
  if(KV_COUNT >= N_LEAF_PAIRS)
     return RC_NODE_FULL;
  
  bool found = false;
  leaf_value temp;
  for(int i = 0; i <= KV_COUNT; i++){
    //cout << "(i,v) = (" << i << ","<< data.kv_pairs[i].key <<"), ";
    if(!found && 
       (data.kv_pairs[i].key >= key
	|| i == KV_COUNT )){
      temp = data.kv_pairs[i];
      data.kv_pairs[i].key = key;
      data.kv_pairs[i].rid = rid;
      found = true;
    }
    else if(found){
      leaf_value aux = data.kv_pairs[i];
      data.kv_pairs[i] = temp;
      temp = aux;
    }
  }
  //cout << "\n";
  int count = (KV_COUNT + 1);
  data.fc = BT_FLAGS | count;
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
  if((sibling.data.fc & COUNT_MASK) != 0)
    return RC_INVALID_ATTRIBUTE;
  
  int middle = (KV_COUNT) / 2;
  int count = 0;

  for(int i = middle+1, j = 0; i < KV_COUNT; i++, j++){
    //printf("(%d,%d,%d)\n",i,j,data.kv_pairs[i]);
    sibling.data.kv_pairs[j] = data.kv_pairs[i];
    count++;
    sibling.data.fc = (sibling.data.fc & FLAG_MASK) | count;
    memset((void*)(&data.kv_pairs[i]),-1,sizeof(leaf_value));
  }
  count = KV_COUNT - count;
  data.fc = BT_FLAGS | count;

  if(key > data.kv_pairs[KV_COUNT-1].key)
    sibling.insert(key,rid);
  else
    insert(key,rid);

  sibling.data.next = data.next;
  siblingKey = sibling.data.kv_pairs[0].key;
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
  for(int i = 0; i < KV_COUNT; i++){
    if(data.kv_pairs[i].key >= searchKey){
      eid = i;
      break;
    }
  }
  if(eid = -1)
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
  if(eid >= KV_COUNT || eid < 0)
    return RC_INVALID_CURSOR;
  
  key = data.kv_pairs[eid].key;
  rid = data.kv_pairs[eid].rid;

  return 0;
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
  return data.next;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
  return data.next;
}

/*
 * Print the keys of the node to stdout
 */
RC BTLeafNode::printKeys()
{
  cout << "Keys:\n";
  for(int i = 0; i < KV_COUNT; i++){
    int key = data.kv_pairs[i].key;
    cout << ((i > 0) ? ", " : "")  << key;
  }
  if(KV_COUNT == 0)
    cout << "Leaf is empty";
  cout << "\n";
  return 0;
}

/*
 * Default constructor for BTNonLeafNode
 */
BTNonLeafNode::BTNonLeafNode(){
  memset((void*)(&data),-1,sizeof(data));
  data.fc = 0;
  data.fc = 0 | BTREE_NONL; 
  printf("NonLeaf sizeof(data) = %d\n",sizeof(data));
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
  return pf.read(pid,(void*)(&data)); 
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
  return pf.write(pid,(void*)(&data)); 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
  return KV_COUNT; 
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 
  if(KV_COUNT >= N_NONLEAF_PAIRS)
     return RC_NODE_FULL;
  
  bool found = false;
  int temp_k = 0;
  int temp_v = 0;
  //cout << "max = " << NONLEAF_VAL_COUNT << "\n";
  for(int i = 1; i <= NONLEAF_VAL_COUNT; i += 2){
    //cout << "(i,v) = (" << i << ","<< data.kv_pairs[i] <<"), ";
    if(!found && 
       (data.kv_pairs[i] > key || i == NONLEAF_VAL_COUNT)){
      //cout << "\nInsertion point (i,v) = (" << i << ","<< data.kv_pairs[i] <<")\n";
      temp_k = data.kv_pairs[i];
      temp_v = data.kv_pairs[i+1];
      data.kv_pairs[i] = key;
      data.kv_pairs[i+1] = pid;
      found = true;
    }
    else if(found){
      int next_k = data.kv_pairs[i];
      int next_v = data.kv_pairs[i+1];
      data.kv_pairs[i] = temp_k;
      data.kv_pairs[i+1] = temp_v;
      temp_k = next_k;
      temp_v = next_v;      
    }
  }
  //cout << "\n";
  int count = (KV_COUNT + 1);
  data.fc = BT_FLAGS | count;
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
  if((sibling.data.fc & COUNT_MASK) != 0)
    return RC_INVALID_ATTRIBUTE;
  
  int middle = (KV_COUNT + 1) / 2;

  if(key > data.kv_pairs[middle*2+1])
    middle++;

  int count = 0;

  sibling.data.kv_pairs[0] = data.kv_pairs[middle*2];
  for(int i = middle*2+1, j = 1; i < NONLEAF_VAL_COUNT; i += 2, j += 2){
    //printf("(%d,%d,%d)\n",i,j,data.kv_pairs[i]);
    sibling.data.kv_pairs[j] = data.kv_pairs[i];
    sibling.data.kv_pairs[j+1] = data.kv_pairs[i+1];
    count++;
    sibling.data.fc = (sibling.data.fc & FLAG_MASK) | count;
    data.kv_pairs[i] = -1;
    data.kv_pairs[i-1] = -1;
  }
  count = KV_COUNT - count;
  data.fc = BT_FLAGS | count;

  if(key > data.kv_pairs[KV_COUNT*2-1])
    sibling.insert(key,pid);
  else
    insert(key,pid);

  midKey = data.kv_pairs[KV_COUNT*2-1];
  count = KV_COUNT - 1;
  data.fc = BT_FLAGS | count;
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
  for(int i = 1; i < KV_COUNT*2; i += 2){
    if(data.kv_pairs[i] >= searchKey){
      //cout << "searchKey found\n";
      pid = data.kv_pairs[i-1];
      break;
    }
  }
  if(pid == -1)
    return RC_NO_SUCH_RECORD;
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
  if(KV_COUNT != 0)
    return RC_INVALID_ATTRIBUTE;

  data.fc |= BTREE_ROOT;
  data.kv_pairs[0] = pid1;
  data.kv_pairs[1] = key;
  data.kv_pairs[2] = pid2;

  data.fc = BT_FLAGS | (KV_COUNT+1);
  
  return 0;
}

/*
 * Print the keys of the node to stdout
 */
RC BTNonLeafNode::printKeys()
{
  cout << "Keys:\n";
  for(int i = 1; i < KV_COUNT*2; i += 2){
    int key = data.kv_pairs[i];
    cout << ((i > 1) ? ", " : "")  << key;
  }
  if(KV_COUNT == 0)
    cout << "Node is empty";
  cout << "\n";
  return 0;
}

/*
 * Print the keys and values of the node to stdout
 */
RC BTNonLeafNode::printKV(){
  cout << "Node Data:\n";

  if(KV_COUNT == 0){
    cout << "Node is empty\n";
    return 0;
  }

  for(int i = 1; i < KV_COUNT*2; i += 2){
    printf("   %3d",data.kv_pairs[i]);
  }
  cout << "\n";
  for(int i = 0; i < NONLEAF_VAL_COUNT; i += 2){
    printf("%3d   ",data.kv_pairs[i]);
  }
  
  cout << "\n";
  return 0;
}

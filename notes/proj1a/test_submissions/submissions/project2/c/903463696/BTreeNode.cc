#include "BTreeNode.h"

using namespace std;

// Default Pair constructor.
Pair::Pair(){
  k = 0;
}

// Pair constructor with values.
Pair::Pair(RecordId recordid, int keys){
  r = recordid;
  k = keys;
}

// Default Leaf layout constructor.
Leaf::Leaf(){
  numKeys = 0;
  siblingNode = -1;
  nextNode = -1;  
//  pair = new Pair[84];
}

// Leaf constructor with values.
Leaf::Leaf(int num, PageId p){
  numKeys = num;
  siblingNode = p;
  nextNode = -1; 
//  pair = new Pair[84];
}

// Default destructor for Leaf.
Leaf::~Leaf(){
//  delete [] pair;
//  pair = NULL;
}

// Default leaf node constructor.
BTLeafNode::BTLeafNode(){  
  error = 0;
  leaf = new Leaf;
}

// Default BTreeNode destructor.
BTLeafNode::~BTLeafNode(){
  delete leaf;
  leaf = NULL; 
}

// Function to see if the node is full or not.
// Returns 0 if not, error if it is.
int BTLeafNode::isNodeFull()
{
  if(this->getKeyCount() == 84)
    return RC_NODE_FULL;
  else
    return 0;
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
  error = pf.read(pid, leaf);  
  this->leaf->setCurrNode(pid); 

  return error; 
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
  error = pf.write(pid, leaf);  
  this->leaf->setCurrNode(pid); 
//  this->pagefile = pf;
  return error;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
  return this->leaf->getnumKeys(); 
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
  int eid; 
  
  // Return error if node is full.
  if(this->isNodeFull() != 0)
    return RC_NODE_FULL;

  // Case where there exists a node larger than given.
  if(locate(key, eid) == 0){

    // Loop moving each pair up including the larger one up one.
    for(int i=83; i>eid; i--){
      this->leaf->pair[i] = this->leaf->pair[i-1];   
    }     

    // Setting the open value to the given pair.
    this->leaf->setPair(eid, rid, key);
    this->leaf->numKeys++;

    return 0;
  }  

  // Case where it is bigger than all in there, and node not full. 
  eid = this->leaf->getnumKeys();
  this->leaf->setPair(eid, rid, key);
  this->leaf->numKeys++;

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
  int eid;   

  // Default values to set to old pair.
  RecordId r;
  int keytemp = 0;
  PageId pid; 
  PageId temp; 
  PageId curr; 
  Pair ptemp; 

  // Copy last 24 entries into new node, empty each of the old Pairs.
  for(int i=42; i<84; i++){
    sibling.leaf->pair[i-42] = this->leaf->pair[i];
    this->leaf->setPair(i, r, keytemp); 
  }
  
  // Change number of keys in each to 42. 
  this->leaf->setnumKeys(42);
  sibling.leaf->setnumKeys(42);

  // Set PageId of sibling node to what the first was pointing to. 
  pid = this->leaf->getSiblingNode();
  temp = this->leaf->getNextNode(); 
  sibling.leaf->setSiblingNode(pid);
  sibling.leaf->setNextNode(temp); 

  // Set PageId of first node to the sibling node. 
  curr = sibling.leaf->getCurrNode(); 
  this->leaf->setSiblingNode(curr);
  this->leaf->setNextNode(pid); 

  // Find first key in sibling and set siblingKey. 
  siblingKey = sibling.leaf->pair[0].getKey();

  // Loops through and finds the first instance of a key >= searchKey.
  for (int i=83; i>=0; i--){
    ptemp = sibling.leaf->pair[i];
    if((key >= ptemp.k) && (ptemp.k != 0)){
      eid = i;
      break;
    }
  }
  
  // Belongs in second node, insert there, add one to numKeys. 
  if(eid > 0){
    error = sibling.insert(key, rid);
    return error;
  }  

  // Belongs in first node, so insert there, add one to numKeys. 
  error = this->insert(key, rid); 

  return error; 
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remember that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
  Pair temp;

  // If node is full, return error.
//  if (this->isNodeFull() == RC_NODE_FULL)
//    return RC_NODE_FULL;

  // Loops through and finds the first instance of a key >= searchKey.
  for (int i=0; i<84; i++){
    temp = this->leaf->pair[i];
    if(temp.k >= searchKey){
      eid = i;
      return 0;}
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
  this->leaf->pair[eid].getPair(rid, key);
  
  if ((rid.pid < 0) || (rid.sid < 0))
    error = RC_INVALID_RID;
  else
    error = 0;

  return error; 
}

/*
 * Output the pid of the next sibling node.
 * @return pid if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
  PageId node = this->leaf->getNextNode();

  if(node < -1)
    return RC_INVALID_PID; 

  return node;  
}

/*
 * Set the pid of the next sibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
  if(pid < -1)
    return RC_INVALID_PID; 

  this->leaf->setNextNode(pid); 

  return 0; 
}


/*
 *
 * STARTS NON-LEAF CLASS DEFINITIONS. 
 *
 */


// Default NonPair constructor.
NonPair::NonPair(){
  k = 0;
  p = -1;
}

// Default NonLeaf layout constructor.
NonLeaf::NonLeaf(){
  numKeys = 0;
  currNode = -1;
  parentNode = -1; 
  first = -1; 
//  nonpair = new NonPair[84];
}

// Default destructor for NonLeaf.
NonLeaf::~NonLeaf(){
//  delete [] nonpair;
//  nonpair = NULL;
}

// Default nonleaf node constructor.
BTNonLeafNode::BTNonLeafNode(){  
  error = 0;
  nonleaf = new NonLeaf;
}

// Default BTreeNode destructor.
BTNonLeafNode::~BTNonLeafNode(){
  delete nonleaf;
  nonleaf = NULL;  
}

// Function to see if the node is full or not.
// Returns 0 if not, error if it is.
int BTNonLeafNode::isNodeFull()
{
  if(this->getKeyCount() == 84)
    return RC_NODE_FULL;
  else
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
  error = pf.read(pid, nonleaf);  
  this->nonleaf->setCurrNode(pid);
  return error; 
}

/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ 
  error = pf.write(pid, nonleaf);  
  this->nonleaf->setCurrNode(pid); 
  return error;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
  return this->nonleaf->getnumKeys(); 
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{   
  int eid; 
  
  // Return error if node is full.
  if(this->isNodeFull() != 0)
    return RC_NODE_FULL;

  NonPair temp;

  // Loops through and finds the first instance of a key >= searchKey.
  for (int i=0; i<84; i++){
    temp = this->nonleaf->nonpair[i];
    if(temp.k >= key){
      eid = i;

      // Loop moving each pair up including the larger one up one.
      for(int j=83; j>eid; j--){
        this->nonleaf->nonpair[j] = this->nonleaf->nonpair[j-1];   
      }     

      // Setting the open value to the given pair.
      this->nonleaf->setNonPair(eid, key, pid);
      this->nonleaf->numKeys++;

      return 0;
    }
      
  }

  // Case where it is bigger than all in there, and node not full. 
  eid = this->nonleaf->getnumKeys();
  this->nonleaf->setNonPair(eid, key, pid);
  this->nonleaf->numKeys++;

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
  int eid = 0;   
 
  // Default values to set to old pair.
  int keytemp = 0;
  PageId middle;
  NonPair temp; 
  PageId p = 0;

  // Copy last 24 entries into new node, empty each of the old NonPairs.
  for(int i=42; i<84; i++){
    sibling.nonleaf->nonpair[i-42] = this->nonleaf->nonpair[i];
    this->nonleaf->setNonPair(i, keytemp, p); 
  }
  
  // Change number of keys in each to 42. 
  this->nonleaf->setnumKeys(42);
  sibling.nonleaf->setnumKeys(42);

  // Set first of sibling node to last pair of first.
  middle = this->nonleaf->nonpair[41].getPid();
  midKey = this->nonleaf->nonpair[41].getKey(); 
  sibling.nonleaf->setFirst(middle);     

  // Loops through and finds the first instance of a key >= searchKey.
  for (int i=83; i>=0; i--){
    temp = sibling.nonleaf->nonpair[i];
    if((key >= temp.k) && (temp.k != 0)){
      eid = i;
      break;
    }
  }
 
  // Belongs in second node, insert there, add one to numKeys. 
  if(eid > 0){
    error = sibling.insert(key, pid);
    return error;
  }  

  // Belongs in first node, so insert there, add one to numKeys. 
  error = this->insert(key, pid); 

  return error; 
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
  int num = this->nonleaf->getnumKeys() - 1; 
  int eid = -1; 
  NonPair temp; 

  // Loop to find the correct key. 
  for(int i=num; i>=0; i--){
    temp = this->nonleaf->nonpair[i];
    if((searchKey >= temp.k) && (temp.k != 0)){
      eid = i;
      break;
    }
  }

  // If there is one, set to the pid in the pair. 
  if (eid >= 0){
    pid = this->nonleaf->nonpair[eid].getPid(); 
    return 0;
  }

  // If there is not, must be the bottom one, so set to first.
  pid = this->nonleaf->getFirst(); 
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
  // Output error if invalid pids.
  if((pid1 < 0) || (pid2 < 0))
    return RC_INVALID_PID; 

  // Set first to the pid.
  this->nonleaf->setFirst(pid1);

  // Set pair to the key and pid2.
  this->nonleaf->setNonPair(0, key, pid2);

  // Set numKeys to one after insertion. 
  this->nonleaf->setnumKeys(1); 

  return 0;
}

//////////// END OF FILE ////////////

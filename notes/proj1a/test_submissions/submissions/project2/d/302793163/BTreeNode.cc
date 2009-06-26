#include <vector>
#include <iostream>
#include "BTreeNode.h"
//#include "PageFile.cc"

using namespace std;

void BTLeafNode::print() {
  cout << "keyCount is " << getKeyCount() << endl;
  for ( int i = 0; i < getKeyCount(); i++ ) {
    cout << "key" << i << " is: " << sKey[i] << " and ("
         << sRid[i].pid << ", " << sRid[i].sid << ")" << endl;
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
  void * nodeBuffer = new char[1024];
  BTLeafNode *  node;
  pf.read(pid, nodeBuffer);
  node = (BTLeafNode * ) nodeBuffer;

  int size = sRid.size();
  for ( int i = 0; i < size; i++ ) {
    sRid.pop_back();
  }
  size = sKey.size();
  for ( int i = 0; i < size; i++ ) {
    sKey.pop_back();
  }
  for ( int i = 0; i < (*node).sRid.size(); i++ ) {
    sRid.push_back( (*node).sRid.at(i) );
  }
  for ( int i = 0; i < (*node).sKey.size(); i++ ) {
    sKey.push_back( (*node).sKey.at(i) );
  }
  ppid = (*node).ppid;
  npid = (*node).npid;

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
  void * nodeBuffer = new char[1024];
  BTLeafNode * node;
  node = (BTLeafNode * ) nodeBuffer;

  for ( int i = 0; i < sRid.size(); i++ ) {
    (*node).sRid.push_back( sRid.at(i) );
  }
  for ( int i = 0; i < sKey.size(); i++ ) {
    (*node).sKey.push_back( sKey.at(i) );
  }
  (*node).ppid = ppid;
  (*node).npid = npid;
  
  pf.write(pid, node);

  return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
  return sKey.size();
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
  // find the position of the key
  int  pos = 0;
  locate(key, pos);

  //insert key and rid into vector
  if ( pos > sKey.size() - 1 ) {
    sKey.push_back(key);
    sRid.push_back(rid);
  } else {
    vector<int>::iterator keyIt;
    vector<RecordId>::iterator ridIt;
    keyIt = sKey.begin();
    sKey.insert( keyIt + pos, key );
    ridIt = sRid.begin();
    sRid.insert( ridIt + pos, rid );
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
  insert(key, rid);
  int midN = (sKey.size() + 1) / 2;
  int sibSize = sKey.size() - midN;

  vector<int>::iterator kit;
  vector<RecordId>::iterator rit;

  if ( sKey.size() > 1 ) {
    for ( int i = 0; i < sibSize; i++ ) {
      if ( sibling.getKeyCount() == 0 ) {
	sibling.sKey.push_back(sKey.back());
	sibling.sRid.push_back(sRid.back());
      } else {
	kit = sibling.sKey.begin();
	rit = sibling.sRid.begin();
	sibling.sKey.insert(kit, sKey.back());
	sibling.sRid.insert(rit, sRid.back());
      }
      sKey.pop_back();
      sRid.pop_back();
    }
    siblingKey = sibling.sKey.front();
  } else {
    return -1;
  }
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
  int pos;
  if ( sKey.size() != 0 && searchKey > sKey.at(sKey.size()-1) ) {
    eid = sKey.size();
  } else {
    for ( pos = 0; pos < sKey.size(); pos++ ) {
      eid = pos;
      if ( searchKey <= sKey[pos] ) {
	break;
      }
    }
  }
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
  key = sKey[eid];
  rid = sRid[eid];
  return 0; 
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
  return npid; 
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
  npid = pid;
  return 0; 
}

/*****************************  **************************/

/////////////notes: sPid is located 1 more space than sKey
void BTNonLeafNode::print() {
  cout << "keyCount is " << getKeyCount() << endl;
  if ( getKeyCount() != 0 ) {
    cout << "sPid[0] is " << sPid[0] << endl;
  }
  for ( int i = 0; i < getKeyCount(); i++ ) {
    cout << "key" << i << " is: " << sKey[i] << " and "
         << sPid[i+1] << endl;
  }
}
void BTNonLeafNode::locate(int searchKey, int& pos)
{
  for ( pos = 0; pos < sKey.size(); pos++ ) {
    if ( searchKey == sKey[pos] ) {
      break;
    } else if ( searchKey < sKey[pos] ) {
      pos--;
      break;
    }
  }
  if ( pos > 0 ) {
    pos--;
  }
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
  void * nodeBuffer = new char[1024];
  BTNonLeafNode *  node;
  pf.read(pid, nodeBuffer);
  node = (BTNonLeafNode * ) nodeBuffer;

  int size = sPid.size();
  for ( int i = 0; i < size; i++ ) {
    //
    cout << "Popping sPid[" <<i << "]:"<< sPid.back() << endl;
    //
    sPid.pop_back();
  }
  size = sKey.size();
  for ( int i = 0; i < size; i++ ) {
    sKey.pop_back();
  }
  for ( int i = 0; i < (*node).sPid.size(); i++ ) {
    //
    cout << "Pushing " <<  (*node).sPid[i] << endl;
    //
    sPid.push_back( (*node).sPid[i] );
  }
  for ( int i = 0; i < (*node).sKey.size(); i++ ) {
    sKey.push_back( (*node).sKey[i] );
  }
  ppid = (*node).ppid;

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
  void * nodeBuffer = new char[1024];
  BTNonLeafNode *  node;
  node = (BTNonLeafNode * ) nodeBuffer;

  for ( int i = 0; i < sPid.size(); i++ ) {
    (*node).sPid.push_back( sPid.at(i) );
  }
  for ( int i = 0; i < sKey.size(); i++ ) {
    (*node).sKey.push_back( sKey.at(i) );
  }
  (*node).ppid = ppid;

  pf.write(pid, node);
  
  return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
  return sKey.size();
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
  int pos = 0;
  locate(key, pos);
  pos++;

  vector<int>::iterator kit;
  vector<PageId>::iterator pit;

  if ( sKey.size() == 0 ) {
    sKey.push_back(key);
    if ( sPid.size() == 0 ) {
      sPid.push_back(0);
    } else {
      sPid.push_back(pid);
    }
  } else {
    kit = sKey.begin();
    sKey.insert(kit+pos, key);
    pit = sPid.begin();
    sPid.insert(pit+pos+1, pid);
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
  insert(key, pid);
  int midN = (sKey.size() + 1) / 2;
  int sibSize = sKey.size() - midN;
  vector<int>::iterator kit;
  vector<PageId>::iterator pit;

  if ( sKey.size() > 1 ) {
    for ( int i = 0; i < sibSize; i++ ) {
      if ( sibling.getKeyCount() == 0 ) {
	sibling.sKey.push_back(sKey.back());
	sibling.sPid.push_back(sPid.back()); 
      } else {
	kit = sibling.sKey.begin();
	sibling.sKey.insert(kit, sKey.back());
	pit = sibling.sPid.begin();
	sibling.sPid.insert(pit, sPid.back());
      }
      sKey.pop_back();
      sPid.pop_back();
    }
    pit = sibling.sPid.begin();
    sibling.sPid.insert(pit, sPid.back());
    sPid.pop_back();
    
    midKey = sKey.back();
    sKey.pop_back();
  } else {
    return -1;
  }
  
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
  int pos;
  locate(searchKey, pos);
  pid = sPid[pos+1];
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
  ppid = -9;
  sPid.push_back(pid1);
  sKey.push_back(key);
  sPid.push_back(pid2);
  return 0; 
}

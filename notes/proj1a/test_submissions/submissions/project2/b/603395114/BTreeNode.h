/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 5/28/2008
 */

#ifndef BTNODE_H
#define BTNODE_H

#include "RecordFile.h"
#include "PageFile.h"

#define BTN_KEYS  0x1
#define BTN_VALS  0x2
#define BTN_FLAGS 0x4
#define BTN_COUNT 0x8

#define NONLEAF_PTRS    (1020/8)
#define LEAF_PTRS       (1016/12)

#define LEAF        0x00001000
#define NONLEAF     0x00002000
#define ROOT        0x00004000

#define COUNT       0x7FF
#define FLAGS       ~COUNT
#define HEIGHT      0xFFFF0000
#define NODE_COUNT(x)  ((x) & COUNT)
#define NODE_FLAGS(x)  ((x) & FLAGS)
#define INCREMENT(x)   (NODE_FLAGS(x) | (NODE_COUNT(x)+1))
#define DECREMENT(x)   (NODE_FLAGS(x) | (NODE_COUNT(x)-1))
#define ADD_COUNT(x,y) (NODE_FLAGS(x) | (NODE_COUNT(x)+(y)))

typedef struct lrec {
  RecordId rid;
  int key;
} lrec;

typedef struct nlrec {
  PageId pid;
  int key;
} nlrec;

inline bool operator<(const lrec& a, const lrec& b){
  if(a.key < b.key)
    return true;
  if(a.key == b.key
     && a.rid.pid < b.rid.pid)
    return true;
  if(a.key == b.key 
     && a.rid.pid < b.rid.pid
     && a.rid.sid < b.rid.sid)
    return true;
  return false;
}

inline bool operator<(const nlrec& a, const nlrec& b){
  if(a.key < b.key)
    return true;
  if(a.key == b.key
     && a.pid < b.pid)
    return true;
  return false;
}

template<class T>
inline bool operator==(const T& a, const T& b){
  return a.key == b.key;
}

template<class T>
int bsearch(const T* a, int start, int end, const T& val){
  if(end < start)
    return -1;
  if(a[end] < val)
    return end;
  if(val < a[start])
    return start;
    
  int mid = (end+start)/2;
  if(a[mid] < val)
    return bsearch(a,mid+1,end,val);
  else if(val < a[mid])
    return bsearch(a,start,mid,val);
  else return mid;
}


/**
 * BTLeafNode: The class representing a B+tree leaf node.
 */
class BTLeafNode {
 private:
  struct node{
    int info;
    lrec entries[LEAF_PTRS];
    PageId next;
    char p[8];
  } d;
 public:
   /**
    * Default constructor for BTLeafNode
    */
    BTLeafNode();

   /**
    * Insert the (key, rid) pair to the node.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert
    * @param rid[IN] the RecordId to insert
    * @return 0 if successful. Return an error code if the node is full.
    */
    RC insert(int key, const RecordId& rid);

   /**
    * Insert the (key, rid) pair to the node
    * and split the node half and half with sibling.
    * The first key of the sibling node is returned in siblingKey.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert.
    * @param rid[IN] the RecordId to insert.
    * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
    * @param siblingKey[OUT] the first key in the sibling node after split.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC insertAndSplit(int key, const RecordId& rid, BTLeafNode& sibling, int& siblingKey);

   /**
    * Find the index entry whose key value is larger than or equal to searchKey
    * and output the eid (entry id) whose key value &gt;= searchKey.
    * Remember that keys inside a B+tree node are sorted.
    * @param searchKey[IN] the key to search for.
    * @param eid[OUT] the entry number that contains a key larger              
    *                 than or equalty to searchKey.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC locate(int searchKey, int& eid);

   /**
    * Read the (key, rid) pair from the eid entry.
    * @param eid[IN] the entry number to read the (key, rid) pair from
    * @param key[OUT] the key from the slot
    * @param rid[OUT] the RecordId from the slot
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC readEntry(int eid, int& key, RecordId& rid);

   /**
    * Return the pid of the next slibling node.
    * @return the PageId of the next sibling node 
    */
    PageId getNextNodePtr();


   /**
    * Set the next slibling node PageId.
    * @param pid[IN] the PageId of the next sibling node 
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC setNextNodePtr(PageId pid);

   /**
    * Return the number of keys stored in the node.
    * @return the number of keys in the node
    */
    int getKeyCount();
 
   /**
    * Read the content of the node from the page pid in the PageFile pf.
    * @param pid[IN] the PageId to read
    * @param pf[IN] PageFile to read from
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC read(PageId pid, const PageFile& pf);
    
   /**
    * Write the content of the node to the page pid in the PageFile pf.
    * @param pid[IN] the PageId to write to
    * @param pf[IN] PageFile to write to
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC write(PageId pid, PageFile& pf);

    RC printData(int mode = 0);
}; 


/**
 * BTNonLeafNode: The class representing a B+tree nonleaf node.
 */
class BTNonLeafNode {
 private:
  struct node{
    int info;
    nlrec entries[NONLEAF_PTRS];
    PageId left;
  } d;
 public:
   /**
    * Default constructor for BTNonLeafNode
    */
    BTNonLeafNode();
  
   /**
    * Insert a (key, pid) pair to the node.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert
    * @param pid[IN] the PageId to insert
    * @return 0 if successful. Return an error code if the node is full.
    */
    RC insert(int key, PageId pid);

   /**
    * Insert the (key, pid) pair to the node
    * and split the node half and half with sibling.
    * The sibling node MUST be empty when this function is called.
    * The middle key after the split is returned in midKey.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert
    * @param pid[IN] the PageId to insert
    * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
    * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey);

   /**
    * Given the searchKey, find the child-node pointer to follow and
    * output it in pid.
    * Remember that the keys inside a B+tree node are sorted.
    * @param searchKey[IN] the searchKey that is being looked up.
    * @param pid[OUT] the pointer to the child node to follow.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC locateChildPtr(int searchKey, PageId& pid);

   /**
    * Initialize the root node with (pid1, key, pid2).
    * @param pid1[IN] the first PageId to insert
    * @param key[IN] the key that should be inserted between the two PageIds
    * @param pid2[IN] the PageId to insert behind the key
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC initializeRoot(PageId pid1, int key, PageId pid2);

   /**
    * Insert the height of the tree into the root node
    * @param height[IN] the height to be stored into the root
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC insertHeight(int height);

   /**
    * Read the height stored int he root of the tree
    * @return height if successful. Return an error code if there is an error.
    */
    RC getHeight();

   /**
    * Return the number of keys stored in the node.
    * @return the number of keys in the node
    */
    int getKeyCount();

   /**
    * Read the content of the node from the page pid in the PageFile pf.
    * @param pid[IN] the PageId to read
    * @param pf[IN] PageFile to read from
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC read(PageId pid, const PageFile& pf);
    
   /**
    * Write the content of the node to the page pid in the PageFile pf.
    * @param pid[IN] the PageId to write to
    * @param pf[IN] PageFile to write to
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC write(PageId pid, PageFile& pf);

    RC printData(int mode = 0);
}; 

#endif /* BTNODE_H */

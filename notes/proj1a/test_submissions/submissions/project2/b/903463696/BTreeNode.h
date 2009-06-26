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

/**
 * Pair: The class representing the pairs of RecordIds and keys.
 */
class Pair {
  public:
    RecordId r; 
    int k;
    
    Pair();
    Pair(RecordId recordid, int keys);

    void setPair(RecordId rid, int key)
    {r = rid;
     k = key;}

    void setPairInd(PageId p, int s, int key)
    {r.pid = p;
     r.sid = s;
     k = key;}

    void getPair(RecordId& rid, int& key)
    {rid = r;
     key = k;}

    void setRid(PageId pid2, int sid2)
    {r.pid = pid2;
     r.sid = sid2;}

    void getRid(PageId& pid2, int& sid2)
    {pid2 = r.pid;
     sid2 = r.sid;}

    void setKey(int setkey) {k = setkey;}

    int getKey() {return k;}
}; 

/**
 * Leaf: The class representing the layout of a B+tree leaf node.
 */
class Leaf {
  public:
    int numKeys; 
    PageId currNode; 
    PageId siblingNode;
    PageId nextNode; 
    Pair *pair;
  
    Leaf();
    Leaf(int num, PageId p);
    ~Leaf();
    
    void setnumKeys(int num) {numKeys = num;}

    int getnumKeys() {return numKeys;}

    void setCurrNode(PageId setnode) {currNode = setnode;}

    PageId getCurrNode() {return currNode;}

    void setSiblingNode(PageId setnode) {siblingNode = setnode;}

    PageId getSiblingNode() {return siblingNode;}

    void setNextNode(PageId setnode) {nextNode = setnode;}

    PageId getNextNode() {return nextNode;};

    void setPair(int eid, RecordId rid, int key) {pair[eid].setPair(rid, key);}

    void setPairInd(int eid, PageId p, int s, int key) {pair[eid].setPairInd(p, s, key);}

    void getPair(int eid, Pair& p) {p = pair[eid];}
}; 

/**
 * BTLeafNode: The class representing a B+tree leaf node.
 */
class BTLeafNode {
  private:
    int error;

  public:
    Leaf *leaf;
    PageFile pagefile; 

    // Default constructor.
    BTLeafNode();
    ~BTLeafNode();

    // Function to see if the node is full or not according to what n is set to.
    // Returns 0 if not, error if it is.
    int isNodeFull();

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
}; 

/**
 * NonPair: The class representing the pairs of keys and PageIds.
 */
class NonPair {
  public: 
    int k;
    PageId p;
    
    NonPair();

    void setNonPair(int key, PageId pid)
    {k = key;
     p = pid;}

    void getNonPair(int& key, PageId& pid)
    {key = k;
     pid = p;}

    void setPid(PageId pid) {p = pid;}

    PageId getPid() {return p;}


    void setKey(int setkey) {k = setkey;}

    int getKey() {return k;}
}; 

/**
 * NonLeaf: The class representing the layout of a B+tree nonleaf node.
 */
class NonLeaf {
  public:
    int numKeys; 
    PageId first;
    NonPair *nonpair;
  
    NonLeaf();
    ~NonLeaf();
    
    void setnumKeys(int num) {numKeys = num;}

    int getnumKeys() {return numKeys;}

    void setFirst(PageId setnode) {first = setnode;}

    PageId getFirst() {return first;}

    void setNonPair(int eid, int key, PageId pid) {nonpair[eid].setNonPair(key, pid);}

    void setNonPair2(int eid, NonPair np) {nonpair[eid] = np;}

    void getNonPair(int eid, NonPair& np) {np = nonpair[eid];}
}; 

/**
 * BTNonLeafNode: The class representing a B+tree nonleaf node.
 */
class BTNonLeafNode {
  private:
    int error;

  public:
    NonLeaf *nonleaf;

    // Default constructor.
    BTNonLeafNode();
    ~BTNonLeafNode();


    // Function to see if the node is full or not.
    // Returns 0 if not, error if it is.
    int isNodeFull();
    
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
}; 

#endif /* BTNODE_H */

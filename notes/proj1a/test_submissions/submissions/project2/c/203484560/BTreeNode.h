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
 * BTLeafNode: The class representing a B+tree leaf node.
 */
class BTLeafNode {
private:
	//char buffer[PageFile::PAGE_SIZE];

	static const int MAX_POINTERS = (PageFile::PAGE_SIZE+4)/12;	//8*(n-1)+4+4*(n-1)+4*1 = PageFile::PAGE_SIZE
	static const int MAX_ELEMENTS = MAX_POINTERS - 1;
	int numKeys;	//tells us the next available location in our structure
	
	//Node structure
	RecordId tuplePointers[MAX_ELEMENTS];	//the last pointer is of a different type
	int searchKeys[MAX_ELEMENTS];
	PageId nextLeafNode;

  /**
    * Find the index of the entry in searchKeys that matches key.
	* If no such entry is found, return the first key larger than it.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to find
    * @param begin[IN] the eid of the start of the section we want to search
	* @param end[IN] the eid of the end of the section (inclusive) we want to search
    * @return the eid where the key can be found
    */
	int binarySearch(int key, int lower, int upper);

  /**
	* Shift all keys starting from eid to the right one space
	* NOTE: There must be space to move keys one space to the right.
	* @param eid[IN] the first key to move
	* @return 0 if success, error otherwise
	*/
	int shiftKeysRight(int eid);

  public:
   /**
    * Default constructor
    */
	BTLeafNode()
	{
		//numPointers = PageFile::PAGE_SIZE / 12;
		numKeys = 0;
		//nextLeafNode = NULL;
		nextLeafNode = RC_END_OF_TREE;
	}

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
    * and output the eid (entry id) whose key value >= searchKey.
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
 * Return the maximum number of keys able to be stored in the node.
 * @return the maximum number of keys for the node
 */
	int getMaxKeyCount();
 
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
 * BTNonLeafNode: The class representing a B+tree nonleaf node.
 */
class BTNonLeafNode {
private:
	static const int MAX_PID = (PageFile::PAGE_SIZE)/8;		
	static const int MAX_KEYS = MAX_PID - 1;					//There can be one more pid than key
	int numKeys;	//tells us the next available location in our structure
	
	//Node structure
	PageId pids[MAX_PID];	
	int keys[MAX_KEYS];

/**
 * Find the index of the first entry in keys[ ] larger than key.
 * @param key[IN] the key to find
 * @return the index in keys[ ] of the first entry larger than key, or if none exists, the number of elements filled in keys[ ]
 */
	int binarySearch(int key);
	
 /**
 * Shift all entries in keys[ ] and pids[ ] that are in indexes >= index to one element greater
 * @param index[IN] the index to start the shifting at
 * @return 0 if successful. Return an error code if the node is full.
 */
	RC shiftRight(int index);
	
 /**
 * Copy all entries in the NonLeafNode from the start index until the end into the sibling node.
 * The sibling node must be empty.
 * @param start[IN] the index to start the copying from
 * @return 0 if successful. Return an error code if sibling is not empty.
 */
RC copy(int start, BTNonLeafNode& sibling);
	
  public:
  
 /**
* Default constructor
*/
BTNonLeafNode()
{
	numKeys = 0;
}

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
 * Return the maximum number of keys able to be stored in the node.
 * @return the maximum number of keys for the node
 */
	int getMaxKeyCount();

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

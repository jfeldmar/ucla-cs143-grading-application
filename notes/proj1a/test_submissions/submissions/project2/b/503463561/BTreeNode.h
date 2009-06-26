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
 * Binary search helper function for use by insertion and search methods
 * @param base the array to search
 * @param item a temporary structure holding our key of interest
 * @param num the number of items to search; MUST be nonzero!
 * @param size the size of each item
 * @param comparator the function to use when making comparisons
 * @param result the variable to store the result of search, or
 * 		 index of the closest key with value less than the actual key
 * @return 0 if successful match, < 0 if the key is less than the index
 * 		  returned, and > 0 if the key is greater than the index returned.
 */

int binary_search_index( const void* base, const void* item, size_t num, 
	size_t size, int(*comparator)(const void*, const void*), int &result );

/**
 * Helper function to quickly insert an item in-place in a sorted array
 * and copy forward all other entries. PRECONDITION: sorted array has enough
 * room for one additional entry!
 * @param base the array to insert our value
 * @param item a temp structure holding the item of interest
 * @param num the number of items in the array NOTE: This is NOT updated by the
 * quick_insert() function; you have to do this YOURSELF!
 * @param size the size of each item
 * @param comparator the function to use when making comparisons
 * @return 0 if successful, error code on error
 */
int quick_insert( void* base, const void* item, size_t num,
	size_t size, int(*comparator)(const void*, const void*));

/**
 * The data structure for an entry in a leaf node.
 * Consists of rid (a RecordId struct) and an int representing the search key.
 */
typedef struct {
   RecordId rid;
   int      key;
   static int comparator(const void *e1, const void *e2);
} LeafNodeEnt;

int LeafNodeEnt::comparator(const void *e1, const void *e2)
{
   return ((LeafNodeEnt*)e1)->key - ((LeafNodeEnt*)e2)->key;
}

/**
 * BTLeafNode: The class representing a B+tree leaf node.
 */
class BTLeafNode {
  public:
   /// Maximum number of entries in a leaf node
   static const int N_KEYS = (PageFile::PAGE_SIZE - sizeof(PageId) - sizeof(int)) / sizeof(LeafNodeEnt);

   /**
    * Constructor
    * Initializes number of entries and next id to zero.
    */

   BTLeafNode() : m_keyCount(0), m_next(0) { }

   /**
    * Drops the current records stored in the leaf node.
    */
   void drop() { m_keyCount = 0; }

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
    * The first key of the sibling node is returned in midKey.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert.
    * @param rid[IN] the RecordId to insert.
    * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
    * @param midKey[OUT] the first key in the sibling node after split.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC insertAndSplit(int key, const RecordId& rid, BTLeafNode& sibling, int& midKey);

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

    /**
     * Print the contents of the node for debugging
     */
    void print();

    private:
      int m_keyCount; // Number of keys stored in node
      PageId m_next; // Pointer to next leaf node
      LeafNodeEnt m_ents[N_KEYS+1];  // Array of leaf node entries
												 // Contains an extra entry for use when
												 // splitting
}; 

/**
 * The data structure for an entry in a nonleaf node.
 * Consists of a PageId type and an int representing the search key.
 */

typedef struct {
   PageId pid;
   int key;
   static int comparator(const void *e1, const void *e2);
} NonLeafNodeEnt;

int NonLeafNodeEnt::comparator(const void *e1, const void *e2)
{
   return ((NonLeafNodeEnt*)e1)->key - ((NonLeafNodeEnt*)e2)->key;
}

/**
 * BTNonLeafNode: The class representing a B+tree nonleaf node.
 */
class BTNonLeafNode {
  public:
   /// Maximum number of entries in a nonleaf node
   static const int N_KEYS = (PageFile::PAGE_SIZE - sizeof(int) - sizeof(PageId)) / sizeof(NonLeafNodeEnt);

   /**
    * Constructor
    * Initializes m_keyCount and m_first to zero.
    */
   BTNonLeafNode() : m_keyCount(0), m_first(0) { }

	/**
	 * Drops the current records stored in the nonleaf node. Only really useful
	 * for debugging purposes.
	 */
   void drop() { m_keyCount = 0; }

	/// Get function for page id (used for debugging)
	const PageId getFirst() { return m_first; }

	/// Set function for page id (used for debugging)
	void setFirst(const PageId pid) { m_first = pid; }

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

    /**
     * Print the contents of the node for debugging
     */
    void print();

    private:
      int m_keyCount; // Number of keys stored in node
      PageId m_first; // First page id (not associated with any key)
      NonLeafNodeEnt m_ents[N_KEYS+1]; // Entries stored in node
													// Contains extra entry for use
													// with splitting
}; 

#endif /* BTNODE_H */


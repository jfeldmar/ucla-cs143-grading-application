/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#ifndef BTREEINDEX_H
#define BTREEINDEX_H

#include "Bruinbase.h"
#include "PageFile.h"
#include "RecordFile.h"
             
/**
 * The data structure to point to a particular entry at a b+tree leaf node.
 * An IndexCursor consists of pid (PageId of the leaf node) and 
 * eid (the location of the index entry inside the node).
 * IndexCursor is used for index lookup and traversal.
 */
typedef struct {
  // PageId of the index entry
  PageId  pid;  
  // The entry number inside the node
  int     eid;  
} IndexCursor;

class Stack {
  public:
  Stack( )
  {
	head = NULL;
  }
  ~Stack( )
  {
	clear();
  }

  void push(PageId pid)
  {
	stackNode* tmp = new stackNode;
	tmp->pid = pid;
	tmp->next = head;
	head = tmp;
	return;
  }
  void pop( )
  {
	if (head != NULL) {
		stackNode* tmp = head->next;
		delete head;
		head = tmp;
	}
	return;
  }
  PageId top( )
  {
	return head->pid;
  }
  int isEmpty( )
  {
	return head == NULL;
  }
  void clear( )
  {
	while(!isEmpty())
		pop();
  }
  
  private:
  struct stackNode
  {
  	PageId pid;
  	stackNode* next;
  };
  stackNode* head;
  
};

/**
 * Implements a B-Tree index for bruinbase.
 * 
 */
class BTreeIndex {
 public:
  BTreeIndex();
  
  void setDebug(int num);

  /**
   * Open the index file in read or write mode.
   * Under 'w' mode, the index file should be created if it does not exist.
   * @param indexname[IN] the name of the index file
   * @param mode[IN] 'r' for read, 'w' for write
   * @return error code. 0 if no error
   */
  RC open(const std::string& indexname, char mode);

  /**
   * Close the index file.
   * @return error code. 0 if no error
   */
  RC close();
    
  /**
   * Insert (key, RecordId) pair to the index.
   * @param key[IN] the key for the value inserted into the index
   * @param rid[IN] the RecordId for the record being inserted into the index
   * @return error code. 0 if no error
   */
  RC insert(int key, const RecordId& rid);

  /**
   * Find the leaf-node index entry whose key value is larger than or
   * equal to searchKey and output its location (i.e., the page id of the node
   * and the entry number in the node) as "IndexCursor."
   * IndexCursor consists of pid (page id of the node that contains the 
   * searchKey) and eid (the entry number inside the node)
   * to indicate the location of a particular index entry in the B+tree.
   * Note that, for range queries, we need to scan the B+tree leaf nodes.
   * For example, if the query is "key > 1000", we should scan the leaf
   * nodes starting with the key value 1000. For this reason,
   * this function returns the location of the leaf node entry
   * for a given searchKey, instead of returning the RecordId
   * associated with the searchKey.
   * Using the returned "IndexCursor", you will have to call readForward()
   * to retrieve the actual (key, rid) pair from the index.
   * @param key[IN] the key to find
   * @param cursor[OUT] the cursor pointing to the first index entry
   * with the key value
   * @return error code. 0 if no error.
   */
  RC locate(int searchKey, IndexCursor& cursor);
  RC locateWithRecord(int searchKey, IndexCursor& cursor);

  /**
   * Read the (key, rid) pair at the location specified by the index cursor,
   * and move foward the cursor to the next entry.
   * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
   * @param key[OUT] the key stored at the index cursor location
   * @param rid[OUT] the RecordId stored at the index cursor location
   * @return error code. 0 if no error
   */
  RC readForward(IndexCursor& cursor, int& key, RecordId& rid);
  
  RC insertInLeaf(PageId parentId, PageId leafNode, int key, const RecordId& rid);
  RC insertInParent(PageId parentId, PageId initialNode, int midKey, PageId sibling);
  
  void printNodes();
  
 private:
  PageFile pf;         /// the PageFile used to store the actual b+tree in disk

  PageId   rootPid;    /// the PageId of the root node
  int      treeHeight; /// the height of the tree
  /// Note that the content of the above two variables will be gone when
  /// this class is destructed. Make sure to store the values of the two 
  /// variables in disk, so that they can be reconstructed when the index
  /// is opened again later.
  bool m_debug;
  Stack m_visitedStack;
  // info about the tree
  static const int ROOT_LOC = 0;
  static const int HEIGHT_LOC = 4;
  
  // info about the nodes
  static const int ENTRY_SIZE = sizeof(int)+sizeof(RecordId);
  static const int MAX_ENTRIES = 80;
  static const int END_ENTRIES_LOC = ENTRY_SIZE*MAX_ENTRIES; // 12 * 80, denotes the end of the entries
  static const int PARENT_LOC = 1008; // parent location
  static const int TYPE_LOC = 1012; // denotes the location of the type of the node
  static const int NUMKEY_LOC = 1016; // denotes the location of the number of keys
  static const int NEXTNODE_LOC = 1020; // location of the pid of the nextnode
  static const int HEADER_SIZE = 16; // size of the metadata
  
};

#endif /* BTREEINDEX_H */

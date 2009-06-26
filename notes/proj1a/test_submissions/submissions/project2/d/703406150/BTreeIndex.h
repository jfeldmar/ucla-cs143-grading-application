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

#include <string>
#include "Bruinbase.h"
#include "PageFile.h"
#include "RecordFile.h"

using namespace std;

/**
 * Implements a B-Tree index for bruinbase.
 * 
 */
class BTreeIndex
{
 public:
  BTreeIndex();

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
   * @param pid[OUT] the pid of the first index entry >= the key value
   * @param eid[OUT] the eid of the first index entry >= the key value
   * @return error code. 0 if no error.
   */
  RC locate(int searchKey, int& pid, int& eid);
  
  // this function locates a key recursively
  RC locateRecursive(int searchKey, int& pid, int& eid);

  /**
   * Read the (key, rid) pair at the location specified by the index cursor,
   * and move foward the cursor to the next entry.
   * @param pid[IN] the pid of the first index entry >= the key value
   * @param eid[IN] the eid of the first page entry >= the key value
   * @param key[OUT] the key stored at the index cursor location
   * @param rid[OUT] the RecordId stored at the index cursor location
   * @return error code. 0 if no error
   */
  RC readForward(int& pid, int& eid, int& key, RecordId& rid);
  
  // parse the index name to remove the extension
  void parse(string &s);
  
 private:
  PageFile pf;         /// the PageFile used to store the actual b+tree in disk
  // the first page is used for bookkeeping as an integer array
  // the first int (0) of the first page(pid=0) is the pid of the root node
  // the last int(255) contains the entry number of the current node
  // ints 1-254 contain pids of nodes visited starting after the root

  PageId   rootPid;    /// the PageId of the root node
  int      treeHeight; /// the height of the tree
  /// Note that the content of the above two variables will be gone when
  /// this class is destructed. Make sure to store the values of the two 
  /// variables in disk, so that they can be reconstructed when the index
  /// is opened again later.
};

#endif /* BTREEINDEX_H */

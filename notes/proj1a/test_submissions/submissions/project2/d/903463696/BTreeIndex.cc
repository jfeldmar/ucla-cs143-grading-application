/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include <iostream>

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
  rootPid = -1;
  treeHeight = -1;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
  // Open the pagefile. 
  error = this->pf.open(indexname, mode);
  
  if (error != 0)
    return error; 

  // Read these values from the memory.  
  int values[1024] = {};

  error = pf.read(0, values); 
  if (error != 0){
    rootPid = -1; 
    treeHeight = -1; 
  }else{ 
    rootPid = values[0]; 
    treeHeight = values[1]; 
  }

  return error;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
  error = this->pf.close(); 

  return error;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
  BTLeafNode templeaf; 
  int buffer[1024] = {};

  // Tree is empty. Creates root as leaf node and initializes it.
  if(rootPid == -1){
    error = templeaf.insert(key, rid);

    // Error inserting, return error.
    if(error != 0)
      return error;
  
    // Set LeafNode member variables.
    templeaf.leaf->setCurrNode(1);
    templeaf.leaf->setSiblingNode(-1);
    templeaf.leaf->setNextNode(-1);

    // Set BTIndex member variables.
    rootPid = 1; 
    buffer[0] = rootPid; // Set rootPid to 1.
    treeHeight = 1; 
    buffer[1] = treeHeight; // Set treeHeight to 1.

    // Write new root node and variables to pagefiles.
    error = templeaf.write(1, pf);
    error = pf.write(0, buffer);
    return 0;
  }

  // Case where root node is leaf. When full, create root as nonleaf.
  if(treeHeight == 1){

    // Read the root node into the leaf again. 
    error = templeaf.read(1, pf);

    if(error != 0)
      return error; 

    // If not full, keep adding to root node. 
    if(!templeaf.isNodeFull()){
      error = templeaf.insert(key, rid); 
      error = templeaf.write(1, pf); 
      return error; 
    }


    // If node is full, we must split into 2 and add root node. 
    // Get portions of root node and transfer into new leaf nodes. 
    BTLeafNode ln_sibling;  
    int siblingKey; 
    error = templeaf.insertAndSplit(key, rid, ln_sibling, siblingKey);     
     
    BTNonLeafNode new_nonleaf; 
    error = new_nonleaf.initializeRoot(2, siblingKey, 3); 

    // Initialize pointers on all nodes.
    new_nonleaf.nonleaf->setParent(-1);

    templeaf.leaf->setSiblingNode(3); 
    templeaf.leaf->setNextNode(-1);

    ln_sibling.leaf->setSiblingNode(-1);
    ln_sibling.leaf->setNextNode(-1); 

    treeHeight = 2; 
    buffer[0] = rootPid; 
    buffer[1] = treeHeight; 

    // Write all three to memory. 
    error = pf.write(0, buffer);
    error = new_nonleaf.write(1, pf);
    error = templeaf.write(2, pf);
    error = ln_sibling.write(3, pf);  

    return error;  
  }

  // Any other case, follows pointers from root down to leaf nodes. 
  int tempkey; 
  BTNonLeafNode nontemp; 
  PageId follow;   

  // Start at root node. 
  error = nontemp.read(rootPid, pf); 

  // Follows down the tree until you reach a leaf node.
  int i=1; 
  while((i < treeHeight) && (error == 0)){
    error = nontemp.locateChildPtr(key, follow);
    error = nontemp.read(follow, pf);   
    i++; 
  }

  // Return error if there is one. 
  if (error != 0)
    return error; 

  // Analyze once you reach the correct leaf node. 
  error = templeaf.read(follow, pf);

  // Return error if there is one. 
  if (error != 0)
    return error; 

  // Case that leaf node is not full, simply insert into leafnode. 
  if (!templeaf.isNodeFull()){
    error = templeaf.insert(key, rid);  
    error = templeaf.write(follow, pf);
    return error; 
  }
  
  // Case that leaf node is full, need to insert and split node into 2. 
  BTLeafNode tempsibling; 
  int sibkey; 
  PageId sibcurr; 
  PageId currcurr = templeaf.leaf->getCurrNode(); 
  PageId noncurr = nontemp.nonleaf->getCurrNode();  

  if (templeaf.isNodeFull()){
    error = templeaf.insertAndSplit(key, rid, tempsibling, sibkey); 
    sibcurr = tempsibling.leaf->getCurrNode(); 
 
    // Write leaf nodes to memory. 
    PageId endpid = pf.endPid(); 
    error = tempsibling.write(endpid, pf); 
    error = templeaf.write(currcurr, pf); 

    // Inserts new leaf's smallest pair into nonleaf and is not full. 
    if (!nontemp.isNodeFull()){
      error = nontemp.insert(sibkey, sibcurr);
      error = nontemp.write(noncurr, pf);      
      return error;   
    }

    // Case that parent is full. Splits until parent that does not need split is found.
    if (nontemp.isNodeFull()){
      BTNonLeafNode tempar; 
      BTNonLeafNode nonsib; 
      BTNonLeafNode newroot; 
      int nonmid; 
      int par = nontemp.nonleaf->getParent(); 

      // New root node case. 
      if (par == -1){
        error = nontemp.insertAndSplit(sibkey, sibcurr, nonsib, nonmid);
        endpid = pf.endPid(); 

        // Initialize and write new root node to memory.
        error = newroot.initializeRoot(endpid, nonmid, (endpid+1)); 
        newroot.nonleaf->setParent(-1); 
        error = newroot.write(1, pf); 

        // Initialize and write the two nonleaf pointers. 
        nontemp.nonleaf->setParent(1); 
        nonsib.nonleaf->setParent(1);
        error = nontemp.write(endpid, pf); 
        error = nonsib.write((endpid+1), pf); 

        // Increase tree height and write to disk.
        rootPid = 1; 
        buffer[0] = rootPid;
        treeHeight = 3; 
        buffer[1] = treeHeight; 

      }
/*      // Case where we split until new parent.
      else{       
        error = tempar.read(par, pf);  
        while ((tempar.isNodeFull()) && (error == 0) && ()){}

        if (error != 0)
          return error; 

      }*/
    }
  }
  return error;
}

/*
 * Find the leaf-node index entry whose key value is larger than or 
 * equal to searchKey, and output the location of the entry in IndexCursor.
 * IndexCursor is a "pointer" to a B+tree leaf-node entry consisting of
 * the PageId of the node and the SlotID of the index entry.
 * Note that, for range queries, we need to scan the B+tree leaf nodes.
 * For example, if the query is "key > 1000", we should scan the leaf
 * nodes starting with the key value 1000. For this reason,
 * it is better to return the location of the leaf node entry 
 * for a given searchKey, instead of returning the RecordId
 * associated with the searchKey directly.
 * Once the location of the index entry is identified and returned 
 * from this function, you should call readForward() to retrieve the
 * actual (key, rid) pair from the index.
 * @param key[IN] the key to find.
 * @param cursor[OUT] the cursor pointing to the first index entry
 *                    with the key value.
 * @return error code. 0 if no error.
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
  PageId pageid = pf.endPid();
  BTLeafNode leaf2; 
  BTNonLeafNode nontemp; 
  int entryid = 0; 

  int key = 0; 
  RecordId rid; 
  // If tree is empty, return error.
  if(rootPid == -1)    
    return RC_END_OF_TREE; 

  // Follow root pid and trace tree from there. 
  // Read content from pagefile into the LeafNode class, then get pair.
  if(treeHeight == 1){
    error = leaf2.read(rootPid, pf);
  
    // Quit if there is an error reading root node. 
    if(error != 0)
      return error; 
    // Locate the searchKey in the root node. 
    error = leaf2.locate(searchKey, entryid);
    if (error != 0)
      return RC_NO_SUCH_RECORD;
    else{
      cursor.eid = entryid;
      cursor.pid = rootPid; 
      return error;
    }
  }


  // Start at root node. 
  error = nontemp.read(rootPid, pf); 

  // Follows down the tree until you reach a leaf node.
  int i=1; 
  PageId follow; 
  while((i < treeHeight) && (error == 0)){
    error = nontemp.locateChildPtr(searchKey, follow);
    error = nontemp.read(follow, pf);   
    i++; 
  }

  // Return error if there is one. 
  if (error != 0)
    return error; 

  // Analyze once you reach the correct leaf node. 
  error = leaf2.read(follow, pf);

  if(error != 0)
    return error; 

  int eidout; 

  // Read forward to return the correct key, rid pair. 
  error = leaf2.locate(searchKey, eidout);
  cursor.eid = eidout;  
  cursor.pid = follow; 

  return error;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
  // Declaration of variables. 
  BTLeafNode leafnode; 
  Pair pair; 
  int keyCount = 0;  

  // Return error if index cursor's pid or eid are invalid.
  if((cursor.pid < 0) || (cursor.eid < 0))
    return RC_INVALID_CURSOR;   

  // Read content from pagefile into the LeafNode class, then get pair.
  error = leafnode.read(cursor.pid, pf);

  // Read and return the key and rid from the pair. 
  leafnode.readEntry(cursor.eid, key, rid);

  // Move cursor forward.
  keyCount = leafnode.getKeyCount();

  if(cursor.eid == (keyCount - 1))  
    error = RC_NO_SUCH_RECORD; 
  else
    cursor.eid++;

 
  return error;
}

/////////////////////////////END OF FILE///////////////////////////////////

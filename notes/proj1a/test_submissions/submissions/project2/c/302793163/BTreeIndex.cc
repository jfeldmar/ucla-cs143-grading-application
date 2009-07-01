/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"
#include <string>

using namespace std;

struct InitInfo {
  PageId rootPid;
  int treeHeight;
  int maxKeyNum;
};

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
  setMaxKeyNum(80);
  //rootPid = -1;
  
  PageFile cPage;
  rootPid = cPage.endPid();
  treeHeight = 2;

  BTNonLeafNode rootNode;
  BTLeafNode firstLeaf;
  rootNode.sPid.push_back(rootPid+1);
  cPage.write( rootPid, &rootNode );
  cPage.write( rootPid + 1, &firstLeaf );
  writeMode = true;
}
BTreeIndex::BTreeIndex( int inMaxKeyNum ) {
  setMaxKeyNum( inMaxKeyNum );
  //rootPid = -1;

  PageFile cPage;
  rootPid = cPage.endPid();
  treeHeight = 2;

  BTNonLeafNode rootNode;
  BTLeafNode firstLeaf;
  rootNode.sPid.push_back(rootPid+1);
  cPage.write( rootPid, &rootNode );
  cPage.write( rootPid + 1, &firstLeaf );
  writeMode = true;
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
  idxName = indexname;

  if ( mode == 'w' ) {
    writeMode = true;
  } else if ( mode == 'r' ) {
    writeMode = false;
  } else {
    return -1006;
  }

  PageFile cPage;
  InitInfo treeInfo;
  cPage.read( startPid, &treeInfo );
  rootPid = treeInfo.rootPid;
  treeHeight = treeInfo.treeHeight;
  maxKeyNum = treeInfo.maxKeyNum;
  return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
  // need to write into memory: rootPid, treeHeight
  PageFile cPage;
  InitInfo treeInfo;
  treeInfo.rootPid = rootPid;
  treeInfo.treeHeight = treeHeight;
  treeInfo.maxKeyNum = maxKeyNum;
  cPage.write( startPid, &treeInfo );
  return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
  PageFile cPage;    //current PageFile
  PageId cPid;       //current pid
  PageId nPid;       //next pid
  int cHeight;       //current height
  BTNonLeafNode nonLeaf;
  BTNonLeafNode nonLeafSib;
  BTLeafNode leaf;
  BTLeafNode leafSib;
  int sibMidKey;

  //start from the root
  cPid = rootPid;
  cHeight = 1;
  while ( cHeight < treeHeight ) {  //it is nonLeafNode
    nonLeaf.read( cPid, cPage );
    nonLeaf.locateChildPtr( key, cPid );
    cHeight++;
  }

  //it is leafNode now;
  leaf.read( cPid, cPage);
  if ( leaf.getKeyCount() < getMaxKeyNum() ) {
    leaf.insert(key, rid);
  } else {
    leaf.insertAndSplit(key, rid, leafSib, sibMidKey);
    nPid = cPage.endPid();
    cPage.write( nPid, &leafSib);
    cPid = leaf.ppid;

    //go back to modify nonLeaf
    nonLeaf.read( cPid, cPage );
    while ( nonLeaf.getKeyCount() >= getMaxKeyNum() ) {
      int tKey = sibMidKey;
      nonLeaf.insertAndSplit( tKey, nPid, nonLeafSib, sibMidKey);
      nPid = cPage.endPid();
      cPage.write( nPid, &nonLeafSib);

      if ( nonLeaf.ppid < 0 ) {            // if it is already root node
	BTNonLeafNode newRoot;
	newRoot.ppid = -1;
	newRoot.sPid.push_back(cPid);
	cPid = cPage.endPid();
	rootPid = cPid;
	nonLeaf.ppid = rootPid;
	cPage.write( cPid, &newRoot);
	nonLeaf.read( cPid, cPage );
      } else {
	cPid = nonLeaf.ppid;
	nonLeaf.read( cPid, cPage );
      }
    }
    nonLeaf.insert( sibMidKey, nPid);
  }
  
  return 0;
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
  PageFile cPage;
  BTNonLeafNode nonLeaf;
  BTLeafNode leaf;
  int cHeight;       //current height

  //start from the root
  cursor.pid = rootPid;
  cHeight = 1;
  while ( cHeight < treeHeight ) {  //it is nonLeafNode
    nonLeaf.read( cursor.pid, cPage );
    nonLeaf.locateChildPtr( searchKey, cursor.pid );
    cHeight++;
  }
  //it is leafNode now;
  leaf.read( cursor.pid, cPage);
  leaf.locate( searchKey, cursor.eid );
  return 0;
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
  PageFile currentP;
  BTLeafNode leaf;
  leaf.read(cursor.pid, currentP);
  leaf.readEntry(cursor.eid, key, rid);
  if ( cursor.eid < maxKeyNum ) {
    cursor.eid = cursor.eid + 1;
  } else {
    cursor.pid = leaf.getNextNodePtr();
    cursor.eid = 0;
  }
  return 0;
}

void BTreeIndex::print(){
  PageFile cPage;
  BTNonLeafNode nonLeaf;
  BTLeafNode leaf;
  int cHeight;       //current height
  PageId cPid;
  int cEid;

  //start from the root
  cout << "The tree is:" << endl;
  cPid = rootPid;
  cHeight = 1;

  while ( cHeight < treeHeight - 1 ) {
    cPid = nonLeaf.sPid[0];
    nonLeaf.read( cPid, cPage );
    cHeight++;
  }
  cPid = nonLeaf.sPid[0];
  leaf.read( cPid, cPage );

  //it is leafNode now;
  cout << "leaf nodes:" << endl;

}
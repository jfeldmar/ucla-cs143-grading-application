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
#include "PageFile.h"
#include <iostream>
#include <string>

using namespace std;

struct InitInfo {
  PageId rootPid;
  int treeHeight;
  int maxKeyNum;
};

//PageId BTreeIndex::startPid = -1;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
  setMaxKeyNum(80);
  
  /*
  //rootPid = -1;
  PageFile cPage;
  rootPid = cPage.endPid();
  treeHeight = 2;
  BTNonLeafNode rootNode;
  BTLeafNode firstLeaf;
  BTLeafNode secondLeaf;
  rootNode.sPid.push_back(rootPid+1);
  rootNode.sPid.push_back(rootPid+2);
  rootNode.write( rootPid, pf );
  firstLeaf.setNextNodePtr(rootPid+2);
  firstLeaf.write( rootPid+1, pf );
  secondLeaf.setNextNodePtr(0);
  secondLeaf.write( rootPid + 2, pf );
  */
}
BTreeIndex::BTreeIndex( int inMaxKeyNum ) {
  setMaxKeyNum( inMaxKeyNum );

  /*
  //rootPid = -1;
  PageFile cPage;
  rootPid = cPage.endPid();
  treeHeight = 2;
  BTNonLeafNode rootNode;
  BTLeafNode firstLeaf;
  rootNode.sPid.push_back(rootPid+1);
  rootNode.write( rootPid, pf );
  firstLeaf.write( rootPid+1, pf );
  */
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
  RC rc;
  rc = pf.open( indexname, mode );

  if ( mode == 'w' || mode == 'W' ) {
    writeMode = true;
  } else if ( mode == 'r' || mode == 'R' ) {
    writeMode = false;
  } else {
    return -1006;
  }

  if ( pf.endPid() == 0 ) {
    //startPid = pf.endPid();
    void * voidPtr = new char[1024];
    pf.write(startPid, voidPtr);

    rootPid = pf.endPid();
    treeHeight = 2;
    BTNonLeafNode rootNode;
    BTLeafNode firstLeaf;
    BTLeafNode secondLeaf;
    rootNode.sPid.push_back(rootPid+1);
    rootNode.sPid.push_back(rootPid+2);
    rootNode.ppid = -1;
    rootNode.write( rootPid, pf );
    firstLeaf.setNextNodePtr(rootPid+2);
    firstLeaf.ppid = rootPid;
    firstLeaf.write( rootPid+1, pf );
    secondLeaf.setNextNodePtr(0);
    secondLeaf.ppid = rootPid;
    secondLeaf.write( rootPid + 2, pf );
  
    /*
    cout << "pf.endPid() at open is " << pf.endPid() << endl;
    cout << "rootPid is " << rootPid << endl;
    cout << "rootNode.sPid.size() is " << rootNode.sPid.size() << endl;
    cout << "rootNode.sPid[0] is " << rootNode.sPid[0] << endl;
    cout << "rootNode.sPid[1] is " << rootNode.sPid[1] << endl;
    BTNonLeafNode tnLeaf;
    tnLeaf.read(rootPid, pf);
    cout << "tnLeaf.sPid.size() is " << tnLeaf.sPid.size() << endl;
    cout << "tnLeaf.sPid[0] is " << tnLeaf.sPid[0] << endl;
    cout << "tnLeaf.sPid[1] is " << tnLeaf.sPid[1] << endl;
    cout << "tnLeaf.ppid is " << tnLeaf.ppid << endl;
    BTLeafNode tLeaf;
    tLeaf.read( tnLeaf.sPid[0], pf);
    cout << "ppid of first leaf is " << tLeaf.ppid << endl;
    tLeaf.read( tnLeaf.sPid[1], pf);
    cout << "ppid of second leaf is " << tLeaf.ppid << endl;
    cout << "^^^^^End of open test^^^^^^" << endl;
    */
  } else {
    void * voidPtr = new char[1024];
    InitInfo *treeInfo;
    pf.read( startPid, voidPtr );
    treeInfo = (InitInfo *) voidPtr;
    rootPid = (*treeInfo).rootPid;
    treeHeight = (*treeInfo).treeHeight;
    maxKeyNum = (*treeInfo).maxKeyNum;
    //
    cout << "Tree already existed" << endl;
    cout << "Reading treeInfo with rootPid = " << rootPid << endl;
    cout << "treeHeight = " << treeHeight << endl;
    cout << "maxKeyNum = " << maxKeyNum << endl;
    cout << "startPid = " << startPid << endl;
    //
  }


  return rc;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
  // need to write into memory: rootPid, treeHeight
  //PageFile cPage;
  InitInfo treeInfo;
  treeInfo.rootPid = rootPid;
  treeInfo.treeHeight = treeHeight;
  treeInfo.maxKeyNum = maxKeyNum;
  pf.write( startPid, &treeInfo );
  pf.close();
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
  //PageFile cPage;    //current PageFile
  PageId cPid;       //current pid
  PageId nPid;       //next pid
  int cHeight;       //current height
  BTNonLeafNode nonLeaf;
  BTNonLeafNode nonLeafSib;
  BTLeafNode leaf;
  BTLeafNode leafSib;
  int sibMidKey;

  //handle first insert
  nonLeaf.read(rootPid, pf);
  if ( nonLeaf.getKeyCount() == 0 ) {
    nonLeaf.sKey.push_back(key);
    nonLeaf.write( rootPid, pf);
  }
  
  //start from the root
  vector<PageId> treeTrace;

  cPid = rootPid;
  cHeight = 1;
  treeTrace.push_back(cPid);

  while ( cHeight < treeHeight ) {  //it is nonLeafNode
    nonLeaf.read( cPid, pf );
    nonLeaf.locateChildPtr( key, cPid );
    //
    cout << "Finding key " << key << " on nonLeaf ";
    printNonLeaf(nonLeaf);
    cout << "key is located on page " << cPid << endl;
    //
    treeTrace.push_back(cPid);
    cHeight++;
  }
  treeTrace.pop_back();

  // 1.it is leafNode now;
  leaf.read( cPid, pf);
  if ( leaf.getKeyCount() < getMaxKeyNum() ) { //regular leaf insert
    leaf.insert(key, rid);
    leaf.write(cPid, pf);
    //
    cout << "Leaf regular insertion on page: " << cPid << endl;
    leaf.read(cPid, pf);
    printLeaf(leaf);
    //
  } else {   // 2.leaf insert and split
    leaf.insertAndSplit(key, rid, leafSib, sibMidKey);
    nPid = pf.endPid();
    leafSib.setNextNodePtr( leaf.getNextNodePtr() );
    //leafSib.ppid = leaf.ppid;
    leaf.setNextNodePtr( nPid );
    leaf.write(cPid, pf);
    leafSib.write(nPid, pf);
    //
    cout << "Leaf split insertion on page: " << cPid << endl;
    leaf.read(cPid, pf);
    printLeaf(leaf);
    cout << " 2nd page: " << nPid << endl;
    leafSib.read(nPid, pf);
    printLeaf(leafSib);
    //
    //cPid = leaf.ppid;
    cPid = treeTrace.back();
    //
    cout << "#treeTrace.back() is " << treeTrace.back() << endl;
    //
    
    treeTrace.pop_back();

    // go back to modify nonLeaf
    nonLeaf.read( cPid, pf );
    while ( nonLeaf.getKeyCount() >= getMaxKeyNum() ) {
      // 4.nonleaf insert and split
      int tKey = sibMidKey;
      nonLeaf.insertAndSplit( tKey, nPid, nonLeafSib, sibMidKey);
      //nonLeafSib.ppid = nonLeaf.ppid;
      nPid = pf.endPid();
      nonLeaf.write( cPid, pf);
      nonLeafSib.write( nPid, pf);

      //
      cout << "nonLeaf split insertion on page: " << cPid << endl;
      nonLeaf.read(cPid, pf);
      printNonLeaf(nonLeaf);
      cout << " 2nd nonLeaf on page: " << nPid << endl;
      nonLeafSib.read(nPid, pf);
      printNonLeaf(nonLeafSib);
      //

      //if ( nonLeaf.ppid < 0 ) {          // 6.nonleaf split&insert and root
      if ( treeTrace.size() ==  0 ) {      // 6.nonleaf split&insert and root  
	treeHeight++;
	BTNonLeafNode newRoot;
	newRoot.ppid = -1;
	newRoot.sPid.push_back(cPid);
	rootPid = pf.endPid();
	//nonLeaf.ppid = rootPid;
	//nonLeaf.write( cPid, pf );
	newRoot.write( rootPid, pf);
	cPid = rootPid;
	nonLeaf.read( cPid, pf );
	// finishing the rest of newRoot at nonLeaf regular insertion
	//
	cout << "Creating new root node at page: " << cPid << endl;
	printNonLeaf(nonLeaf);
	//
      } else {                   // 5.nonLeaf split&insert and not a root node
	//cPid = nonLeaf.ppid;
	cPid = treeTrace.back();
	treeTrace.pop_back();
	nonLeaf.read( cPid, pf );
      }
    }
    // 3.nonLeaf regular insertion
    nonLeaf.insert( sibMidKey, nPid);
    nonLeaf.write( cPid, pf);
    //
    cout << "nonLeaf regular insertion on Page: " << cPid << endl;
    nonLeaf.read(cPid, pf);
    printNonLeaf(nonLeaf);  
    //
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
  //PageFile cPage;
  BTNonLeafNode nonLeaf;
  BTLeafNode leaf;
  int cHeight;       //current height

  //start from the root
  cursor.pid = rootPid;
  cHeight = 1;
  while ( cHeight < treeHeight ) {  //it is nonLeafNode
    nonLeaf.read( cursor.pid, pf );
    nonLeaf.locateChildPtr( searchKey, cursor.pid );
    cHeight++;
  }
  //it is leafNode now;
  leaf.read( cursor.pid, pf);
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
  //PageFile currentP;
  BTLeafNode leaf;
  leaf.read(cursor.pid, pf);
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
  //PageFile cPage;
  BTNonLeafNode nonLeaf;
  BTLeafNode leaf;
  int cHeight;       //current height
  PageId cPid;
  int cEid;

  //start from the root
  cout << "-------------------------------------" << endl;
  cout << "The tree root is at page:" << rootPid << endl;
  cPid = rootPid;
  cHeight = 1;

  nonLeaf.read(cPid, pf);
  cout << "RootNode level 1" << endl;;
  printNonLeaf(nonLeaf);
  while ( cHeight < treeHeight - 1 ) {
    cout << "At level " << cHeight + 1 << endl;
    for ( int i = 0; i < nonLeaf.sPid.size(); i++ ) {
      BTNonLeafNode nonLeafSameLevel;
      nonLeafSameLevel.read( nonLeaf.sPid[i], pf);
      printNonLeaf(nonLeafSameLevel);
    }
    cPid = nonLeaf.sPid[0];
    nonLeaf.read( cPid, pf );
    cHeight++;
  }
  cPid = nonLeaf.sPid[0];
  //leaf.read( cPid, pf );
  
  //it is leafNode now;
  cout << "leaf nodes:" << endl;
  while ( cPid != 0 ) {
    leaf.read( cPid, pf );
    cout << "On page " << cPid << ": ";
    for ( int i = 0; i < leaf.getKeyCount(); i++ ) {
      cout << "[(" << leaf.sRid[i].pid << "," << leaf.sRid[i].sid
	   << ")" << leaf.sKey[i] << "] ";
    }
    cout << endl;
    cPid = leaf.getNextNodePtr();
    //leaf.read( cPid, pf );
  }
}

void BTreeIndex::printLeaf( BTLeafNode & leaf ) {
  cout << " Rids are ";
  for ( int i = 0; i < leaf.sRid.size(); i++ ) {
    cout << "(" << leaf.sRid[i].pid << "," << leaf.sRid[i].sid << ") ";
  }
  cout << endl;
  cout << " Keys are ";
  for ( int i = 0; i < leaf.sKey.size(); i++ ) {
    cout << leaf.sKey[i] << " ";
  }
  cout << endl;
}
void BTreeIndex::printNonLeaf( BTNonLeafNode & nonLeaf) {
  cout << " Pids are ";
  for ( int i = 0; i < nonLeaf.sPid.size(); i++ ) {
    cout << "(" << nonLeaf.sPid[i] << ") ";
  }
  cout << endl;
  cout << " Keys are ";
  for ( int i = 0; i < nonLeaf.sKey.size(); i++ ) {
    cout << nonLeaf.sKey[i] << " ";
  }
  cout << endl;
}


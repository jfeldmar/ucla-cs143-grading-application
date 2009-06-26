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

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
    treeHeight = 0;
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
  RC   rc;
  PageId pid;
  
  rc = 0;
  // open the page file
  if ((rc = pf.open(indexname, mode)) < 0) return rc;
  //printf("Opened index file\n");
  
  //Read the first page if it exist
  pid = pf.endPid();
  if(pid > 0){
    //printf("Index persisted, reading data page\n");
    if((rc = pf.read(0,(void*)(&d))) < 0) return rc;
    treeHeight = d.height;
    rootPid = d.root;
    rc = root.read(rootPid,pf);
    //printf("height %d, rootPid %d, rc %d\n",treeHeight,rootPid,rc);
  }
  else{
    //printf("Creating new data page\n");
    treeHeight = d.height = d.min = d.max = d.num_entries = 0;
    rootPid = d.root = -1;
    pf.write(pid,(void*)(&d));
  }
  return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
  RC rc;
  d.root = rootPid;
  d.height = treeHeight;
  if((rc = pf.write(0,(void*)(&d))) < 0) return rc;
  if((rc = root.write(rootPid,pf)) < 0) return rc;
  return pf.close();
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
  if(key > d.max)
    d.max = key;
  if(key < d.min)
    d.min = key;
  d.num_entries++;

  if(treeHeight == 0 && rootPid == -1){
    rootPid = pf.endPid();
    treeHeight = 2;
    PageId pidl, pidr;
    BTLeafNode leaf;
    pidl = rootPid + 1;
    pidr = rootPid + 2;
    root.initializeRoot(pidl,key,pidr);
    root.write(rootPid,pf);
    leaf.setNextNodePtr(pidr);
    leaf.write(pidl,pf);
    leaf.insert(key,rid);
    leaf.setNextNodePtr(RC_END_OF_TREE);
    leaf.write(pidr,pf);
  }
  else{
    int skey = 0;
    PageId spid = 0;
    RC rc;
    rc = insertHelper(root,key,rid,1,skey,spid);
    if(rc == BTREEINDEX_SPLIT){
      return insertRoot(skey,spid);
    }
  }
  return 0;
}

RC BTreeIndex::insertRoot(int& skey, PageId& spid){
  RC rc;
  rc = root.insert(skey,spid);
  if(rc == RC_NODE_FULL){
    /*
    printf("Inserting new root\n");
    printf("Old root before split\n");
    root.printData(0);
    */
    BTNonLeafNode newroot,newright;
    int midkey = 0;
    PageId pid_right, pid_root;
    pid_right = pf.endPid();
    pid_root = pf.endPid()+1;
    rc = root.insertAndSplit(skey,spid,newright,midkey);
    /*
    printf("NRPID %d, midkey %d\n",pid_root,midkey);
    printf("Data after split\n");
    printf("Old root\n");
    root.printData(0);
    printf("New right\n");
    newright.printData(0);
    */
    root.write(rootPid,pf);
    newright.write(pid_right,pf);
    newroot.initializeRoot(rootPid,midkey,pid_right);
    newroot.write(pid_root,pf);
    rootPid = pid_root;
    root = newroot;
    treeHeight++;
  }
  return 0;
}

RC BTreeIndex::insertHelper(BTNonLeafNode& n, int key, const RecordId& rid, 
			    int height, int& skey, PageId& spid){
  PageId pid;
  RC rc;
  rc = n.locateChildPtr(key,pid);
  
  if(height == treeHeight-1){
    BTLeafNode leaf;
    leaf.read(pid,pf);
    rc = leaf.insert(key,rid);
    if(rc == RC_NODE_FULL){//leaf.getKeyCount() > 0){
      skey = 0;
      spid = pf.endPid();
      BTLeafNode newleaf;
      rc == leaf.insertAndSplit(key,rid,newleaf,skey);
      //printf("Insert Key %d, RC = %d, Sibkey is %d\n",key,rc,skey);
      leaf.setNextNodePtr(spid);
      leaf.write(pid,pf);
      newleaf.write(spid,pf);
      
      return BTREEINDEX_SPLIT;
    }
    //rc = leaf.insert(key,rid);
    leaf.write(pid,pf);
  }
  else{
    BTNonLeafNode internal;
    internal.read(pid,pf);
    rc = insertHelper(internal,key,rid,(height+1),skey,spid);
    if(rc == BTREEINDEX_SPLIT){
      return insertNonLeaf(internal,pid,skey,spid);
    }
  }
  return 0;
}

RC BTreeIndex::insertNonLeaf(BTNonLeafNode& n, const PageId wpid, int& skey, PageId& spid){
  RC rc;
  rc = n.insert(skey,spid);
  if(rc == RC_NODE_FULL){
    int midkey = 0;
    PageId newpid = pf.endPid();
    BTNonLeafNode newnode;
    n.insertAndSplit(skey,spid,newnode,midkey);
    newnode.write(newpid,pf);
    n.write(wpid,pf);
    skey = midkey;
    spid = newpid;
    return BTREEINDEX_SPLIT;
  }
  else if(rc == 0){
    return n.write(wpid,pf);
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
  if(treeHeight == 0 && rootPid == -1)
    return RC_INVALID_ATTRIBUTE;
  
  if(treeHeight == 2)
    return locateHelper(root,searchKey,cursor,1);
  else{
    BTNonLeafNode n;
    PageId pid;
    RC rc;
    if((rc = root.locateChildPtr(searchKey,pid)) != 0) return rc;
    n.read(pid,pf);
    return locateHelper(n,searchKey,cursor,2);
  }
}

RC BTreeIndex::locateHelper(BTNonLeafNode& n, int key, IndexCursor& cursor, int height){
  PageId pid;
  RC rc;
  
  if((rc = n.locateChildPtr(key,pid)) != 0) return rc;

  if(height == treeHeight-1){
    int entry,eid = 0;
    RecordId rid;
    BTLeafNode leaf;
    //if((rc = n.locateChildPtr(key,pid)) != 0) return rc;
    leaf.read(pid,pf);
    if((rc = leaf.locate(key,eid)) != 0) return rc;
    cursor.pid = pid;
    cursor.eid = eid;
    leaf.readEntry(eid,entry,rid);
    if(entry < key)
      readForward(cursor,entry,rid);
  }
  else{
    n.read(pid,pf);
    rc = locateHelper(n,key,cursor,(height+1));
  }
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
  if(cursor.pid < 0 || cursor.eid < 0){
    return RC_INVALID_ATTRIBUTE;
  }
   
  if(cached_pid != cursor.pid){
    //fprintf(stderr,"Updating cache (pid,eid) = (%d,%d)\n",cursor.pid,cursor.eid);
    cached_node.read(cursor.pid,pf);
    cached_pid = cursor.pid;
  }
  cached_node.readEntry(cursor.eid,key,rid);
  cursor.eid++;
  if(cursor.eid >= cached_node.getKeyCount()){
    cursor.pid = cached_node.getNextNodePtr();
    cursor.eid = 0; 
  }
  return 0;
}

/*
 * Outputs the contents of the Index
 */
RC BTreeIndex::printIndex(){
  printf("Root\n");
  return printIndexNonLeaf(root,1);
}

RC BTreeIndex::printIndexNonLeaf(BTNonLeafNode& n, int height){
  if(treeHeight == 0){
    printf("Index is not yet initialized\n");
    return 0;
  }

  printf("NonLeaf: Level %d\n",height);

  for(int i = 0; i < n.getKeyCount(); i++){
    if(i > 0) printf(", ");
    printf("%d",n.d.entries[i].key);
  }  
  printf("\n");
  
  if(height == treeHeight - 1){
    printf("Leaf: Level %d\n", height+1);
    BTLeafNode leaf;
    leaf.read(n.d.left,pf);
    printf("\tLeaf %d @ pid %d\n",0,n.d.left);
    printIndexLeaf(leaf);
    for(int i = 0; i < n.getKeyCount(); i++){
      printf("\tLeaf %d @ pid %d\n",i+1,n.d.entries[i].pid);
      leaf.read(n.d.entries[i].pid,pf);
      printIndexLeaf(leaf);
    }
  }
  else{
    BTNonLeafNode internal;
    internal.read(n.d.left,pf);
    printIndexNonLeaf(internal,height+1);
    for(int i = 0; i < n.getKeyCount(); i++){
      internal.read(n.d.entries[i].pid,pf);
      printIndexNonLeaf(internal,height+1);
    }
  }

  return 0;
}

RC BTreeIndex::printIndexLeaf(BTLeafNode& leaf){
  printf("\t");
  for(int i = 0; i < leaf.getKeyCount(); i++){
    if(i > 0) printf(", ");
    printf("%d",leaf.d.entries[i].key);
  }
  printf("\n");
  return 0;
}

void BTreeIndex::getStats(btstats& stats){
  stats.min = d.min;
  stats.max = d.max;
  stats.num_entries = d.num_entries;
}

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
 * BTreeIndex destructor
 */
BTreeIndex::~BTreeIndex()
{
  geneprintf("\nin BTreeIndex DESTRUCTORRR\n\n");
  if (close() < 0)
    geneprintf("\nBTreeIndex closed already\n\n");
  else
    geneprintf("\nBTreeIndex now closed\n\n");
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 *
 * gene: metadata is stored in page 0 of pagefile
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
  RC rc = 0;

  int buffer[PageFile::PAGE_SIZE/sizeof(int)]; // the buffer used for caching
  memset(buffer, 0, PageFile::PAGE_SIZE);

  // Open the page file, create if not exist if in write mode
  if ((rc = pf.open(indexname + ".idx", mode)) < 0 && mode == 'r') {
    geneprintf("BTreeIndex::open() page file could not be opened.\trc==%d\n", rc);
    return rc;
  } else {
    geneprintf("BTreeIndex::open() page file created.\trc==%d\n", rc);
  }

  // Setup index metadata
  if (pf.endPid() == 0) { // if index is new
    pf.write(0, buffer); // Grab page 0 for index metadata
    /* Reserving page 0 only. Leave metadata (rootPid treeHeight) as is. */
  } else {
    /* Metadata stored in page as:
     *  +---------+------------+
     *  | rootPid | treeHeight |
     *  +---------+------------+
     * Assume both elements of type int (hence buffer type is int)
     */
    pf.read(0, buffer);
    rootPid = buffer[0];
    treeHeight = buffer[1];
    gene2printf("buffer[ 0==rootPid==%d , 1==treeHeight%d ]\n", rootPid, treeHeight);
#if debug==1
    assert(rootPid > 0); // page 0 reserved for index metadata
    assert(treeHeight > 0);
#endif
  }

  return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
  RC rc = 0;

  int buffer[PageFile::PAGE_SIZE/sizeof(int)];
  memset(buffer, 0, PageFile::PAGE_SIZE);

  buffer[0] = rootPid;
  buffer[1] = treeHeight;

  if ((rc = pf.write(0, buffer)) < 0) {
    geneprintf("BTreeIndex::close() failed to write to PageFile pf\n");
    return rc;
  }
  pf.close();

  return rc;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
  RC rc = 0;
  geneprintf("in BTreeIndex::insert(key==%d)\ttreeHeight==%d\n",key,treeHeight);

  // Get root
  if (rootPid < 0) { // if root has not been initialized yet; treeHeight == 0
    geneprintf("BTreeIndex::insert()\trootPid < 0\n");
    rootPid = pf.endPid();
    treeHeight = 1;
  }
  // because insert_aux() is recursive and it returns multiple values to itself
  // by reference, we clone our rootPid to ensure it does not get changed by
  // insert_aux(). Changing root is handled by BTreeIndex::insert()
  int pid = rootPid;
  if (!insert_aux(key, rid, pid, treeHeight)) { // need new root?
    // key is now the middle key. pid is now it's pointer.
    int old_rootPid = rootPid;
    BTNonLeafNode new_root;
    // Create new root and update metadata
    rootPid = pf.endPid();
    new_root.read(rootPid, pf);
    ++treeHeight;
    // Initialise
#if debug==1
    assert(old_rootPid != pid);
#endif
    new_root.initializeRoot(old_rootPid, key, pid);
    // Write to disk
    new_root.write(new_root.getPid(), pf);
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
  int rc = 0;
  PageId pid_to_follow;
  BTNonLeafNode nonleaf;
  BTLeafNode leaf;

  //========================================================//
  // This block of code gets the leaf with searchKey        //
  //========================================================//
  if (rootPid < 0) { // If index is uninitialised
    return RC_END_OF_TREE;
  } else if (treeHeight == 1) { // Elif root is leaf
    gene2printf("BTreeIndex::locate()\ttreeHeight == 1\n"); 
    pid_to_follow = rootPid; 
  } else if (treeHeight > 1) { // Elif root is nonleaf 
    gene2printf("BTreeIndex::locate()\ttreeHeight > 1\n"); 
    nonleaf.read(rootPid, pf); 
    // Prepare to get leaf 
    int height = treeHeight; 
    while (height-- > 1) { 
      if ((rc = nonleaf.locateChildPtr(searchKey, pid_to_follow) < 0)) { 
        gene2printf("BTNonLeafNode::locate()\tlocateChildPtr() failed. rc==%d\n h was %d\n", rc, height+1);
        return RC_END_OF_TREE;
      }
      // can reuse nonleaf because we're reading only
      nonleaf.read(pid_to_follow, pf);
    }
  } else { 
    gene2printf("\n\nBTreeIndex::locate()\tXXX should not happen\n\n"); 
#if debug==1
    assert(1==0);
#endif
    return RC_END_OF_TREE; 
  }

  // Get leaf
  leaf.read(pid_to_follow, pf);

  int eid;
#if debug==1
  eid = INT_MIN;
#endif
  if ((rc = leaf.locate(searchKey, eid)) < 0) { // locate() modifies eid
    gene2printf("BTreeIndex::locate()\tCannot locate searchKey %d in leaf w/pid==%d\n",searchKey,leaf.getPid());
#if debug==1
    eid = INT_MIN;
#endif
    // if failed, Get sibling's (if have one) first record
    if ((pid_to_follow = leaf.getNextNodePtr()) >= 0) {
      // Get sibling
      leaf.read(pid_to_follow, pf);
      // below call to BTLeafNode::locate() should not fail
#if debug==1
      gene2printf("BTreeIndex::locate()\tsibling w/pid==%d about to locate(searchKey==%d, eid==%d)\n",leaf.getPid(),searchKey,eid);
      assert(leaf.locate(searchKey, eid) >= 0); // modifies eid
#else
      if (leaf.locate(searchKey, eid) < 0) // modifies eid
        return -1337; // for grepping's sake
#endif
    } else { // No sibling implies searchKey >ALL keys in tree
      return RC_NO_SUCH_RECORD;
    }
  }
#if debug==1
  gene2printf("\nback in BTI:locate()\teid is %d\n\n", eid);
  assert(eid != INT_MIN);
#endif
  // Set return values
  cursor.pid = pid_to_follow;
  cursor.eid = eid;
  gene2printf("BTreeIndex::locate()\treturning. cursor.pid==%d .eid==%d\n",cursor.pid,cursor.eid);
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
  int rc = 0;
  BTLeafNode leaf;
#if debug==1
  int _eid = cursor.eid;
  int _key = key = INT_MIN;
  int _ridp = rid.pid = INT_MAX;
  int _rids = rid.sid = INT_MAX;
#endif
  // BTLeafNode::read() returns non-RC_INVALID_PID (-1007) errors now
  if ((rc = cursor.pid) < 0 || (rc = leaf.read(cursor.pid, pf)) < 0 ||
      (rc = leaf.readEntry(cursor.eid, key, rid)) < 0) // modifies key, rid
    return rc;
#if debug==1
  assert(key != _key);
  assert(rid.pid != _ridp);
  assert(rid.sid != _rids);
  assert(cursor.eid == _eid);
#endif
  ++cursor.eid;
  // If need to cross to next leaf
  if (cursor.eid == leaf.getKeyCount()) {
    gene2printf("BTreeIndex::readForward()\tcursor.eid was %d now %d. exhausted leaf node w/pid==%d. Getting next node w/pid==%d\n",cursor.eid-1,cursor.eid,(int)leaf.getPid(),(int)leaf.getNextNodePtr());
    cursor.pid = leaf.getNextNodePtr();
    cursor.eid = 0;
  }
  gene2printf("BTreeIndex::readForward()\tReturning cursor.pid==%d, .eid==%d\n",cursor.pid,cursor.eid);
  return 0; // btw, BTLeafNode::readEntry()'s complexity sucks balls
}


/*
 * (diabolically side-effect-filled) Auxillary function for BTreeIndex::insert()
 */
bool BTreeIndex::insert_aux(int &k, const RecordId &r, PageId &p, unsigned int h)
{
  geneprintf("in BTreeIndex::insert_aux(&k==%d, &r, &p==%d, h==%d)\n",k,(int)p,h);

  BTNonLeafNode nonleaf; // used only when treeHeight > 1
  BTLeafNode leaf;

  if (h > 1) { // if current node is nonleaf
    nonleaf.read(p, pf);
#if debug==1
    PageId _p = p;
#endif
    nonleaf.locateChildPtr(k, p); // modifies p
#if debug==1
    if (p != _p)
      geneprintf("insert_aux(&k==%d, &r, &p==%d, h==%d)\n",k,(int)p,h);
    assert(p != _p);
#endif
    if (!insert_aux(k, r, p, h-1)) { // if need to update current (nonleaf) node
      // k is now the key to insert. p is now the pid to insert.
      if (nonleaf.insert(k, p) < 0) {
        geneprintf("nonleaf.insert() failed. getting pid %d\n", pf.endPid());
        // Create new sibling node
        BTNonLeafNode sibling;
        sibling.read(pf.endPid(), pf);
        // Split with sibling
        int mid_key;
#if debug==1
        mid_key = INT_MIN;
#endif
        nonleaf.insertAndSplit(k, p, sibling, mid_key); // modifies mid_key
#if debug==1
        assert(mid_key != INT_MIN);
#endif
        // Pass back by reference
        k = mid_key;
        p = sibling.getPid();
        // Update current node and write sibling node
        nonleaf.write(nonleaf.getPid(), pf);
        sibling.write(sibling.getPid(), pf);
        geneprintf("   BTreeIndex::insert_aux() returning false\n\n");
        return false;
      }
      nonleaf.write(nonleaf.getPid(), pf);
    }
    geneprintf("   BTreeIndex::insert_aux() returning true\n\n");
    return true;
  } else { // otherwise current node is leaf
    leaf.read(p, pf);
    if (leaf.insert(k, r) < 0) {
      PageId old_neighbour_pid = leaf.getNextNodePtr();
      gene2printf("   BTreeIndex::insert_aux(k==%d)\tcurr_pid==%d, old_neighbour_pid==%d\n",k,leaf.getPid(), old_neighbour_pid);
      // Create new sibling node
      BTLeafNode sibling;
      sibling.read(pf.endPid(), pf);
      gene2printf("   BTreeIndex::insert_aux(k==%d)\tgetting pid==%d sibling_pid==%d\n",k,pf.endPid(),sibling.getPid());
      // Split with sibling
      int sibling_key;
#if debug==1
      sibling_key = INT_MAX;
#endif
      leaf.insertAndSplit(k, r, sibling, sibling_key); // modifies sibling_key
#if debug==1
      assert(sibling_key != INT_MAX);
#endif
      // Update pointers to next nodes
      sibling.setNextNodePtr(old_neighbour_pid);
      leaf.setNextNodePtr(sibling.getPid());
      gene2printf("   BTreeIndex::insert_aux(k==%d)\tsibling's_neighbour==%d\n",k,sibling.getNextNodePtr());
      // Pass back by reference
      k = sibling_key; 
      p = sibling.getPid();
      // Write nodes
      leaf.write(leaf.getPid(), pf);
      sibling.write(sibling.getPid(), pf);
      geneprintf("   BTreeIndex::insert_aux() returning false\n\n");
      return false;
    }
    leaf.write(leaf.getPid(), pf);
    geneprintf("   BTreeIndex::insert_aux() returning true\n\n");
    return true;
  }
}

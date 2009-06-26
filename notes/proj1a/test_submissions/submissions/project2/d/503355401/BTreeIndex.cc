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
    treeHeight = -1;
}

/* Displays the contents of a B-tree */
void BTreeIndex::print(PageId pid, int depth)
{
    if (pid == -1) pid = rootPid;
    /* Sanity check: tree is initialized */
    if (treeHeight == -1) return;
    if (depth == 0) fprintf(stderr, "==============================\n");
    fprintf(stderr, ">>>>> Page %d <<<<< ", pid);
    if (depth == treeHeight) {
        fprintf(stderr, "(LEAF)\n");
        BTLeafNode node;
        node.read(pid, pf);
        node.print();
    }
    else {
        fprintf(stderr, "(NONLEAF)\n");
        BTNonLeafNode node;
        node.read(pid, pf);
        node.print();
        /* First print Prev node */
        BTNonLeafNode::Prev prev;
        node.getPrev(prev);
        print(prev.pid, depth + 1);
        for (int eid = 0; eid < node.getKeyCount(); eid++) {
            BTNonLeafNode::Pair pair;
            node.getPair(eid, pair);
            print(pair.pid, depth + 1);
        }
    }
    if (depth == 0) fprintf(stderr, "==============================\n");
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
    int r = pf.open(indexname, mode);
    if (mode == 'r') {
        BTLeafNode metadata;
        metadata.read(0, pf);
        BTLeafNode::Pair meta;
        metadata.getPair(0, meta);
        treeHeight = meta.key;
        metadata.getPair(1, meta);
        rootPid = meta.key;
    }
    return r;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    BTLeafNode metadata;
    BTLeafNode::Pair meta;
    meta.key = treeHeight;
    metadata.setPair(0, meta);
    meta.key = rootPid;
    metadata.setPair(1, meta);
    metadata.write(0, pf);
    pf.close();
    return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid, int& insertPid, int& insertKey, PageId pid, int depth)
{
    /* If tree is unitialized, start with leaf node */
    if (treeHeight == -1) {
        BTLeafNode metadata;
        metadata.write(0, pf);
        BTLeafNode node;
        node.insert(key, rid);
        rootPid = pf.endPid();
        node.write(rootPid, pf);
        treeHeight++;
        return 0;
    }
    if (pid == -1) pid = rootPid;
    /* If we are at the bottom of the tree, insert */
    if (depth == treeHeight) {
        BTLeafNode node;
        node.read(pid, pf);
        /* Insert node if space permits */
        if (node.getKeyCount() < BTLeafNode::MAX_PAIRS) {
            node.insert(key, rid);
            node.write(pid, pf);
        }
        /* Else, split */
        else {
            BTLeafNode sibling;
            int siblingKey;
            node.insertAndSplit(key, rid, sibling, siblingKey);
            PageId siblingPid = pf.endPid();
            node.setNextNodePtr(siblingPid);
            node.write(pid, pf);
            sibling.write(siblingPid, pf);
            /* Create a new non-leaf root */
            if (depth == 0) {
                BTNonLeafNode root;
                root.initializeRoot(pid, siblingKey, siblingPid);
                rootPid = pf.endPid();
                root.write(rootPid, pf);
                treeHeight++;
                return 0;
            }
            else {
                insertPid = siblingPid;
                insertKey = siblingKey;
                return 2;
            }
        }
    }
    else {
        BTNonLeafNode node;
        PageId nextPid;
        node.read(pid, pf);
        node.locateChildPtr(key, nextPid);
        /* Split this node if necessary */
        if (insert(key, rid, insertPid, insertKey, nextPid, depth + 1) == 2) {
            /* Insert if space permits */
            if (node.getKeyCount() < BTNonLeafNode::MAX_PAIRS) {
                node.insert(insertKey, insertPid);
                node.write(pid, pf);
            }
            /* Else, split */
            else {
                BTNonLeafNode sibling;
                int midKey;
                node.insertAndSplit(insertKey, insertPid, sibling, midKey);
                node.write(pid, pf);
                PageId siblingPid = pf.endPid();
                sibling.write(siblingPid, pf);
                /* If we must split root */
                if (depth == 0) {
                    BTNonLeafNode root;
                    root.initializeRoot(pid, midKey, siblingPid);
                    rootPid = pf.endPid();
                    root.write(rootPid, pf);
                    treeHeight++;
                    return 0;
                }
                else {
                    insertPid = siblingPid;
                    insertKey = midKey;
                }
                return 2;
            }
        }
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
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor, PageId pid, int depth)
{
    /* Sanity check: tree is initialized */
    if (treeHeight == -1) return 1;
    if (pid == -1) pid = rootPid;
    /* If we are at the bottom of the tree, read a leaf node and locate */
    if (depth == treeHeight) {
        BTLeafNode node;
        int eid;
        node.read(pid, pf);
        node.locate(searchKey, eid);
        cursor.pid = pid;
        cursor.eid = eid;
    }
    else {
        BTNonLeafNode node;
        PageId nextPid;
        node.read(pid, pf);
        node.locateChildPtr(searchKey, nextPid);
        locate(searchKey, cursor, nextPid, depth + 1);
    }
    return 0;
}

RC BTreeIndex::getFirst(PageId &pid, PageId parent, int depth)
{
    /* Sanity check: tree is initialized */
    if (treeHeight == -1) return 1;
    if (treeHeight == 0) {
        pid = rootPid;
        return 0;
    }
    if (parent = -1) parent = rootPid;
    /* If we are at the bottom of the tree, read a leaf node and locate */
    if (depth == treeHeight - 1) {
        BTNonLeafNode node;
        BTNonLeafNode::Prev prev;
        node.read(parent, pf);
        node.getPrev(prev);
        pid = prev.pid;
    }
    else {
        BTNonLeafNode node;
        BTNonLeafNode::Prev prev;
        node.read(parent, pf);
        node.getPrev(prev);
        getFirst(pid, prev.pid, depth + 1);
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
 *      returns 1 if done reading forward
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
    BTLeafNode node;
    node.read(cursor.pid, pf);
    node.readEntry(cursor.eid, key, rid);
    if (cursor.eid < node.getKeyCount()) {
        cursor.eid++;
    }
    else {
        cursor.pid = node.getNextNodePtr();
        cursor.eid = 0;
    }
    if (cursor.pid == -1) return 1;
    return 0;
}

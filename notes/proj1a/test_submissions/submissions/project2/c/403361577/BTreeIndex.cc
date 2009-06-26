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
    rootPid = 1;
    treeHeight = 0;
    nextPid = 0;
    isEmpty = 1;
    indexPid = 0;
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
    // get the file name
    string file = indexname;
    // set the file type
    file.append(".idx");
    
    if (mode == 'r') {
        // open the file with read mode
        if (pf.open(file, 'r') == 0)
            return 0;
        return -1;
    }
    
    if (mode == 'w') {
        // open the file with write mode
        if (pf.open(file, 'w') == 0) {
            // update next available pid
            // note that pid 1 is reserved for root
            nextPid = 2;
            // update treeHeight since we're creating a node for root
            treeHeight = 1;
            
            // update index's information into pid 0, index's page
            updateIndex();

            // create a root node
            BTNonLeafNode root;
            // write this root node to disk
            if (root.write(rootPid, pf) != 0)
                return -1;
            
            return 0;
        }
        return -1;
    }
    
    return -1;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    if (pf.close() == 0)
        return 0;
    return -1;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
    // indicator of the current tree level
    int treeLevel = 1;

    // on empty set up root
    if (isEmpty) {
        // create a root pointer
        BTNonLeafNode root;
        // read contents of the root into buffer
        if (root.read(rootPid, pf) != 0)
            return -1;
        // set left and right page Ids
        PageId lpid = nextPid;
        PageId rpid = nextPid + 1;
        // initialize the root
        root.initializeRoot(lpid, key, rpid);
        // write the root information back to disk
        if (root.write(rootPid, pf) != 0)
            return -1;
        
        // create a 2 empty leaf nodes
        BTLeafNode leftLeaf;
        BTLeafNode rightLeaf;
        
        // set next node pointer in left leaf
        leftLeaf.setNextNodePtr(rpid);
        
        // write the leaf information back to disk
        if (leftLeaf.write(lpid, pf) != 0)
            return -1;
        // write the leaf information back to disk
        if (rightLeaf.write(rpid, pf) != 0)
            return -1;
        
        // update next available pid
        nextPid += 2;
        // update new tree height
        treeHeight++;
        // the index is no longer empty
        isEmpty = 0;
    }
    
    // initialize a leaf node
    BTLeafNode leafNode;
    // initialize a non-leaf node
    BTNonLeafNode nonLeafNode;
    // initialize a root node
    BTNonLeafNode rootNode;
    // child and parent pid trackers
    PageId childPid;
    PageId parentPid[treeHeight];

    ////////////////////////////////////
    // START SEARCHING FROM ROOT NODE //
    ////////////////////////////////////
    
    // read to the root node
    if (rootNode.read(rootPid, pf) != 0)
        return -1;
    // update parent pid
    parentPid[treeLevel-1] = rootPid;
    // locate the child pointer
    if (rootNode.locateChildPtr(key, childPid) != 0) {
        return -1;
        printf("\nFAILED TO LOCATE FROM ROOT\n");
    }
    // update tree level
    treeLevel++;
    
    ////////////////////////////////////////////
    // SEARCH THROUGH POSSIBLE NON-LEAF NODES //
    ////////////////////////////////////////////
        
    // traverse down the tree level to find leaf node
    while (treeLevel < treeHeight) {
        // update parent pid
        parentPid[treeLevel-1] = childPid;
        // read to the non-leaf node
        if (nonLeafNode.read(childPid, pf) != 0)
            return -1;
        // locate the next child pointer
        if (nonLeafNode.locateChildPtr(key, childPid) != 0) {
            printf("\nFAILED TO LOCATE FROM NON-LEAF\n");
            return -1;
        }
        treeLevel++;
    }
    
    /////////////////////
    // FOUND LEAF NODE //
    /////////////////////

    // read the leaf node
    if (leafNode.read(childPid, pf) != 0)
        return -1;
    // insert the key and rid into leaf
    if (leafNode.insert(key, rid) == 0) {
        // write the leaf contents to disk
        if (leafNode.write(childPid, pf) != 0)
            return -1;
    } else {
        
        ///////////////////
        // LEAF OVERFLOW //
        ///////////////////

        int sibkey;
        BTLeafNode leafSibling;
        
        // we will need to split the leaf node if we failed
        if (leafNode.insertAndSplit(key, rid, leafSibling, sibkey) != 0)
            return -1;
        
        // get the leaf node's current next node ptr
        PageId nextNodePid = leafNode.getNextNodePtr();
        // set up sibling's next node ptr pointer
        leafSibling.setNextNodePtr(nextNodePid);
        
        // overwrite original leaf node's next node ptr
        leafNode.setNextNodePtr(nextPid);
        
        // write contents of the leaf node to disk
        if (leafNode.write(childPid, pf) != 0)
            return -1;
        
        // write sibling node into disk
        if (leafSibling.write(nextPid, pf) != 0)
            return -1;
        
        PageId splittedPid = nextPid;
        
        // update next available pid
        nextPid++;
        
        BTNonLeafNode nonLeafParent;

        treeLevel--;

        // read parent to the non-leaf node
        if (nonLeafParent.read(parentPid[treeLevel-1], pf) != 0)
            return -1;
        // insert the first key of this overflow to parent
        if (nonLeafParent.insert(sibkey, splittedPid) == 0) {
            // write contents of the non-leaf node to disk
            if (nonLeafParent.write(parentPid[treeLevel-1], pf) != 0)
                return -1;
        } else {

            ///////////////////////
            // NON-LEAF OVERFLOW //
            ///////////////////////
            
            int midkey;
            
            // insert any non-leaf overflows to needed non-leaf nodes
            do {
                BTNonLeafNode nonLeafSibling;
            
                // we will need to split the non-leaf node if we failed
                if (nonLeafParent.insertAndSplit(sibkey, splittedPid, nonLeafSibling, midkey) != 0)
                    return -1;
                    
                // write contents of the non-leaf node to disk
                if (nonLeafParent.write(parentPid[treeLevel-1], pf) != 0)
                    return -1;
                    
                // write contents of the sibling non-leaf node to disk
                if (nonLeafSibling.write(nextPid, pf) != 0)
                    return -1;
                
                sibkey = midkey;
                splittedPid = nextPid;
                    
                // update next available pid
                nextPid++;

                // if we need to overflow the root
                if (parentPid[treeLevel-1] == rootPid) {
                /*
                    BTNonLeafNode newRoot;
                    BTNonLeafNode oldRoot;
                    // set left and right page Ids
                    PageId lpid = parentPid[treeLevel-1];
                    PageId rpid = splittedPid;
                    // initialize the root
                    newRoot.initializeRoot(lpid, sibkey, rpid);
                    // read the root information
                    if (oldRoot.read(rootPid, pf) != 0)
                        return -1;
                    // write the root information back to disk
                    if (newRoot.write(nextPid, pf) != 0)
                        return -1;
                    nextPid++;
                */
                    BTNonLeafNode newRoot;
                    //BTNonLeafNode oldRoot;
                    // set left and right page Ids
                    PageId lpid = rootPid;
                    PageId rpid = splittedPid;
                    // initialize the root
                    newRoot.initializeRoot(lpid, sibkey, rpid);

                    rootPid = nextPid;
                    // write the root information back to disk
                    if (newRoot.write(rootPid, pf) != 0)
                        return -1;
                    // update next available pid
                    nextPid++;
                    // update tree height
                    treeHeight++;
                }
            
                // we wanna traverse back up the tree
                treeLevel--;
                
                //printf("\nthis is tree level: %i",treeLevel);
                //printf("\ntried to insert parent pageid: %i", parentPid[treeLevel-1]);
                //printf("\ntried to insert parent sibkey: %i\n", sibkey);
                
                // finish on root
                if (treeLevel-1 < 0)
                    break;
            
                // read parent to the non-leaf node
                if (nonLeafParent.read(parentPid[treeLevel-1], pf) != 0)
                    return -1;
            } while (nonLeafParent.insert(sibkey, splittedPid) != 0);
            
            // write parent to the non-leaf node
            if (nonLeafParent.write(parentPid[treeLevel-1], pf) != 0)
                return -1;
        }
    }

    // update index's information into pid 0, index's page
    updateIndex();

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
    int treeLevel = 1;
    // initialize a leaf node
    BTLeafNode leafNode;
    // initialize a non-leaf node
    BTNonLeafNode nonLeafNode;
    // initialize a root node
    BTNonLeafNode rootNode;
    // child and parent pid trackers
    PageId childPid;

    ////////////////////////////////////
    // START SEARCHING FROM ROOT NODE //
    ////////////////////////////////////
    
    // read to the root node
    if (rootNode.read(rootPid, pf) != 0)
        return -1;
    // locate the child pointer
    if (rootNode.locateChildPtr(searchKey, childPid) != 0) {
        return -1;
        printf("\nFAILED TO LOCATE FROM ROOT\n");
    }
    // update tree level
    treeLevel++;
    
    ////////////////////////////////////////////
    // SEARCH THROUGH POSSIBLE NON-LEAF NODES //
    ////////////////////////////////////////////
        
    // traverse down the tree level to find leaf node
    while (treeLevel < treeHeight) {
        // read to the non-leaf node
        if (nonLeafNode.read(childPid, pf) != 0)
            return -1;
        // locate the next child pointer
        if (nonLeafNode.locateChildPtr(searchKey, childPid) != 0) {
            printf("\nFAILED TO LOCATE FROM NON-LEAF\n");
            return -1;
        }
        treeLevel++;
    }
    
    /////////////////////
    // FOUND LEAF NODE //
    /////////////////////

    // set the pid for the cursor
    cursor.pid = childPid;

    // read the leaf node
    if (leafNode.read(childPid, pf) != 0)
        return -1;
    
    // Find the leaf-node index entry whose key value is larger than or 
    // equal to searchKey
    if (leafNode.locate(searchKey, cursor.eid) != 0)
        return -1;
    
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
    // get the pid and eid we're looking for from cursor
    int pid = cursor.pid;
    int eid = cursor.eid;
    
    // initialize a leaf node
    BTLeafNode leafNode;
    
    // read the leaf node
    if (leafNode.read(pid, pf) != 0)
        return -1;
    
    // read the entry using eid
    if (leafNode.readEntry(eid, key, rid) != 0)
        return -1;
    
    // update cursor
    if (eid+1 > MAX_KEY_PER_PAGE) {
        cursor.pid = leafNode.getNextNodePtr();
        cursor.eid = 1;
        // set pid to -1 if we're at the very very end
        if (cursor.pid == 0) {
            cursor.pid = -1;
            cursor.eid = -1;
        }
    } else {
        cursor.eid++;
    }
    
    return 0;
}

// print contents of a node
void BTreeIndex::printNode(PageId pid)
{
    // create a node
    BTNonLeafNode node;
    // read the contents of this page ID
    node.read(pid, pf);
    // print the buffer inside this node
    node.printBuffer();
}

// clear contents of a node
void BTreeIndex::clearNode(PageId pid)
{
    // create a node
    BTNonLeafNode node;
    // read the contents of this page ID
    node.read(pid, pf);
    // print the buffer inside this node
    node.clearBuffer();
    // write the contents of this page ID
    node.write(pid, pf);
}

// update all index information into 0, index's Pid
void BTreeIndex::updateIndex()
{
    // create a buffer/page
    char buf[1024];
    // clear the buffer
    memset(buf, 0, 1024);
    // write the contents of index to this buffer
    memcpy(buf, &rootPid, sizeof(int));
    memcpy(buf + sizeof(int), &treeHeight, sizeof(int));
    memcpy(buf + 2*sizeof(int), &nextPid, sizeof(int));
    memcpy(buf + 3*sizeof(int), &isEmpty, sizeof(int));
    // write the contents to disk with page ID 0, index's pid
    pf.write(indexPid, buf);
}

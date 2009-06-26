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
#include "iostream"

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode) {
    RC   rc;

    // open the page file
    if ((rc = pf.open(indexname, mode)) < 0) return rc;

    if (pf.endPid() == 0) { // That mesns the file is empty
        rootPid = 1;
        treeHeight = 0;
        
        // write the new assigned rootPid and treeHeight into file
        write();
    }
    else {
        // Load the rootPid, treeHeight and the page of root into memory
        read();
        root.read(rootPid, pf);

        cout << "rootPid = " << rootPid << endl;
        cout << "treeHeight = " << treeHeight << endl;
    }

    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
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
    IndexCursor cursor;

    // Find the leaf node L that contain value V
    locate(key, cursor);

    // L = cursor.pid
    insert_entry(cursor.pid, key, rid);

    return 0;
}

// This insert_entry is for Leaf node
RC BTreeIndex::insert_entry(PageId L_pid, int key, const RecordId& rid) {
    RC rc;
    BTLeafNode L;
    BTLeafNode L_prime;
    int midKey;
    int L_prime_pid = pf.endPid();

    // Load the leaf into memory for inserting
    if ((rc = L.read(L_pid, pf)) < 0) return rc;

    if (!L.IsFull()) {
        // L.insert(v, p);
        L.insert(key, rid);
        L.write(L_pid, pf);
    }
    else {
        // "Create" a new page L_prime
        L_prime.read(L_prime_pid, pf);
        L.insertAndSplit(key, rid, L_prime, midKey);

        // Update "pointers"
        L_prime.setNextNodePtr(L.getNextNodePtr());
        L.setNextNodePtr(L_prime_pid);


        // Write to file
        L.write(L_pid, pf);
        L_prime.write(L_prime_pid, pf);

        // don't worry to check whether this is root, coz we are in Leaf class
        PageId parent_pid = path.back();
        path.pop_back();
        insert_entry(parent_pid, midKey, L_prime_pid);
    }

    // don't worry to check whether this is Leafr, coz we are in Leaf class
    // Fix next child pointers

    return 0;
}

// This insert_entry is for Non Leaf node, including root
RC BTreeIndex::insert_entry(PageId pid, int key, const PageId& pid_insert) {
    RC rc;
    BTNonLeafNode L;
    BTNonLeafNode L_prime;
    BTNonLeafNode newRoot;
    int midKey, midPid, newRootPid;
    int L_prime_pid = pf.endPid();

    // Load the leaf into memory for inserting

    if ((rc = L.read(pid, pf)) < 0)
        return rc;

    if (!L.IsFull()) {
        // L.insert(v, p);
        L.insert(key, pid_insert);
        L.write(pid, pf);
    }
    else {
        // "Create" a new page L_prime
        L_prime.read(L_prime_pid, pf);
        L.insertAndSplit(key, pid_insert, L_prime, midKey, midPid);

        // Update "pointers"
        L_prime.setPreNodePtr(midPid);

        // Write to file
        L.write(pid, pf);
        L_prime.write(L_prime_pid, pf);
        
        if (pid != rootPid) {
            PageId parent_pid = path.back();
            path.pop_back();
            insert_entry(parent_pid, midKey, L_prime_pid);
        }
        else
        {
            // Create a new root
            newRootPid = pf.endPid();
            newRoot.read(newRootPid, pf);
            newRoot.initializeRoot(pid, midKey, L_prime_pid);
            newRoot.write(newRootPid, pf);

            // Update both memory and file
            root = newRoot;
            rootPid = newRootPid;
            treeHeight++;
            root.write(rootPid, pf);
            
            // Update the infomation page of in the index file(the 1st page)
            write();
        }
    }

    // Update the root in memory, we don't need to update the file because
    // when we do L.write(pid, pf); above, the root is update, only in case of
    // pid = rootPid
    if (pid == rootPid)
        root = L;
    // Don't bother the check leaf node here coz here we are Non Leaf
    // (it is part of the algorithm tho)

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
    int rc;
    int level = 0;
    BTLeafNode leaf;
    BTNonLeafNode nonLeaf;

    if (pf.endPid() == 1) { // then we know there is no root
        // Initialize the root
        // the left point should be < 0, to indicate a "NULL" pointer,
        // corresponding to line Special(*)
        //root.initializeRoot(1, searchKey, 2);
        root.initializeRoot(-1, searchKey, 2);
        root.write(rootPid, pf);

        /* The first index will be at 3rd page of the index file(1st page
           used to store infomation of the index tree, 2nd page is the root)
           and the first entry ID is obviously 0
         */
        cursor.pid = pf.endPid();
        cursor.eid = 0;

        // Write the very first page in the new index file
        leaf.write(cursor.pid, pf);

        // Update the treeHeight
        setTreeHeight(2);
        write();
    }
    else {
        // Record the search path
        path.push_back(rootPid);

        // Found the next level node
        if ((rc = root.locateChildPtr(searchKey, cursor.pid)) < 0)
            return rc;

        // Special(*) case of new index file, "NULL" pointer
        if (cursor.pid < 0) {
            cursor.pid = pf.endPid();
            cursor.eid = 0;

            // Update the left-most pointer of the root
            root.read(rootPid, pf);
            memcpy(root.buffer + sizeof(int), &cursor.pid, sizeof(int));
            root.write(rootPid, pf);

            // Write the very first page in the new index file
            leaf.read(cursor.pid, pf);

            // Potential bug: May not always point to 2, need to come after
            // finishing the insertANDsplit
            leaf.setNextNodePtr(2);

            leaf.write(cursor.pid, pf);
        }
        else {
            while((treeHeight - level++) > 2) {
                path.push_back(cursor.pid);

                // Load the nonLeaf into memory for finding the next nonLeaf
                nonLeaf.read(cursor.pid, pf);

                // Found the next level node
                if ((rc = nonLeaf.locateChildPtr(searchKey, cursor.pid)) < 0)
                    return rc;
            }


            // At this point, we've found the cursor.pid which is the Leaf's pid
            // we want. We now get the cursor.eid and we done.
            leaf.read(cursor.pid, pf);
            leaf.locate(searchKey, cursor.eid);
        }
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
    return 0;
}

int BTreeIndex::getRootPid() {return rootPid;}
int BTreeIndex::getTreeHeight() {return treeHeight;}
void BTreeIndex::setRootPid(PageId pid) {rootPid = pid;}
void BTreeIndex::setTreeHeight(int height) {treeHeight = height;}

RC BTreeIndex::read() {
    
    // Load both rootPid and treeHeight into memory
    // 1st 4 bytes (sizeof(PageId)) is the rootpid
    // 2nd 4 bytes (sizeofint)) is the tree height
    RC rc;
    if ((rc = pf.read(0, buffer)) < 0) return rc;
    memcpy(&rootPid, buffer, sizeof(PageId));
    memcpy(&treeHeight, buffer + sizeof(PageId), sizeof(int));

    return 0;
}

RC BTreeIndex::write() {

    RC rc;
    memcpy(buffer, &rootPid, sizeof(PageId));
    memcpy(buffer + sizeof(PageId), &treeHeight, sizeof(int));
    if ((rc = pf.write(0, buffer)) < 0) return rc;

    return 0;
}

RC BTreeIndex::print_tree(PageId pid, int level) {

    char* location;
    BTNonLeafNode nonLeaf;
    PageId child_pid;
    int keycount;
    int size_non_pair = sizeof(PageId) + sizeof(int);

    // Print this node
    print_node(pid, level--);
    
    if (level > 0 ) { // Print children
        nonLeaf.read(pid, pf);
        keycount = nonLeaf.getKeyCount();
        location = nonLeaf.buffer + sizeof(int);

        for (int i = 0; i < keycount + 1; i++) {
            memcpy(&child_pid, location, sizeof(PageId));
            
            // child_pid = 1 is the special this for new index file
            if (child_pid > 0)
                print_tree(child_pid, level);

            location += size_non_pair;
        }
    }
    
    return 0;
}

RC BTreeIndex::print_node(PageId pid, int level) {

    char* location;
    int size_non_page = sizeof(PageId) + sizeof(int);
    BTLeafNode leaf;
    BTNonLeafNode nonLeaf;
    PageId child_pid = -1, last_pid = -1;
    RecordId rid;
    
    rid.pid = -2;
    rid.sid = -2;

    int key, keycount, id_size, size_pair = sizeof(int);

    // Print indent
    for (int j = 0; j < treeHeight - level; j++)
        fprintf(stdout, "    ");
    
    if (level > 1) { // Print nonLeaf node
        id_size = sizeof(PageId);
        nonLeaf.read(pid, pf);
        keycount = nonLeaf.getKeyCount();
        location = nonLeaf.buffer + sizeof(int);
        memcpy(&last_pid, location + keycount*size_non_page, sizeof(PageId));
    }
    else { // Print leaf node
        id_size = sizeof(RecordId);
        leaf.read(pid, pf);
        keycount = leaf.getKeyCount();
        location = leaf.buffer + sizeof(int);
        last_pid = leaf.getNextNodePtr();
    }

    size_pair += id_size;

    fprintf(stdout, "%d->[%d] ", pid, keycount);

    for (int i = 0; i < keycount; i++) {    
        memcpy(&key, location + id_size, sizeof(int));
        if (level > 1) {
            memcpy(&child_pid, location, id_size);
            fprintf(stdout, "[%d,%d] ", child_pid, key);
        }
        else {
            memcpy(&rid, location, id_size);
            fprintf(stdout, "[(%d,%d),%d] ", rid.pid, rid.sid, key);
        }
        location += size_pair;
    }
    fprintf(stdout, "[%d]\n", last_pid);
    
    return 0;
}

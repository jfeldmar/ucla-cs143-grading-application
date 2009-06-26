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

#include <iostream>

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
	treeHeight = -1;
	m_debug = false;
}

void BTreeIndex::setDebug(int num)
{
	if (num)
		m_debug = true;
	else
		m_debug = false;
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
	char buffer[PageFile::PAGE_SIZE];
	// is it stored in a page file? yes
	RC rc;
	rc = pf.open(indexname+".idx", mode);
	if (!rc) {
		if (pf.endPid() == 0) {
			// don't have to read anything, junk is fine
			memcpy(buffer+ROOT_LOC, &rootPid, sizeof(int)); // these values will be -1
			memcpy(buffer+HEIGHT_LOC, &treeHeight, sizeof(int));
			rc = pf.write(0, buffer); // endpid should increment here
			if (rc) return rc; // perhaps it wasn't opened for writing
		} else {
			rc = pf.read(0, buffer);
			if (rc) return rc;
			// get the correct rootPid and height
			memcpy(&rootPid, buffer+ROOT_LOC, sizeof(int));
			memcpy(&treeHeight, buffer+HEIGHT_LOC, sizeof(int));
		}
	}
    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	// write everything
	char buffer[PageFile::PAGE_SIZE];
	memcpy(buffer+ROOT_LOC, &rootPid, sizeof(int));
	memcpy(buffer+HEIGHT_LOC, &treeHeight, sizeof(int));
	pf.write(0, buffer);
	m_visitedStack.clear();
	RC rc;
	rc = pf.close();
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
	RC rc;
	// if there are no nodes at all (root is not set)
	if (rootPid == -1) {
		BTLeafNode ln;
		ln.insert(key, rid);
		treeHeight = 1;// height = 1
		if (m_debug) printf("Writing key: %d to pid: %d\n", key, pf.endPid());
		rootPid = pf.endPid(); // set it as the root
		rc = ln.write(pf.endPid(), pf); // endPid increments
	} else {
		// find the leaf node that should contain key
		IndexCursor cursor;
		rc = locateWithRecord(key, cursor); // this will populate the visitedStack
		
		// get parent
		int parent;
		if (!m_visitedStack.isEmpty()) {
			parent = m_visitedStack.top(); // pop stack
			m_visitedStack.pop();
		} else {
			parent = -1;
		}
		insertInLeaf(parent, cursor.pid, key, rid);
		m_visitedStack.clear();
	}
	char buffer[PageFile::PAGE_SIZE];
	memcpy(buffer+ROOT_LOC, &rootPid, sizeof(int));
	memcpy(buffer+HEIGHT_LOC, &treeHeight, sizeof(int));
	pf.write(0, buffer);
	if (m_debug) cout << "TreeHeight: " << treeHeight << endl;
	printNodes();
	if (m_debug) {
		int junk = 1;
		while (cin >> junk)
			if (junk == 0)
				break;
	}
	return 0;
}

RC BTreeIndex::insertInLeaf(PageId parentId, PageId leafNodePid, int key, const RecordId& rid)
{
	BTLeafNode ln;
	
	ln.read(leafNodePid, pf);
	int rc = ln.insert(key, rid);
	if (rc == 0) { // if this works, write and return
		if (m_debug) printf("ITL: Writing key: %d to pid: %d\n", key, leafNodePid);
		ln.write(leafNodePid, pf);
		return 0;
	} else if (rc == RC_NODE_FULL) { // if insert failed, then split
		int sibKey; // the first key in the sibling node after the split
		int newPid; // the new pid
		BTLeafNode lSib; // the sibling node
		
		if (m_debug) printf("ITL: Inserting key: %d, Splitting pid:%d \n", key, leafNodePid);
		ln.insertAndSplit(key, rid, lSib, sibKey);
		newPid = pf.endPid(); // sibling gets a brand new pid
		
		// make sure everything is fine at this level
		// set next node
		lSib.setNextNodePtr(ln.getNextNodePtr()); // paranoia, this should be done in iAS already
		ln.setNextNodePtr(newPid); // order matters! now it points to sibling
		
		// write
		ln.write(leafNodePid, pf);
		lSib.write(newPid, pf);
		
		// get parent : this was done in the previous functionq
		//int parentId;
		//if (!m_visitedStack.isEmpty()) {
		//	parentId = m_visitedStack.top(); // pop stack
		//	m_visitedStack.pop();
		//} else {
		//	parentId = -1;
		//}
		insertInParent(parentId, leafNodePid, sibKey, newPid);
	}
	return 0;
}

RC BTreeIndex::insertInParent(PageId parentId, PageId initialNodePid, int midKey, PageId siblingPid)
{
	BTNonLeafNode nlnParent;
	BTNonLeafNode nlnSib;
	int splitKey;
	int newPid;
	RC rc;
	if (parentId == -1) { // if there's no parent, create a new root node
		nlnParent.setPrevNodePtr(initialNodePid); // create a new node and set it as the root
		nlnParent.insert(midKey, siblingPid);
		newPid = pf.endPid();
		nlnParent.write(newPid, pf);
		treeHeight++; // height++
		rootPid = newPid; // change root pointer
		return 0; // return
	} else { // if there's a parent
		// SECTION 3b: leaf overflow->simple non-leaf insertion
		nlnParent.read(parentId, pf);
		rc = nlnParent.insert(midKey, siblingPid); // it should already have initPid
		if (rc == 0) { // insert to parent succeeded
			if (m_debug) printf("ITP: Writing key: %d to pid: %d\n", midKey, parentId);
			nlnParent.write(parentId, pf);
			return 0;
		} else if (rc == RC_NODE_FULL) { // insert to parent failed
			if (m_debug) printf("ITP: Inserting key: %d, Splitting pid:%d \n", midKey, parentId);
			nlnParent.insertAndSplit(midKey, siblingPid, nlnSib, splitKey); // if fail, insert and split and insert_in_parent
			newPid = pf.endPid();
			nlnParent.write(parentId, pf);
			nlnSib.write(newPid, pf);
			
			// get parent
			int parent_parentId;
			if (!m_visitedStack.isEmpty()) {
				parent_parentId = m_visitedStack.top(); // pop stack
				m_visitedStack.pop();
			} else {
				parent_parentId = -1;
			}
			insertInParent(parent_parentId, parentId, splitKey, newPid);
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
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
	BTNonLeafNode nln;
	BTLeafNode ln;
	char buffer[PageFile::PAGE_SIZE];
	int pid = rootPid;
	int node_type;
	// set C = root node
	pf.read(pid, buffer); // read the root node into the buffer
	memcpy(&node_type, buffer+TYPE_LOC, sizeof(int));
	// node type = rootNode's pid
	if (m_debug) cout << "Node type: " << node_type << endl;
	
	// while C is not a leaf node:
	while (node_type != 1) {
		// keep going until we get to a leaf node
		nln.read(pid, pf);
		if (nln.locateChildPtr(searchKey, pid))
			return RC_NO_SUCH_RECORD; // TODO: see locateChildPtr
		pf.read(pid, buffer);
		memcpy(&node_type, buffer+TYPE_LOC, sizeof(int));
	}
	// if there is a key value Ki in C such that Ki = V
	//   then pointer Pi directs us to the desired record/bucket
	//   else no record with key value k exists
	ln.read(pid, pf);
	int eid;
	cursor.pid = pid;
	if (ln.locate(searchKey, eid)) // perform locate
		return RC_NO_SUCH_RECORD;
	
	// put it in cursor
	cursor.pid = pid;
	cursor.eid = eid;
	
    return 0;
}

// locateWithRecord
//
// This function works the same as locate, except that it keeps track of the nodes visited
RC BTreeIndex::locateWithRecord(int searchKey, IndexCursor& cursor)
{
	BTNonLeafNode nln;
	BTLeafNode ln;
	char buffer[PageFile::PAGE_SIZE];
	int pid = rootPid;
	int node_type;
	// set C = root node
	pf.read(pid, buffer); // read the root node into the buffer
	
	memcpy(&node_type, buffer+TYPE_LOC, sizeof(int));
	// node type = rootNode's pid
	if (m_debug) cout << "Node type: " << node_type << endl;
	
	// while C is not a leaf node:
	while (node_type != 1) {
		// keep going until we get to a leaf node
		nln.read(pid, pf);
		
		// put pid on the stack
		m_visitedStack.push(pid);
		
		if (nln.locateChildPtr(searchKey, pid))
			return RC_NO_SUCH_RECORD; // TODO: see locateChildPtr
		pf.read(pid, buffer);
		memcpy(&node_type, buffer+TYPE_LOC, sizeof(int));
	}
	// if there is a key value Ki in C such that Ki = V
	//   then pointer Pi directs us to the desired record/bucket
	//   else no record with key value k exists
	ln.read(pid, pf);
	int eid;
	cursor.pid = pid;
	if (ln.locate(searchKey, eid)) // perform locate
		return RC_NO_SUCH_RECORD;
	
	// put it in cursor
	cursor.pid = pid;
	cursor.eid = eid;
	
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
	// open the pid containing the leaf node
	BTLeafNode ln;
	ln.read(cursor.pid, pf);
	
	// READ the location specified by the index into key, rid
	if (ln.readEntry(cursor.eid, key, rid))
		return RC_INVALID_CURSOR;
	// key and rid now have the proper values

	if (m_debug) printf("Cursor has read %d, %d:%d\n", key, rid.pid, rid.sid);
	
	// move cursor forward
	if (cursor.eid < ln.getKeyCount() - 1) { // if this is not the last entry in the node
		cursor.eid++;
	} else { // it is the last entry in the node
		if (m_debug) printf("Cursor moving to next node\n");
		cursor.pid = ln.getNextNodePtr();
		if (cursor.pid == RC_END_OF_TREE) return RC_END_OF_TREE;
		cursor.eid = 0;
	}

    return 0;
}

void BTreeIndex::printNodes()
{
	char     page[PageFile::PAGE_SIZE];
    char     nodeType;
    int     entryCount;
    PageId     nextNodePtr;
	BTLeafNode ln;
	BTNonLeafNode nln;
	cout << "RootPid: " << rootPid << endl;
    for(int i = 1; i < pf.endPid() ; i++)
    {
        pf.read(i, page);
        memcpy(&nodeType, page+TYPE_LOC, sizeof(char));
        memcpy(&entryCount, page+NUMKEY_LOC, sizeof(int));
   
        nodeType = nodeType + '0';
   
        if(nodeType == '0')
        {
			memcpy(&nextNodePtr, page+NEXTNODE_LOC, sizeof(int));
            cout<<"pid: " << i << "\t node type: nonLeaf \t" 
			<< "\t entryCount: " << entryCount 
			<< "\t prevNodePtr: " << nextNodePtr << endl;
			nln.read(i, pf);
			nln.print_buffer();
			cout << "========================================" 
			<<  "========================================" << endl;
        }
        else
        {
            memcpy(&nextNodePtr, page+NEXTNODE_LOC, sizeof(int));
            cout<<"pid: " << i << "\t node type: leaf \t"
			<< "\t entryCount: " << entryCount
            <<"\t nextNodePtr: " << nextNodePtr << endl;
			ln.read(i, pf);
			ln.print_buffer();
			cout << "========================================" 
			<<  "========================================" << endl;
		}
	}

}

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

#define CONFIG_DFLAG 0
#define DEBUG if (CONFIG_DFLAG) 

RC insertInto(int pid, int key, const RecordId& rid, bool& childsent, int& childkey);

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
		numnodes = CONFIG_NODEKEYS;
		lastnode = numnodes - 1;
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
		RC rc;
		BTNonLeafNode root, supernode;
		BTLeafNode leaf0, leaf1; // new leaves
		RecordId srid; // temporary, reusable rid
		int skey; // temporary, reusable key

		if ((rc = pf.open(indexname, mode)) < 0) return rc; // error
		
		// is there a superpid in this pagefile?
		int superpid = 0;
		// read alleged supernode
		if ((rc = pf.read(superpid, supernode.nodepage)) < 0) {
						if (mode == 'w') {
										DEBUG fprintf(stdout, "Found no index, creating...\n");
								/* opened in write mode, so we initialize
								 * supernode AND root node and two leaves
								 */
								srid.pid = 0; // this will hold the pid of the root node
								srid.sid = -1; // unlikely sid
								skey = 1010; // marker

								/* write supernode into superblock
								 */
								if ((rc = supernode.writeEntry(0, skey, srid)) < 0) return rc;
								if ((rc = pf.write(superpid, supernode.nodepage)) < 0) return rc;
								supernode.mypid = superpid;
	
								/* initialize root page
								 */
								rootPid = pf.endPid();
								if ((rc = pf.write(rootPid, root.nodepage)) < 0) return rc;
								root.mypid = rootPid;
		
								/* need to get pids for leaves, save, them, create them
								 * and initialize root with them */
		
								// get first pid
								int pid1 = pf.endPid();
								// write out blank leaf to extend file
								if ((rc = pf.write(pid1, leaf0.nodepage)) < 0) return rc;
								leaf0.mypid = pid1;
								leaf0.print();
								
								// get next endPid
								int pid2 = pf.endPid();
								// write out page to extend file
								if ((rc = pf.write(pid2, leaf1.nodepage)) < 0) return rc;
								leaf1.mypid = pid2;
								leaf1.print();
	
								// initialize root with the new pids and key = 0
								if ((rc = root.initializeRoot(pid1, -1010, pid2)) < 0) return rc;
								// write root in
								root.print();
								if ((rc = pf.write(rootPid, root.nodepage)) < 0) return rc;

								// re-write supernode with root node in place
								srid.pid = rootPid; // the pid of the root node
								if ((rc = supernode.writeEntry(0, skey, srid)) < 0) return rc;
								if ((rc = pf.write(superpid, supernode.nodepage)) < 0) return rc;
								supernode.print();
					
						} else if (mode == 'r') {
										/* opened in read mode but no tree so we just fail
										 */
								return RC_FILE_READ_FAILED;
						} else {
										/* opened in bogus mode so we fail
										 */
								return RC_INVALID_FILE_MODE;
						}
		} 
		/* we found a supernode
		 * and we read it into supernode.nodepage above
		 * so now we need to read the root and the two leaves
		 */
		supernode.mypid = superpid;
		if ((rc = supernode.readEntry(0, skey, srid)) < 0) return rc;
			rootPid = srid.pid;
		/* here we check for the special stamp in the superblock
		 * if it is not here, this is not a valid tree!
		 */
		if (skey == 1010) { 
			/* yay, this is a supernode
			 */
			if ((rc = pf.read(rootPid, root.nodepage)) < 0) return rc;
			root.mypid = rootPid;
			root.print();
			if ((rc = pf.read(root.nodepage[0].rid.pid, leaf0.nodepage)) < 0) return rc;
			leaf0.mypid = root.nodepage[0].rid.pid;
			leaf0.print();
			if ((rc = pf.read(root.nodepage[lastnode].rid.pid, leaf1.nodepage)) < 0) return rc;
			leaf1.mypid = root.nodepage[lastnode].rid.pid;
			leaf1.print();
		} else {
			/* boo, not a supernode
			 */
			DEBUG fprintf(stdout, "Invalid index file.\n");
			return RC_INVALID_FILE_FORMAT;
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
		if ((rc = pf.close()) < 0) return rc; // error
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
		RC rc;
		BTNonLeafNode node;
		BTLeafNode leaf;
		int pid, childkey;
		bool childsent = false;

		DEBUG fprintf(stdout, "In insert with key: %d pid: %d sid: %d rootPid: %d\n", key, rid.pid, rid.sid, rootPid);
		
		// load root
		if ((rc = pf.read(rootPid, node.nodepage)) < 0) {
						DEBUG fprintf(stdout, "Couldn't read root node at rootPid: %d\n", rootPid);
						return rc;
		}
		node.mypid = rootPid;
		node.print();

		// insert into tree
		if ((rc = insertInto(rootPid, key, rid, childsent, childkey)) < 0) return rc;

		if (childsent == true) {

						DEBUG fprintf(stdout, "FIXME: I got a key back up to the root!: %d\n", childkey);
		}



    return 0;
}

/*
 * recursively insert a node into the tree; if this causes any node 
 * along the way to split, insert the resulting "midkey" into the
 * parent of the splitting node.
 * @param pid[IN] the node (pid) we are inserting into
 * @param key[IN] the key we are inserting to the leaf
 * @param rid[IN] the rid for the key in the leaf
 * @param childkey[OUT] the resulting childkey to insert into parent
 * 		-- if childkey = '\0' the insertion did not cause a split
 * */

RC BTreeIndex::insertInto(int pid, int key, const RecordId& rid, bool& childsent, int& childkey)
{

		DEBUG fprintf(stdout, "insertInto: target page: %d key: %d\n", pid, key);
		RC rc;
		BTNonLeafNode node, child;
		int my_childkey, childpid;
		bool my_childsent;
		BTLeafNode leaf;

		// read this page (pid)
		if ((rc = pf.read(pid, node.nodepage)) < 0) return rc;
		node.mypid = pid;
		node.print();

		/* if this page is a leaf, skip this part
		 * and insert the key,rid into the leaf below
		 * */
		if (node.nodepage[lastnode].rid.sid != -2) {
						DEBUG fprintf(stdout, "Non-leaf:\n");
		
			/* recursively insert into the interal nodes
			 * until you hit a leaf
			 * */

			// find the appropriate child node for the current key
			if ((rc = node.locateChildPtr(key, childpid)) < 0) return rc;

			// read child
			if ((rc = pf.read(childpid, child.nodepage)) < 0) return rc;
			DEBUG fprintf(stdout, "Following children down, found: %d\n", childpid);
			child.mypid = childpid;
			child.print();
			
			// insert into child with insertInto (recursion)
			my_childsent = false;
			if ((rc = insertInto(childpid, key, rid, my_childsent, my_childkey)) < 0) return rc;
			/* we're now processing return values from the return values
			 * of all the recursions of insertInto. childkey is the key
			 * to the new sibling of the child we inserted into and which
			 * we now must insert into ourselves (and according pass up a 
			 * new childkey if necessary).
			 * */
			if (my_childsent == true) {
							DEBUG fprintf(stdout, "Child returned value: %d ... inserting.\n", my_childkey);
							/* if childkey return value 
							 * is not null, we have a key to push
							 * back up the tree
							 * */

							/* is the parent of the child full?
							 * */
							if (node.nodeIsFull()) {
											/* if so, allocate new page to be the
											 * new sibling of the parent
											 * */
											BTNonLeafNode sibling;
											int midkey;
											
											// insert childkey and split with new sibling
											if ((rc = node.insertAndSplit(my_childkey, 
																										pid, 
																										sibling, 
																										midkey)) < 0) return rc;
											
											// write NEW sibling out to disk
											int sibpid = pf.endPid();
											pf.write(sibpid, sibling.nodepage);
											sibling.mypid = sibpid;
											
											// write updated original page
											pf.write(pid, node.nodepage);


											/* since we split, we need to hand up the pid
											 * of the new sibling
											 * */
											childkey = midkey;
											childsent = true;
											DEBUG fprintf(stdout, "... returning childkey: %d\n", childkey);
											
							return 0;
							} else { // node doesn't need to split

											// directly insert key into node											
											//
											// should be pid for childkey!
											node.insert(my_childkey, pid);
											
											// write updated original page
											pf.write(pid, node.nodepage);

											/* since we did not split, we need to tell
											 * caller we have no childkey to insert
											 * */
											childkey = -13;
											DEBUG fprintf(stdout, "... returning childkey: %d\n", childkey);
							return 0;
							}
							
							return 0;
											
			} // childsent == false

			DEBUG fprintf(stdout, "No child sent. Anything to do?\n");
			if (node.mypid == rootPid) {
							DEBUG fprintf(stdout, "Hi, I'm the root node.\n");
							if ((node.nodepage[0].key == -1010) 
											&& (node.nodepage[lastnode].key == 1)) {
											node.nodepage[0].key = key;
											if ((rc = pf.write(node.mypid, node.nodepage)) < 0) return rc;

							}
							node.print();


			}
			return 0;
		
		} 

		DEBUG fprintf(stdout, "Leaf:\n");
		
		/* the pid in insertInto points to a leaf!
		 * we will insert the key into the leaf, splitting if we have to
		 * if we split, we will pass the pid of the new leaf back up
		 * */
		
		// read the leaf (pid)
		if ((rc = pf.read(pid, leaf.nodepage)) < 0) return rc;
		leaf.mypid = pid;
		leaf.print();
		
		/* if the leaf is full, we need to split it
		 * */

		if (leaf.nodeIsFull()) { // the node is "full"

						// allocate new page for sibling
						BTLeafNode sibling;
						int siblingKey; // the first key of the new sibling
						
						// insert and split with new sibling
						if ((rc = leaf.insertAndSplit(key, rid, sibling, siblingKey)) 
								< 0) return rc;

						// write sibling page out
						int sibpid = pf.endPid();
						sibling.mypid = sibpid;
						if ((rc = pf.write(pf.endPid(), sibling.nodepage)) < 0) return rc; 
						DEBUG fprintf(stdout, "Original table:\n");
						node.print();
						DEBUG fprintf(stdout, "New table:\n");
						sibling.print();

						// write updated original page out
						if ((rc = pf.write(pid, leaf.nodepage)) < 0) return rc;

						/* we split, so we have to return the key to insert
						 * into the parent to our caller
						 * */
						childkey = siblingKey;
						childsent = true;
						DEBUG fprintf(stdout, "... returning childkey: %d\n", childkey);

		} else { // leaf is not full, just insert and be done!

						if ((rc = leaf.insert(key, rid)) < 0) return rc;

						childkey = -13; // nothing to do
						
						// write updated original page
						if ((rc = pf.write(pid, leaf.nodepage)) < 0) return rc;

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
		RC rc;
		BTNonLeafNode node;
		BTLeafNode leaf;
		int pid;

		// load root
		if ((rc = pf.read(rootPid, node.nodepage)) < 0) return rc;
		node.mypid = rootPid;
		node.print();
		while (node.nodepage[lastnode].rid.sid != -2) { // while not a leaf
		
			node.locateChildPtr(searchKey, pid);
			if ((rc = pf.read(pid, node.nodepage)) < 0) return rc;
			node.mypid = pid;
			node.print();
		
		} // found our leaf node
		
		// read leaf
		if ((rc = pf.read(pid, leaf.nodepage)) < 0) return rc;
		leaf.mypid = pid;
		cursor.pid = pid; // leaf pid

		// put eid in cursor
		if ((rc = leaf.locate(searchKey, cursor.eid)) < 0) return rc;

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

		RC rc;
		BTLeafNode leaf;
		if ((rc = pf.read(cursor.pid, leaf.nodepage)) < 0) return rc;
		leaf.mypid = cursor.pid;
		if ((rc = leaf.readEntry(cursor.eid, key, rid)) < 0) return rc;
		if (cursor.eid = lastnode - 2) { // time to switch nodes
			// get pid of next sibling leaf
			cursor.pid = leaf.getNextNodePtr();
		} else { // increment eid
			cursor.eid++;
		}
		return 0;
}

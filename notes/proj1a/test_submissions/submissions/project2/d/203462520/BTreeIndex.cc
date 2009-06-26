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

/* REMOVE ME */

int BTreeIndex::PrintTree()
{
				BTNode node;
				int pid = 0;
				RC rc;
				DEBUG fprintf(stderr, "---------------------------------\n");
				DEBUG fprintf(stderr, "Printing tree:\n");
				while ((rc = pf.read(pid, node.nodepage)) >= 0) {
								node.mypid = pid;
								node.print();
								pid++;
				}
				DEBUG fprintf(stderr, "End of tree.\n");
				DEBUG fprintf(stderr, "---------------------------------\n");

				return 0;

}

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
										DEBUG fprintf(stderr, "Found no index, creating...\n");
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
								int pid2 = pf.endPid();
								// write out blank leaf to extend file
								if ((rc = pf.write(pid2, leaf1.nodepage)) < 0) return rc;
								leaf1.mypid = pid2;
								leaf1.print();
								
								// get next endPid
								int pid1 = pf.endPid();

								// set the NNP for the two leaves
								leaf0.setNextNodePtr(leaf1.mypid);

								// write out page to extend file
								if ((rc = pf.write(pid1, leaf0.nodepage)) < 0) return rc;
								leaf0.mypid = pid1;
								leaf0.print();
								
								// initialize root with the new pids and key = 0
								if ((rc = root.initializeRoot(pid1, -1010, pid2)) < 0) return rc;
								leaf1.print();
								leaf0.print();
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
			DEBUG fprintf(stderr, "Invalid index file.\n");
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
		int pid, childkey, childpage;
		bool childsent = false;

		DEBUG fprintf(stderr, "In insert with key: %d pid: %d sid: %d rootPid: %d\n", key, rid.pid, rid.sid, rootPid);
		
		// load root
		if ((rc = pf.read(rootPid, node.nodepage)) < 0) {
						DEBUG fprintf(stderr, "Couldn't read root node at rootPid: %d\n", rootPid);
						return rc;
		}
		node.mypid = rootPid;
		node.print();

		// insert into tree
		if ((rc = insertInto(rootPid, key, rid, childsent, childkey, childpage)) < 0) return rc;

		if (childsent == true) {

						DEBUG fprintf(stderr, "FIXME: I got a key back up to the SUPERNODE!: %d pid: %d\n", childkey, childpage);

						BTNonLeafNode newroot, supernode;
						int superpid = 0;

						if ((rc = newroot.initializeRoot(rootPid, childkey, childpage)) < 0) return rc;
						rootPid = pf.endPid();
						if ((rc = pf.write(rootPid, newroot.nodepage)) < 0) return rc;
						DEBUG fprintf(stderr, "Initialized new root at pid: %d\n", rootPid);
						if ((rc = pf.read(superpid, supernode.nodepage)) < 0) return rc;
						supernode.nodepage[0].rid.pid = rootPid;
						if ((rc = pf.write(superpid, supernode.nodepage)) < 0) return rc;
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

RC BTreeIndex::insertInto(int pid, int key, const RecordId& rid, bool& childsent, int& childkey, int& childpage)
{

		DEBUG fprintf(stderr, "insertInto: target page: %d key: %d\n", pid, key);
		RC rc;
		BTNonLeafNode node, child;
		int returnedChildkey, returnedChildpage, childpid;
		bool returnedChildsent;
		BTLeafNode leaf;

		// read this page (pid)
		if ((rc = pf.read(pid, node.nodepage)) < 0) return rc;
		node.mypid = pid;
		node.print();

		/* if this page is a leaf, skip this part
		 * and insert the key,rid into the leaf below
		 * */
		if (node.nodepage[lastnode].rid.sid != -2) {
						DEBUG fprintf(stderr, "Non-leaf:\n");
		
			/* recursively insert into the interal nodes
			 * until you hit a leaf
			 * */

			// find the appropriate child node for the current key
			if ((rc = node.locateChildPtr(key, childpid)) < 0) return rc;

			// read child
			if ((rc = pf.read(childpid, child.nodepage)) < 0) return rc;
			DEBUG fprintf(stderr, "Following children down, found: %d\n", childpid);
			child.mypid = childpid;
			
			// insert into child with insertInto (recursion)
			returnedChildsent = false;
			if ((rc = insertInto(childpid, key, rid,  returnedChildsent,  returnedChildkey,  returnedChildpage)) < 0) return rc;
			/* we're now processing return values from the return values
			 * of all the recursions of insertInto. childkey is the key
			 * to the new sibling of the child we inserted into and which
			 * we now must insert into ourselves (and according pass up a 
			 * new childkey if necessary).
			 * */
			if (returnedChildsent == true) {
							DEBUG fprintf(stderr, "Child returned key: %d, pid: %d  ... inserting.\n", returnedChildkey, returnedChildpage);
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
											if ((rc = node.insertAndSplit(returnedChildkey, 
																										returnedChildpage, 
																										sibling, 
																										midkey)) < 0) return rc;
											
											// write NEW sibling out to disk
											int sibpid = pf.endPid();
											sibling.mypid = sibpid;
											pf.write(sibling.mypid, sibling.nodepage);
											
											// write updated original page
											pf.write(pid, node.nodepage);


											/* since we split, we need to hand up the pid
											 * of the new sibling
											 * */
											childkey = midkey;
											childpage = sibling.mypid;
											childsent = true;
											DEBUG fprintf(stderr, "... returning returnedChildkey: %d, page %d, sent: %b\n", returnedChildkey, returnedChildpage, returnedChildsent);
											return 0;
							} else { // node doesn't need to split

											// directly insert key into node											
											//
											// should be pid for childkey!
											node.insert(returnedChildkey, returnedChildpage);
											
											// write updated original page
											pf.write(pid, node.nodepage);

											/* since we did not split, we need to tell
											 * caller we have no childkey to insert
											 * */
											childkey = -13;
											childpage = -13;
											childsent = false;
											DEBUG fprintf(stderr, "Ending recursion.\n");
											//DEBUG fprintf(stderr, "... returning childkey: %d\n", childkey);
											return 0;
							}
				
							DEBUG fprintf(stderr, "FALLING OUT!\n");
							return 0;
											
			} // childsent == false

			DEBUG fprintf(stderr, "No child sent.\n");
			if ((node.mypid == rootPid) && (node.getKeyCount() == 1)) {
							DEBUG fprintf(stderr, "Setting FIRST KEY in new root.\n");
							if ((node.nodepage[0].key == -1010) 
											&& (node.nodepage[lastnode].key == 1)) {
											node.nodepage[0].key = key;
											if ((rc = pf.write(node.mypid, node.nodepage)) < 0) return rc;

							}
							node.print();


			}

			childkey = -13;
			childpage = -13;
			childsent = false;
			return 0;
		
		} 

		
		/* the pid in insertInto points to a leaf!
		 * we will insert the key into the leaf, splitting if we have to
		 * if we split, we will pass the pid of the new leaf back up
		 * */
		
		// read the leaf (pid)
		if ((rc = pf.read(pid, leaf.nodepage)) < 0) return rc;
		leaf.mypid = pid;
		
		DEBUG fprintf(stderr, "WE HAVE Leaf-TOFF for leaf: %d\n", leaf.mypid);

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
						/* new next sibling pointer is my current next ptr
						 */
						//sibling.setNextNodePtr(leaf.getNextNodePtr());
						/* set new leaf next ptr to new sibling pid
						 */
						DEBUG fprintf(stderr, "Setting updated NNP for pid %d to sibpid %d\n", leaf.mypid, sibling.mypid);
						leaf.setNextNodePtr(sibling.mypid);

						/* write sibling out to disk */
						DEBUG fprintf(stderr, "Writing sibling into pid: %d\n", sibling.mypid);
						if ((rc = pf.write(sibling.mypid, sibling.nodepage)) < 0) return rc; 
						// write updated original page out
						if ((rc = pf.write(pid, leaf.nodepage)) < 0) return rc;

						DEBUG fprintf(stderr, "Original node:\n");
						leaf.print();
						DEBUG fprintf(stderr, "New node:\n");
						sibling.print();
						DEBUG fprintf(stderr, "endpid: %d\n", pf.endPid());

						/* we split, so we have to return the key to insert
						 * into the parent to our caller
						 * */
						childsent = true;
						childkey = siblingKey;
						childpage = sibling.mypid;
						DEBUG fprintf(stderr, "... returning childkey: %d, childpage: %d\n", childkey, childpage);

		} else { // leaf is not full, just insert and be done!

						if ((rc = leaf.insert(key, rid)) < 0) return rc;

						childkey = -13; // nothing to do
						childpage = -13;
						childsent = false;
						
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

		//fprintf(stderr, "reading forward pid: %d, eid: %d ... ", cursor.pid, cursor.eid);
		RC rc;
		BTLeafNode leaf;
		if ((rc = pf.read(cursor.pid, leaf.nodepage)) < 0) return rc;
		leaf.mypid = cursor.pid;
		if ((rc = leaf.readEntry(cursor.eid, key, rid)) < 0) return rc;
		int numkeys = leaf.getKeyCount();
		if ((numkeys < 1) || // no keys; skip this leaf
				(cursor.eid >= numkeys) || // past end of valid entries
				(cursor.eid == lastnode - 1)) { // time to switch nodes
			// get pid of next sibling leaf
			int nextleaf = leaf.getNextNodePtr();

			if (nextleaf == 0) {
							// this is the end of the tree
							return RC_END_OF_TREE;
			}
			
			// assign cursor.pid to next leaf value
			cursor.pid = nextleaf;
			cursor.eid = 0;
			readForward(cursor, key, rid);
			return 0;
		} else { // next entry in this leaf
			// set found values and return
			key = leaf.nodepage[cursor.eid].key;
			rid.pid = leaf.nodepage[cursor.eid].rid.pid;
			rid.sid = leaf.nodepage[cursor.eid].rid.sid;
			cursor.eid++;
		}
		return 0;
}

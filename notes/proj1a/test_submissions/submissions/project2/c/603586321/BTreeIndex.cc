/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include "BTreeIndex.h"

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
RC BTreeIndex::open(const string& indexname, char mode)
{
	int pf_error;

	// Check if file exists
	bool file_exists = true;
	if(!fopen(indexname.c_str(), "r")) {
		file_exists = false;
	}

	// Open the page file
	pf_error = pf.open(indexname, mode);
	if(pf_error != 0) {
		return pf_error;
	}

	// Load root pid and tree height
	if (file_exists) {
		root_page rp;
		pf.read(0, &rp);
		rootPid = rp.root_pid;
		treeHeight = rp.tree_height;
	} else {
		rootPid = -1;
		treeHeight = -1;
	}
	return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
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
	// Create a B+-Tree is it doesn't already exists
	if(treeHeight == -1) {
		create_tree(key, rid);
		return 0;
	}

	// Find the leaf node that should contain key
	PageId path[treeHeight];
	path_to_leaf(key, path);
	BTLeafNode leaf_node;
	PageId leaf_pid = path[treeHeight-1];
	leaf_node.read(leaf_pid, pf);

	// Try inserting new key
	if(leaf_node.insert(key, rid) != 0) {
		// Leaf node is full, split it

		// Create sibling node and store half the keys in each leaf node
		BTLeafNode sibling;
		PageId sibling_pid = pf.endPid();

		int siblingKey;
		leaf_node.insertAndSplit(key, rid, sibling, sibling_pid, siblingKey);
		sibling.write(sibling_pid, pf);

		// Insert key & pointer to new sibling node in parent node
		insert_in_parent(path, treeHeight-2, siblingKey, sibling_pid);
	}

	// Store leaf node
	leaf_node.write(leaf_pid, pf);

    return 0;
}

RC BTreeIndex::insert_in_parent(PageId path[], int parent_pos, int new_key, PageId new_pid)
{
	// Check if we need a new root
	if(parent_pos == -1) {
		// Get the pid for the old root
		PageId old_root_pid = path[0];

		// Add new root
		BTNonLeafNode root_node;
		rootPid = pf.endPid();
		treeHeight++;
		root_node.initializeRoot(old_root_pid, new_key, new_pid);
		root_node.write(rootPid, pf);

		// Write the new root pid and tree height
		root_page rp;
		rp.root_pid = rootPid;
		rp.tree_height = treeHeight;
		pf.write(0, &rp);

		return 0;
	}

	// Get parent node
	BTNonLeafNode parent_node;
	PageId parent_pid = path[parent_pos];
	parent_node.read(parent_pid, pf);

	// Try inserting new key
	if(parent_node.insert(new_key, new_pid) != 0) {
		// Parent node is full, split it

		// Create sibling node and store half the keys in each parent node
		BTNonLeafNode sibling;
		PageId sibling_pid = pf.endPid();
		int mid_key;
		parent_node.insertAndSplit(new_key, new_pid, sibling, mid_key);
		sibling.write(sibling_pid, pf);

		// Insert the middle key & pointer to new sibling node in parent node
		insert_in_parent(path, parent_pos-1, mid_key, sibling_pid);
	}

	// Store parent node
	parent_node.write(parent_pid, pf);

	return 0;
}

RC BTreeIndex::create_tree(int key, const RecordId& rid)
{
	/* Create and initialize a root node */

	// Create a page to store the root pid and tree height
	root_page rp;
	pf.write(0, &rp);

	// Create the left node
	BTLeafNode left_node;
	PageId left_pid = pf.endPid();
	left_node.write(left_pid, pf);

	// Create the right node
	BTLeafNode right_node;
	right_node.insert(key, rid);
	PageId right_pid = pf.endPid();
	right_node.write(right_pid, pf);

	// Set nodes as siblings
	left_node.setNextNodePtr(right_pid);
	left_node.write(left_pid, pf);

	// Create the root node
	BTNonLeafNode root_node;
	rootPid = pf.endPid();
	treeHeight = 2;
	root_node.initializeRoot(left_pid, key, right_pid);
	root_node.write(rootPid, pf);

	// Write the new root pid and tree height
	rp.root_pid = rootPid;
	rp.tree_height = treeHeight;
	pf.write(0, &rp);

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
	BTLeafNode leaf_node;
	BTNonLeafNode parent_node;
	PageId leaf_pid;
	PageId parent_pid;
	PageId path[treeHeight];
	path_to_leaf(searchKey, path);
	cursor.pid = path[treeHeight-1];
	leaf_node.read(cursor.pid, pf);
	leaf_node.locate(searchKey, cursor.eid);

	// Print Node
	//leaf_node.print();

    return 0;
}

RC BTreeIndex::path_to_leaf(int searchKey, PageId path[]) {

	// Store the pids on the path to the leaf node that should contain searchKey
	BTNonLeafNode current_node;
	path[0] = rootPid;
	for(int i=1; i<treeHeight; i++) {
		current_node.read(path[i-1], pf);
		current_node.locateChildPtr(searchKey, path[i]);
	}

	return 0;
}
/*
RC BTreeIndex::print()
{
	// Get root node
	fprintf(stdout, "** ROOT NODE **\n");
	BTNonLeafNode root_node;
	root_node.read(rootPid, pf);
	// Print tree
	root_node.print(pf, treeHeight, 1);

	return 0;
}
*/
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

#include "BTreeNode.h"
#include <math.h>

using namespace std;

BTLeafNode::BTLeafNode()
{
  lns.key_count = 0;
}

/*
 * Read the content of the node into buffer
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf) {
	void * buffer = &lns;
	return pf.read(pid, buffer);
}

/*
 * Write the content of the node in buffer
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf) {
	void * buffer = &lns;
	return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount() {
	return lns.key_count;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid) {
	// Check if the node is full
	if (lns.key_count == MAX_LEAF_KEYS) {
		return RC_NODE_FULL;
	}

	// Find location in keys array to store new key
	int insert_loc = lns.key_count;
	for(int i = 0; i < lns.key_count; i++) {
		if (key < lns.keys[i]) {
			insert_loc = i;
			break;
		}
	}

	// Shift all key, rid pairs with key values greater than new key to the right
	for(int i = lns.key_count; i > insert_loc; i--) {
		lns.keys[i] = lns.keys[i-1];
		lns.rids[i].pid = lns.rids[i-1].pid;
		lns.rids[i].sid = lns.rids[i-1].sid;
	}

	// Insert new key, rid pair into the arrays
	lns.keys[insert_loc] = key;
	lns.rids[insert_loc].pid = rid.pid;
	lns.rids[insert_loc].sid = rid.sid;

	lns.key_count++;

	return 0;
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, BTLeafNode& sibling, PageId sibling_pid, int& siblingKey) {
	// Set the new siblings to point to the current nodes sibling
	sibling.lns.sibling = lns.sibling;

	// Set the current node to point to the new sibling
	lns.sibling = sibling_pid;

	// Create temp key and rid arrays
	int temp_keys[MAX_LEAF_KEYS+1];
	RecordId temp_rids[MAX_LEAF_KEYS+1];

	// Copy	node key, rid pairs to temp arrays
	for(int i = 0; i < MAX_LEAF_KEYS; i++) {
		temp_keys[i] = lns.keys[i];
		temp_rids[i].pid = lns.rids[i].pid;
		temp_rids[i].sid = lns.rids[i].sid;
	}

	// insert new key, rid pair to temp array
	int insert_loc = MAX_LEAF_KEYS;
	for(int i = 0; i < MAX_LEAF_KEYS; i++) {
		if (key < temp_keys[i]) {
			insert_loc = i;
			break;
		}
	}
	for(int i = MAX_LEAF_KEYS; i > insert_loc; i--) {
		temp_keys[i] = temp_keys[i-1];
		temp_rids[i].pid = temp_rids[i-1].pid;
		temp_rids[i].sid = temp_rids[i-1].sid;
	}
	temp_keys[insert_loc] = key;
	temp_rids[insert_loc].pid = rid.pid;
	temp_rids[insert_loc].sid = rid.sid;

	// Transfer first half of the key, rid pairs to node
	int num_keys_node = (int)ceil((MAX_LEAF_KEYS + 1.0) / 2.0);
	for (int i = 0; i < num_keys_node; i++) {
		lns.keys[i] = temp_keys[i];
		lns.rids[i].pid = temp_rids[i].pid;
		lns.rids[i].sid = temp_rids[i].sid;
	}
	lns.key_count = num_keys_node;

	// Transfer second half of the key, rid pairs to sibling
	int num_keys_sibling = (MAX_LEAF_KEYS + 1) - num_keys_node;
	for (int i = 0; i < num_keys_sibling; i++) {
		sibling.lns.keys[i] = temp_keys[num_keys_node+i];
		sibling.lns.rids[i].pid = temp_rids[num_keys_node+i].pid;
		sibling.lns.rids[i].sid = temp_rids[num_keys_node+i].sid;
	}
	sibling.lns.key_count = num_keys_sibling;

	// Return the first key of the sibling
	siblingKey = sibling.lns.keys[0];

	return 0;
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid) {

	for(int i = 0; i < lns.key_count; i++) {
		if (searchKey >= lns.keys[i]) {
			eid = i;
			return 0;
		}
	}

	return RC_NO_SUCH_RECORD;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid) {

	key = lns.keys[eid];
	rid.pid = lns.rids[eid].pid;
	rid.sid = lns.rids[eid].sid;

	return 0;
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr() {
	return lns.sibling;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid) {
	lns.sibling = pid;
	return 0;
}

/***************************************** NON LEAF NODE FUNCTIONS *****************************************/

BTNonLeafNode::BTNonLeafNode() {
	nlns.key_count = 0;
}

/*
 * Read the content of the node into buffer
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf) {
	void * buffer = &nlns;
	return pf.read(pid, buffer);
}

/*
 * Write the content of the node in buffer
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf) {
	void * buffer = &nlns;
	return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount() {
	return nlns.key_count;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid) {
	// Check if the node is full
	if (nlns.key_count == MAX_NONLEAF_KEYS) {
		return RC_NODE_FULL;
	}

	// Find location in keys array to store new key
	int insert_loc = nlns.key_count;
	for(int i = 0; i < nlns.key_count; i++) {
		if (key < nlns.keys[i]) {
			insert_loc = i;
			break;
		}
	}

	// Shift all key, rid pairs with key values greater than new key to the right
	for(int i = nlns.key_count; i > insert_loc; i--) {
		nlns.keys[i] = nlns.keys[i-1];
		nlns.pids[i+1] = nlns.pids[i];
	}

	// Insert new key, rid pair into the arrays
	nlns.keys[insert_loc] = key;
	nlns.pids[insert_loc+1] = pid;

	nlns.key_count++;

	return 0;
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey) {
	// Create temp key and rid arrays
	int temp_keys[MAX_NONLEAF_KEYS+1];
	PageId temp_pids[MAX_NONLEAF_KEYS+2];

	// Copy	node key, rid pairs to temp arrays
	for(int i = 0; i < MAX_NONLEAF_KEYS; i++) {
		temp_keys[i] = nlns.keys[i];
		temp_pids[i] = nlns.pids[i];
	}
	temp_pids[MAX_NONLEAF_KEYS] = nlns.pids[MAX_NONLEAF_KEYS];

	// insert new key, rid pair to temp array
	int insert_loc = MAX_NONLEAF_KEYS;
	for(int i = 0; i < MAX_NONLEAF_KEYS; i++) {
		if (key < temp_keys[i]) {
			insert_loc = i;
			break;
		}
	}
	for(int i = MAX_NONLEAF_KEYS; i > insert_loc; i--) {
		temp_keys[i] = temp_keys[i-1];
		temp_pids[i+1] = temp_pids[i];
	}
	temp_keys[insert_loc] = key;
	temp_pids[insert_loc+1] = pid;

	// Transfer first half of the key, rid pairs to node
	int mid_keys_node = (int)ceil((MAX_NONLEAF_KEYS + 1.0) / 2.0);
	int num_keys_node = mid_keys_node - 1;
	for (int i = 0; i < num_keys_node; i++) {
		nlns.keys[i] = temp_keys[i];
		nlns.pids[i] = temp_pids[i];
	}
	nlns.pids[num_keys_node] = temp_pids[num_keys_node];
	nlns.key_count = num_keys_node;

	// Transfer second half of the key, rid pairs to sibling
	int num_keys_sibling = (MAX_NONLEAF_KEYS + 1) - mid_keys_node;
	for (int i = 0; i < num_keys_sibling; i++) {
		sibling.nlns.keys[i] = temp_keys[mid_keys_node+i];
		sibling.nlns.pids[i] = temp_pids[mid_keys_node+i];
	}
	sibling.nlns.pids[num_keys_sibling] = temp_pids[mid_keys_node+num_keys_sibling];
	sibling.nlns.key_count = num_keys_sibling;

	// Return the first key of the sibling
	midKey = temp_keys[mid_keys_node-1];

	return 0;
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid) {

	for(int i = 0; i < nlns.key_count; i++) {
		if (searchKey < nlns.keys[i]) {
			pid = nlns.pids[i];
			return 0;
		}
	}

	pid = nlns.pids[nlns.key_count];
	return 0;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2) {
	nlns.keys[0] = key;
	nlns.pids[0] = pid1;
	nlns.pids[1] = pid2;
	nlns.key_count = 1;

	return 0;
}


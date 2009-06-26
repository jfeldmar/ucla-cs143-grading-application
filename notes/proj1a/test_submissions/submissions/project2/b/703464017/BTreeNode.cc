#include "BTreeNode.h"
#include <math.h>


using namespace std;

/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf) {
//    char* page[PageFile::PAGE_SIZE];
    pf.read( pid, buffer );
    return 0;
}
    
/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf) {
//    char* page[PageFile::PAGE_SIZE];
	pf.write( pid, buffer );
    return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount() {
    int num_key;
    memcpy( &num_key, buffer, sizeof(int) );
    return num_key;
}

/**
* Set the number of keys stored in the node.
* @return the number of keys in the node
*/
int BTLeafNode::setKeyCount(const int& num_key) {
    memcpy( buffer, &num_key, sizeof(int) );
    return num_key;
}


/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid) {

    // We don't need to worry about whether there is enough space because
    // the checking is done before this function called

    int eid;
    int num_key_moved;
    int size_pair = sizeof(RecordId) + sizeof(int);
    char* mybuffer[PageFile::PAGE_SIZE] = {0};

    locate( key, eid );
    BTLeafNode* location = buffer + sizeof(int) + eid*size_pair;
    num_key_moved = getKeyCount() - eid;

    // Move all keys that greater than the insert key to right
    memcpy( mybuffer, location, num_key_moved*size_pair );
    memcpy( location + size_pair, mybuffer, num_key_moved*size_pair );
    
    // Insert the key and rid
    memcpy( location, &rid, sizeof(RecordId) );
    memcpy( location + sizeof(RecordId), &key, sizeof(int) );

    // Update the key count
    setKeyCount( getKeyCount() + 1 );
    
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
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& L_prime, int& midKey)
{
	 PageId tmp_pid;
	 
    int size_pair = sizeof(int) + sizeof(RecordId);
    
    // tmp_eid is the "middle" eid, where we should split
    int tmp_eid = (int) ceil(NUM_LEAF_PTRS/2);

    // Since the node is full, it must contain the max # of keys
    int num_key_move = NUM_LEAF_PTRS - tmp_eid - 1;

    // For determining whether to insert into L or L_prime;
    int pre_v_prime;

    // ----- Determine v' -----

    // tmp_ptr -> ceil(n/2);
    BTLeafNode* tmp_ptr = buffer + sizeof(int) + tmp_eid*(size_pair);
    
    //v_prime -> ceil(n/2) - 1;
    BTLeafNode* v_prime = tmp_ptr - size_pair;
    memcpy(&pre_v_prime, v_prime + sizeof(RecordId), sizeof(int));


    //if (v < v_prime)
    if (memcmp(&key, v_prime + sizeof(RecordId), sizeof(int)) >= 0) {
        v_prime = tmp_ptr;

        // we will move 1 less pair, so update the # of keys to move
        num_key_move -= 1;
    }
    else
        // update this tmp_eid for set the correct # of keys in L
        tmp_eid -= 1;

    // ----- Move keys from Leaf L to sibling L_prime -----

    memcpy(&L_prime + sizeof(int), v_prime, num_key_move*size_pair);

    // update the key count of L so that is not full anymore
    setKeyCount(tmp_eid);
    // update the key count of L_prime also
    L_prime.setKeyCount(num_key_move);

    if (key < pre_v_prime)
        insert(key, rid);
    else
        L_prime.insert(key, rid);

    memcpy(&midKey, &L_prime + sizeof(int) + sizeof(RecordId), sizeof(int));
    
    
    getPid(tmp_pid);
    L_prime.setPid(tmp_pid);
    
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
    BTLeafNode* leaf = buffer + sizeof(int);
    int size_pair = sizeof(RecordId) + sizeof(int);

    // "Initialize" the eid
    eid = 0;

    // concept: while (searchKey >= pid)
    while (memcmp(&searchKey, leaf + sizeof(RecordId), sizeof(int)) >= 0) {
        leaf += size_pair;
        eid++;
    }

    // eid is updated every loop above, so at this point, we've already "output"
    // the eid and we done :)

    return 0;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid) {
    if (eid >= 0 && eid < MAX_NODE_PTRS) {
        BTLeafNode* ptr = buffer + sizeof(int) + eid*(sizeof(RecordId) + sizeof(int));
        memcpy(&rid, ptr, sizeof(RecordId));
        memcpy(&key, ptr + sizeof(RecordId), sizeof(int));
        return 0;
    }
    else
        return RC_INVALID_ATTRIBUTE;
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr() {
    PageId pid;
    memcpy(&pid, buffer + PageFile::PAGE_SIZE - sizeof(pid), sizeof(pid));
    return pid;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid) {
    if (pid > 0) {
        memcpy(buffer + PageFile::PAGE_SIZE - sizeof(pid), &pid, sizeof(pid));
        return 0;
    }
    else
        return RC_INVALID_PID;
}

/**
* Check whether th node is full
* @return true if the node is full
*/
bool BTLeafNode::IsFull() {
    int max = NUM_LEAF_PTRS - 1;
    return memcmp(buffer, &max, sizeof(int)) < 0 ? false : true;
}

/**
* Set the last pid pointer
* @return 0 if successful. Return an error code if there is an error.
*/
RC BTLeafNode::setPid(const PageId& pid) {
    memcpy(buffer + PageFile::PAGE_SIZE - sizeof(PageId), &pid, sizeof(PageId));
    return 0;
}

/**
* Get the last pid pointer
* @param pid[OUT] the PageId to retrieve
* @return 0 if successful. Return an error code if there is an error.
*/
RC BTLeafNode::getPid(PageId& pid) {
    memcpy(&pid, buffer + PageFile::PAGE_SIZE - sizeof(PageId), sizeof(PageId));
    return 0;
}








/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ return 0; }
    
/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ return 0; }

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount() {
    int num_key;
    memcpy(&num_key, buffer, sizeof(int));
    return num_key;
}

/**
* Set the number of keys stored in the node.
* @return the number of keys in the node
*/
int BTNonLeafNode::setKeyCount(const int& num_key) {
    memcpy(buffer, &num_key, sizeof(int));
    return num_key;
}

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid) {

    // We don't need to worry about whether there is enough space because
    // the checking is done before this function called

    int eid;
    int num_key_moved;
    int size_non_pair = sizeof(PageId) + sizeof(int);
    char* mybuffer[PageFile::PAGE_SIZE] = {0};
    
    locate(key, eid);
    BTNonLeafNode* location = buffer + sizeof(int) + eid*size_non_pair;
    num_key_moved = getKeyCount() - eid;

    // Move all keys that greater than the insert key to right
    memcpy(mybuffer, location, num_key_moved*size_non_pair);
    memcpy(location + size_non_pair, mybuffer, num_key_moved*size_non_pair);
    
    // Insert the key and pid
    memcpy(location, &pid, sizeof(PageId));
    memcpy(location + sizeof(PageId), &key, sizeof(int));

    // Update the key count
    setKeyCount( getKeyCount() + 1 );
    
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
RC BTNonLeafNode::insertAndSplit(int key, const PageId& pid,
                                 BTNonLeafNode& L_prime, int& midKey) {
    int size_non_pair = sizeof(int) + sizeof(PageId);
    
    // tmp_eid is the "middle" eid, where we should split
    int tmp_eid = (int) ceil(NUM_NONLEAF_PTRS/2);

    // Since the node is full, it must contain the max # of keys
    int num_key_move = NUM_NONLEAF_PTRS - tmp_eid - 1;

    // For determining whether to insert into L or L_prime;
    int pre_v_prime;

    // ----- Determine v' -----

    // tmp_ptr -> ceil(n/2);
    BTNonLeafNode* tmp_ptr = buffer + sizeof(int) + tmp_eid*(size_non_pair);
    
    //v_prime -> ceil(n/2) - 1;
    BTNonLeafNode* v_prime = tmp_ptr - size_non_pair;
    memcpy(&pre_v_prime, v_prime + sizeof(PageId), sizeof(int));


    //if (v < v_prime)
    if (memcmp(&key, v_prime + sizeof(PageId), sizeof(int)) >= 0) {
        v_prime = tmp_ptr;

        // we will move 1 less pair, so update the # of keys to move
        num_key_move -= 1;
    }
    else
        // update this tmp_eid for set the correct # of keys in L
        tmp_eid -= 1;

    // ----- Move keys from Leaf L to sibling L_prime -----

    memcpy(&L_prime + sizeof(int), v_prime, num_key_move*size_non_pair);

    // update the key count of L so that is not full anymore
    setKeyCount(tmp_eid);
    // update the key count of L_prime also
    L_prime.setKeyCount(num_key_move);

    if (key < pre_v_prime)
        insert(key, pid);
    else
        L_prime.insert(key, pid);

    memcpy(&midKey, &L_prime + sizeof(int) + sizeof(PageId), sizeof(int));
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid) {
    int tempPid;
    BTNonLeafNode* nonLeaf = buffer + sizeof(int); // Skip the first INT
    int size_non_pair = sizeof(PageId) + sizeof(int);

    // concept: while (searchKey >= pid)
    while (memcmp(&searchKey, nonLeaf + sizeof(PageId), sizeof(int)) >= 0)
            nonLeaf += size_non_pair;

    memcpy(&pid, nonLeaf, sizeof(PageId));

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
	if (pid1 > 0 && pid2 > 0) {
		// Set # of key in root to 1
		buffer->setKeyCount( 1 );
		
		// Set the "left pointer" points to the old root
		memcpy( buffer + sizeof(int), &pid1, sizeof(int) );
		
		// Insert the only key for the root
		memcpy( buffer + 2*sizeof(int), &key, sizeof(int) );
		
		// Set the "right pointer" points to the sibling of the old root
		memcpy( buffer + 3*sizeof(int), &pid2, sizeof(int) );
	}
	else
		return RC_INVALID_PID;
}

/**
* Check whether th node is full
* @return true if the node is full
*/
bool BTNonLeafNode::IsFull() {
    int max = NUM_LEAF_PTRS - 1;
    return memcmp(buffer, &max, sizeof(int)) < 0 ? false : true;
}

RC BTNonLeafNode::locate(int searchKey, int& eid) {
    int tempEid = 0;
    BTNonLeafNode* leaf = buffer + sizeof(int);

    while(leaf != 0) {
        // Assume no "int" at the begining
        // concept: if (searchKey >= pid)
        if (memcmp(&searchKey, leaf + sizeof(PageId), sizeof(int)) > 0) {
            leaf += (sizeof(PageId) + sizeof(searchKey));
            tempEid++;
        }
        else
            break;
    }

    eid = tempEid;
    return 0;
}

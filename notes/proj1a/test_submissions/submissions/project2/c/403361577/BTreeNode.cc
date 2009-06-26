#include "BTreeNode.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// LEAF //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// leaf node constructor
BTLeafNode::BTLeafNode() {
    clearBuffer();
}

/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{
    // read the content of pid page into buffer
    if (pf.read(pid, &buffer) == 0)
        return 0;
    // return -1 on error
    return -1;
}
    
/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{
    // write the content in buffer to the pid page
    if (pf.write(pid, &buffer) == 0)
        return 0;
    // return -1 on error
    return -1;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
    int keycount;
    // get the key count from the first 4 bytes
    // since the first thing stored in each page will
    // be the key count
  	memcpy(&keycount, buffer, sizeof(int));
  	// return this result
  	return keycount;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
    // get the key count
    int keycount = getKeyCount();
    
    // return error code if the node is full
    if (keycount >= MAX_KEY_PER_PAGE)
      return -1;
    
    // insert the key, rid pair into the node
    // should always be successful
    if (insertLeafKey(buffer, keycount, key, rid) != 0)
      return -1;
    
    // increment the key count
    keycount++;
    
    // update the total key count in buffer
    memcpy(buffer, &keycount, sizeof(int));
    
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
                              BTLeafNode& sibling, int& siblingKey)
{
    // return error if sibling is not empty
    if (sibling.getKeyCount() != 0)
        return -1;

    // get the key count
    int keycount = getKeyCount();

    // insert the key, rid pair into the node
    insertLeafKey(buffer, keycount, key, rid);
    
    // increment the key count
    keycount++;
    
    // number of keys to split
    int halfkeycount = keycount / 2;
    
    // set p to point to the beginning of the splitting point
    char *p = buffer + sizeof(int) + halfkeycount*3*sizeof(int);
    
    // sibling buffer pointer
    char *sibbuf = sibling.buffer + sizeof(int);
    
    // set sibling key
    memcpy(&siblingKey, p, sizeof(int));
    
    for (int i = halfkeycount; i < keycount; i++) {
        // copy contents of this buffer to sibling's buffer
        memcpy(sibbuf, p, 3*sizeof(int));
        // set contents of this buffer to 0
        memset(p, 0, 3*sizeof(int));
        // advance pointers
        p += 3*sizeof(int);
        sibbuf += 3*sizeof(int);
    }
    
    // set new key count for this buffer
    int sibkeycount = keycount - halfkeycount;
    
    // update the total key count in buffers
    memcpy(buffer, &halfkeycount, sizeof(int));
    memcpy(sibling.buffer, &sibkeycount, sizeof(int));

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
RC BTLeafNode::locate(int searchKey, int& eid)
{
    for (int i = 0; i < getKeyCount(); i++) {
        // set pointer to point past the first 4 bytes
        // since our key count is stored there
        char *p = buffer + sizeof(int);
        // update p to point to the right pair
        p += i*(3*sizeof(int));
            
        int thiskey;
        // get this key
        memcpy(&thiskey, p, sizeof(int));
        
        if (thiskey >= searchKey) {
            i++;
            memcpy(&eid, &i, sizeof(int));
            return 0;
        }
    }
    
    return -1;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{
    // return error on invalid entry
    if (eid < 1 || eid > MAX_KEY_PER_PAGE)
        return -1;
    // set pointer to point to the eid
    char *p = buffer + sizeof(int) + (eid-1)*3*sizeof(int);
    // get the key
    memcpy(&key, p, sizeof(int));
    // update pointer to point to pid
    p += sizeof(int);
    // get the pid
    memcpy(&rid.pid, p, sizeof(int));
    // update pointer to point to sid
    p += sizeof(int);
    // get the sid
    memcpy(&rid.sid, p, sizeof(int));
    return 0;
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
    int pid;
    // get the pid of the next sibling node
    memcpy(&pid, buffer + 1024 - sizeof(int), sizeof(int));
    // return the pid
    return pid;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
    // copy the pid to the last four bytes of the buffer
    memcpy(buffer + 1024 - sizeof(int), &pid, sizeof(int));
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// NON-LEAF //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// non-leaf node constructor
BTNonLeafNode::BTNonLeafNode() {
    clearBuffer();
}

/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ 
    // read the content of pid page into buffer
    if (pf.read(pid, &buffer) == 0)
        return 0;
    // return -1 on error
    return -1;
}
    
/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{
    // write the content in buffer to the pid page
    if (pf.write(pid, &buffer) == 0)
        return 0;
    // return -1 on error
    return -1;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
    int keycount;
    // get the key count from the first 4 bytes
    // since the first thing stored in each page will
    // be the key count
  	memcpy(&keycount, buffer, sizeof(int));
  	// return this result
  	return keycount;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
    // get the key count
    int keycount = getKeyCount();
    
    // return error code if the node is full
    if (keycount >= MAX_KEY_PER_PAGE)
      return -1;
    
    // insert the key, rid pair into the node
    // make sure that we only insert when the nonleaf node has been created
    if (insertNonLeafKey(buffer, keycount, key, pid) != 0)
      return -1;
    
    // increment the key count
    keycount++;
    
    // update the total key count in buffer
    memcpy(buffer, &keycount, sizeof(int));
    
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
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{
    // return error if sibling is not empty
    if (sibling.getKeyCount() != 0)
        return -1;

    // get the key count
    int keycount = getKeyCount();

    // insert the key, pid pair into the node
    insertNonLeafKey(buffer, keycount, key, pid);
    
    // increment the key count
    keycount++;
    
    // number of keys to split
    int halfkeycount = keycount / 2;
    
    // set p to point to the beginning of the splitting point
    char *p = buffer + sizeof(int) + 3*sizeof(int) + halfkeycount*3*sizeof(int);
    
    // sibling buffer pointer
    char *sibbuf = sibling.buffer + sizeof(int);
    
    // set midkey
    memcpy(&midKey, p, sizeof(int));
    // get the pid of this midkey
    memcpy(sibbuf + sizeof(int), p + sizeof(int), sizeof(int));
    // erase midkey entry
    memset(p, 0, 3*sizeof(int));
    
    // update p because we don't wanna copy the middle entry
    p += 3*sizeof(int);
    
    // update sibling buffer to point to the first entry
    sibbuf += 3*sizeof(int);
    
    for (int i = halfkeycount; i < keycount; i++) {
        // copy contents of this buffer to sibling's buffer
        memcpy(sibbuf, p, 3*sizeof(int));
        // set contents of this buffer to 0
        memset(p, 0, 3*sizeof(int));
        // advance pointers
        p += 3*sizeof(int);
        sibbuf += 3*sizeof(int);
    }
    
    // get position of the last pointer in buffer
    //char *t = buffer + sizeof(int) + 3*sizeof(int) + halfkeycount*3*sizeof(int);
    // save the pid of this spot
    //memset(t, 0, 3*sizeof(int));
    // erase the original last pointer spot
    //t = buffer + sizeof(int) + 3*sizeof(int) + keycount*3*sizeof(int);
    //memset(t, 0, 3*sizeof(int));
    
    // set new key count for the sibling buffer
    int sibkeycount = keycount - 1 - halfkeycount;
    
    // update the total key count in buffers
    memcpy(buffer, &halfkeycount, sizeof(int));
    memcpy(sibling.buffer, &sibkeycount, sizeof(int));

    return 0;
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{
    // flag to indicate that we've tried to search
    int flag = 0;
    // get key count
    int keycount = getKeyCount();

    for (int i = 0; i < keycount; i++) {
        // set flag to indicate that we've searched
        flag = 1;
        
        // set pointer to point past the first 4 bytes
        // since our key count is stored there
        char *p = buffer + sizeof(int) + 3*sizeof(int);
        // update p to point to the right pair
        p += i*(3*sizeof(int));
            
        int thiskey;
        // get this key
        memcpy(&thiskey, p, sizeof(int));
        
        if (searchKey < thiskey) {
            // store the pid into output pid
            memcpy(&pid, p - 3*sizeof(int) + sizeof(int), sizeof(int));
            return 0;
        }
    }
    
    // if we've searched and couldn't find it, it must be in the last pointer
    if (flag == 1) {
        // set pointer to point to the first pointer
        char *p = buffer + sizeof(int) + keycount*3*sizeof(int);
        // store the pid into output pid
        memcpy(&pid, p + sizeof(int), sizeof(int));
        return 0;
    }
    
    return -1;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{
    // set keycount to 1
    int keycount = 1;
    memcpy(buffer, &keycount, sizeof(int));
    // set pointer to point to first entry spot
    char *p = buffer + sizeof(int);
    
    // NOTE: we want same structure for non-leaf and leaf nodes for convenience later
    
    // set pid1 for root
    memcpy(p + sizeof(int), &pid1, sizeof(int));
    // update pointer to the next entry
    p += 3*sizeof(int);
    // set key for root
    memcpy(p, &key, sizeof(int));
    // update pointer to the next spot
    p += sizeof(int);
    // set pid2 for root
    memcpy(p, &pid2, sizeof(int));
    
    return 0;
}

/*
 * Initialize the non-leaf node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeNonLeaf(PageId pid1, int key, PageId pid2)
{
    // set keycount to 1
    int keycount = 1;
    memcpy(buffer, &keycount, sizeof(int));
    // set pointer to point to first entry spot
    char *p = buffer + sizeof(int);
    
    // NOTE: we want same structure for non-leaf and leaf nodes for convenience later
    
    // set pid1 for root
    memcpy(p + sizeof(int), &pid1, sizeof(int));
    // update pointer to the next entry
    p += 3*sizeof(int);
    // set key for root
    memcpy(p, &key, sizeof(int));
    // update pointer to the next spot
    p += sizeof(int);
    // set pid2 for root
    memcpy(p, &pid2, sizeof(int));
    
    return 0;
}

///////////////////////////////////////HELPER FUNCTION////////////////////////////////////////////////////

// insert (key, rid) to the buffer for leaf
// return 0 for success, -1 for failure
static int insertLeafKey(char* buffer, int keycount, int key, const RecordId& rid)
{
    // set pointer to point past the first 4 bytes
    // since our key count is stored there
    char *p = buffer + sizeof(int);

    // search for the right spot to insert the key if there's more than 0 keys
    if (keycount > 0) {
        // flag to see if we did need to shift keys
        int flag = 0;
    
        // search and check each key to insert the key at the right spot
        for(int i = 0; i < keycount; i++) {
            // set pointer to point past the first 4 bytes
            // since our key count is stored there
            p = buffer + sizeof(int);
            // update p to point to the right pair
            p += i*(3*sizeof(int));
            
            int thiskey;
            // get this key
            memcpy(&thiskey, p, sizeof(int));
            
            // we'll need to shift all pairs to the right
            // if a key is less than our previous keys
            if (key < thiskey) {
                char *t = buffer + sizeof(int) + (keycount-1)*3*sizeof(int);
                // shift the rest of the keys to the right
                for (; i < keycount; i++) {
                    memcpy(t + 3*sizeof(int), t, 3*sizeof(int));
                    // we want to copy from the right so we don't overwrite anything
                    t -= 3*sizeof(int);
                }
                // set flag to indicate that we shifted
                flag = 1;
                break;
            }
        }

        // append this key pair to the end if there wasn't any lower key
        
        // update the pointer to point to a new available space if no shifting was required
        if (flag == 0)
            p = buffer + sizeof(int) + keycount*3*sizeof(int);
    }
    
    // store the key into buffer
    memcpy(p, &key, sizeof(int));
            
    // update pointer to point to the next available memory spot
    p += sizeof(int);
    // store the pid into buffer
    memcpy(p, &rid.pid, sizeof(int));
            
    // update pointer to point to the next available memory spot
    p += sizeof(int);
    // store the sid into buffer
    memcpy(p, &rid.sid, sizeof(int));
    
    return 0;
}

// insert (key, pid) to the buffer for non-leaf
// return 0 for success, -1 for failure
static int insertNonLeafKey(char* buffer, int keycount, int key, PageId pid)
{
    // set pointer to point past the first 4 bytes
    // since our key count is stored there
    // bypass another entry since first pointer is stored there
    char *p = buffer + sizeof(int) + 3*sizeof(int);

    // only insert if there's more than 0
    if (keycount > 0) {
        // flag to see if we did need to shift keys
        int flag = 0;

        // search and check each key to insert the key at the right spot
        for(int i = 0; i < keycount; i++) {
            // set pointer to point past the first 4 bytes
            // since our key count is stored there
            // bypass another entry since first pointer is stored there
            p = buffer + sizeof(int) + 3*sizeof(int);
            // update p to point to the right pair
            p += i*(3*sizeof(int));
            
            int thiskey;
            // get this key
            memcpy(&thiskey, p, sizeof(int));
            
            // we'll need to shift all pairs to the right
            // if a key is less than our previous keys
            if (key < thiskey) {
                char *t = buffer + sizeof(int) + 3*sizeof(int) + (keycount-1)*3*sizeof(int);
                // shift the rest of the keys to the right
                for (; i < keycount; i++) {
                    memcpy(t + 3*sizeof(int), t, 3*sizeof(int));
                    // we want to copy from the right so we don't overwrite anything
                    t -= 3*sizeof(int);
                }
                // set flag to indicate that we shifted
                flag = 1;
                break;
            }
        }

        // append this key pair to the end if there wasn't any lower key
        
        // update the pointer to point to a new available space if no shifting was required
        if (flag == 0)
            p = buffer + sizeof(int) + 3*sizeof(int) + keycount*3*sizeof(int);
        
        // store the key into buffer
        memcpy(p, &key, sizeof(int));
                
        // update pointer to point to the next available memory spot
        p += sizeof(int);
        // store the pid into buffer
        memcpy(p, &pid, sizeof(int));
        
        return 0;
    }
    
    return -1;
}

////////////////////////////////////// TEST FUNCTION FOR LEAF //////////////////////////////////////

// clear the buffer
void BTLeafNode::clearBuffer()
{
    memset(buffer, 0, 1024); 
}

// print the buffer
void BTLeafNode::printBuffer()
{
    for (int i = 0; i < 1024; i+=4) {
        int r;
        memcpy(&r, buffer+i, sizeof(int));
        printf("%i", r);
        if (i%12 == 0) {
          printf(" ");
        }
    }
    printf("\n");
}

////////////////////////////////////// TEST FUNCTION FOR NON-LEAF //////////////////////////////////////

// clear the buffer
void BTNonLeafNode::clearBuffer()
{
    memset(buffer, 0, 1024); 
}

// print the buffer
void BTNonLeafNode::printBuffer()
{
    for (int i = 0; i < 1024; i+=4) {
        int r;
        memcpy(&r, buffer+i, sizeof(int));
        printf("%i", r);
        if (i%12 == 0) {
          printf(" ");
        }
    }
    printf("\n");
}


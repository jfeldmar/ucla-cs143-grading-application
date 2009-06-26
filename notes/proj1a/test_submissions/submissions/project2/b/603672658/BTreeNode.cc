#include "BTreeNode.h"

using namespace std;

/*
 * This is the constructor of BTLeafNode
 */
BTLeafNode::BTLeafNode()
{
    clearBuffer();
	setNextNodePtr(RC_INVALID_PID);
} // end of constructor BTLeafNode


/*
 * Clear the buffer by setting all bytes of the page to 0
 */
RC BTLeafNode::clearBuffer()
{
    memset(page, 0, PageFile::PAGE_SIZE);
    return 0;
} // end of clearBuffer

/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{
    return pf.read(pid, page);
} // end of read
    
/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{
    return pf.write(pid, page);
} // end of write

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
    int count;
    
    memcpy(&count, page, sizeof(int));

    return count;
} // end of getKeyCount

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
    char* ptr;
    int eid;
    int count = getKeyCount();

    if (count >= LMAXENTRY)
        return RC_NODE_FULL;

    if (!locate(key, eid)) {
        ptr = page + HEADER_SIZE + LENTRY_SIZE * eid;
        memmove(ptr+LENTRY_SIZE, ptr, (count-eid)*LENTRY_SIZE);
    }
    else {
        ptr = page + HEADER_SIZE + LENTRY_SIZE * count;
    }

    memcpy(ptr, &key, sizeof(int));
    memcpy(ptr+4, &rid, sizeof(RecordId));
    memcpy(page, &++count, sizeof(int));

    return 0;
} // end of insert

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
                              BTLeafNode& sibling, const PageId& siblingPid, int& siblingKey)
{
    int count = getKeyCount();
    int stayEntry = count / 2;
    int moveEntry = count - stayEntry;
    char* ptr = page + HEADER_SIZE + stayEntry * LENTRY_SIZE;

    PageId pid;
    memcpy(&pid, page+4, sizeof(PageId));    
    memcpy(&siblingKey, ptr, sizeof(int));

    // Split Entries: Sibling
    sibling.construct(ptr, moveEntry);
    sibling.setNextNodePtr(pid);

    // Split Entries: Origin
    memset(ptr, 0, moveEntry*LENTRY_SIZE);
    memcpy(page, &stayEntry, sizeof(int));
	setNextNodePtr(siblingPid);

    // Insert key
    if (key >= siblingKey)
        sibling.insert(key, rid);
    else
        insert(key, rid);

    return 0;
} // end of insertAndSplit

/*
 * copy numEntry entries from the original leaf to the new leaf
 * also set the count of the new leaf equal to numEntry
 */
RC BTLeafNode::construct(char* inPtr, int numEntry)
{
	if (numEntry > LMAXENTRY)
		return RC_NODE_FULL;

    clearBuffer();

    memcpy(page+HEADER_SIZE, inPtr, numEntry*LENTRY_SIZE);
    memcpy(page, &numEntry, sizeof(int));

    return 0;
} // end of construct

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
    int key;
    int count = getKeyCount();
    char* ptr = page + HEADER_SIZE;

    for (int i = 0; i < count; ++i) {
        if (i)
            ptr = ptr + LENTRY_SIZE;

        memcpy(&key, ptr, sizeof(int));

        if (key >= searchKey) {
            eid = i;
            return 0;
        }
    }

    return RC_NO_SUCH_RECORD;
} // end of locate

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{
	int count = getKeyCount();

	if (eid >= count)
		return RC_NO_SUCH_RECORD;

    char* ptr = page + HEADER_SIZE + eid * LENTRY_SIZE;

    memcpy(&key, ptr, sizeof(int));
    memcpy(&rid, ptr+4, sizeof(RecordId));    

    return 0;
} // end of readEntry

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
    PageId pid;
    
    memcpy(&pid, page+4, sizeof(PageId));

    return pid;
} // end of getNextNodePtr

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
    memcpy(page+4, &pid, sizeof(int));
    return 0;
} // end of setNextNodePtr

//-----------------------------------------------------------------------------


/*
 * This is the constructor of BTNonLeafNode
 */
BTNonLeafNode::BTNonLeafNode()
{
    clearBuffer();
} // end of constructor BTNonLeafNode

/*
 * Clear the buffer by setting all bytes of the page to 0
 */
RC BTNonLeafNode::clearBuffer()
{
    memset(page, 0, PageFile::PAGE_SIZE);
    return 0;
} // end of clearBuffer

/*
 * copy numEntry entries from the original leaf to the new leaf
 * also set the count of the new leaf equal to numEntry
 */
RC BTNonLeafNode::construct(char* inPtr, int numEntry)
{
	if (numEntry > NMAXENTRY)
		return RC_NODE_FULL;

    clearBuffer();

    memcpy(page, &numEntry, sizeof(int));
    memcpy(page+4, inPtr, sizeof(PageId)+numEntry*NENTRY_SIZE);

    return 0;
} // end of construct

/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{
    return pf.read(pid, page);
} // end of read
    
/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{
    return pf.write(pid, page);
} // end of write

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
    int count;

    memcpy(&count, page, sizeof(int));

    return count;
} // end of getKeyCount


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
	char* ptr;
	int eid;
	int count = getKeyCount();

	if (count >= NMAXENTRY)
		return RC_NODE_FULL;

	if (!locate(key, eid)) {
        ptr = page + HEADER_SIZE + NENTRY_SIZE * eid;
        memmove(ptr+NENTRY_SIZE, ptr, (count-eid)*NENTRY_SIZE);
    }
    else {
        ptr = page + HEADER_SIZE + NENTRY_SIZE * count;
    }

    memcpy(ptr, &key, sizeof(int));
    memcpy(ptr+4, &pid, sizeof(PageId));
    memcpy(page, &++count, sizeof(int));

	return 0;
} // end of insert

/*
 * Find the entry whose key value is larger than searchKey
 * and output the eid (entry number) whose key value > searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locate(int searchKey, int& eid)
{
    int key;
    int count = getKeyCount();
    char* ptr = page + HEADER_SIZE;

    for (eid = 0; eid < count; ++eid) {
        if (eid)
            ptr = ptr + NENTRY_SIZE;

        memcpy(&key, ptr, sizeof(int));

        if (key > searchKey) {
            return 0;
        }
    }

    return RC_NO_SUCH_RECORD;
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
    int count = getKeyCount();
    int stayEntry = count / 2;
    int moveEntry = count - stayEntry - 1;
    char* ptr = page + HEADER_SIZE + stayEntry * NENTRY_SIZE;
    
    memcpy(&midKey, ptr, sizeof(int));

    // Split Entries: Sibling
    sibling.construct(ptr+4, moveEntry);

    // Split Entries: Origin
    memset(ptr, 0, (moveEntry+1)*NENTRY_SIZE);
    memcpy(page, &stayEntry, sizeof(int));

    // Insert key
    if (key >= midKey)
        sibling.insert(key, pid);
    else
        insert(key, pid);

	return 0;
} // end of insertAndSplit

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{
	char* ptr;
	int eid;

	if(!locate(searchKey, eid)) {
		ptr = page + HEADER_SIZE + eid*NENTRY_SIZE;
		memcpy(&pid, ptr-4, sizeof(PageId));
	}
	else {
		ptr = page + HEADER_SIZE + (eid-1)*NENTRY_SIZE;
		memcpy(&pid, ptr+4, sizeof(PageId));
	}

	return 0;
} // end of locateChildPtr

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{
	clearBuffer();

	int count = 1;

	memcpy(page, &count, sizeof(int));
	memcpy(page+4, &pid1, sizeof(PageId));
	memcpy(page+8, &key, sizeof(int));
	memcpy(page+12, &pid2, sizeof(PageId));

	return 0;
} // end of initializeRoot
#include "BTreeNode.h"
#include <cmath>

using namespace std;

/* Constructs the leaf node */
BTLeafNode::BTLeafNode()
: m_nkeys(0)
{
    Next next; next.pid = -1; setNext(next);
}

/* Retrieves a handle to a Pair */
int BTLeafNode::getPair(int eid, Pair &pair)
{
        if (eid > MAX_PAIRS) {
                //fprintf(stderr, "error: eid out of bounds\n");
                return 1;
        }
        //if (eid > getKeyCount()) fprintf(stderr, "warning: non-sequential pair access\n");
        pair = *(Pair *)(m_buffer + (eid * sizeof(Pair)));
        return 0;
}

int BTLeafNode::setPair(int eid, const Pair &pair)
{
        if (eid > MAX_PAIRS) {
                //fprintf(stderr, "error: eid out of bounds\n");
                return 1;
        }
        //if (eid > getKeyCount()) fprintf(stderr, "warning: non-sequential pair access\n");
        memcpy(m_buffer + (eid *sizeof(Pair)), &pair, sizeof(pair));
        return 0;
}

/* Retrieves a handle to the Next pointer */
int BTLeafNode::getNext(Next &next)
{
        next = *(Next *)(m_buffer + (PageFile::PAGE_SIZE - sizeof(Next)));
        return 0;
}

int BTLeafNode::setNext(const Next &next)
{
        memcpy(m_buffer + (PageFile::PAGE_SIZE - sizeof(Next)), &next, sizeof(Next));
        return 0;
}

/* Print out a node residency */
void BTLeafNode::print()
{
        fprintf(stderr, "keys (%d)\n", getKeyCount());
        int eid;
        for (eid = 0; eid < getKeyCount(); eid++) {
                Pair p;
                getPair(eid, p);
                fprintf(stderr, "|%d,(%d,%d)", p.key, p.rid.pid, p.rid.sid);
        }
        for ( ; eid < MAX_PAIRS; eid++) {
                fprintf(stderr, "|........");
        }
        Next n;
        getNext(n);
        fprintf(stderr, "|%d|\n", n.pid);
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
        //fprintf(stderr, "reading a leaf node from disk\n");
        pf.read(pid, (void *)m_buffer);
        int n; for (n = 0; n < MAX_PAIRS; n++) { Pair temp; getPair(n, temp); if (temp.rid.pid == -1) break; } m_nkeys = n;  /* find count */
        return 0;
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
        //fprintf(stderr, "writing a leaf node to disk\n");
        Pair temp; temp.rid.pid = -1; if (getKeyCount() < MAX_PAIRS) { setPair(getKeyCount(), temp); } /* set end pair marker */
        pf.write(pid, (const void*)m_buffer);
        return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
        return m_nkeys;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 *      Returns 1 if leaf is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
        /* sanity check: any room? */
        if (getKeyCount() >= MAX_PAIRS) return 1;
        /* locate where we need to insert */
        int eid;
        binarySearch(key, 0, getKeyCount() - 1, eid);
        //fprintf(stderr, "inserting %d,(%d,%d) at eid (%d)\n", key, rid.pid, rid.sid, eid);
        /* move entries up */
        for (int n = getKeyCount() - 1; n >= eid; n--) {
                Pair temp;
                getPair(n, temp);
                setPair(n + 1, temp);
        }
        Pair temp;
        temp.key = key;
        temp.rid = rid;
        setPair(eid, temp);
        m_nkeys++;
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
 *      Returns 1 if sibling node is non-empty.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{
        /* sanity check: sibling is empty */
        if (sibling.getKeyCount() != 0) {
                //fprintf(stderr, "error: sibling node is non-empty on split\n");
                return 1;
        }
        /* locate where we need to insert */
        int eid;
        binarySearch(key, 0, getKeyCount() - 1, eid);
        //fprintf(stderr, "inserting %d,(%d,%d) at eid (%d)\n", key, rid.pid, rid.sid, eid);
        /* insert the pair */
        insert(key, rid);
        int middle = ceil((double)getKeyCount() / 2.0) - 1;
        /* copy records over */
        for (int n = getKeyCount() - 1; n > middle; n--) {
                Pair temp;
                getPair(n, temp);
                if (n == middle + 1) {
                        siblingKey = temp.key;
                }
                sibling.setPair(n - middle - 1, temp);
                sibling.m_nkeys++;
                m_nkeys--;
        }
        //fprintf(stderr, "new sibling created with key (%d)\n", siblingKey);
        return 0;
}

int BTLeafNode::binarySearch(int searchKey, int begin, int end, int &eid)
{
        if (getKeyCount() == 0) {
                eid = 0;
                return 0;
        }
        if (end >= getKeyCount()) {
                //fprintf(stderr, "error: search end (%d) out of bounds\n", end);
                return 1;
        }
        int midpoint = (end + begin) / 2;
        /* found it at midpoint? */
        Pair temp;
        getPair(midpoint, temp);
        if (temp.key == searchKey) {
                eid = midpoint;
                return 0;
        }
        /* lower? */
        else if (temp.key > searchKey && midpoint > begin) {
                return binarySearch(searchKey, begin, midpoint - 1, eid);
        }
        /* higher? */
        else if (temp.key < searchKey && midpoint < end) {
                return binarySearch(searchKey, midpoint + 1, end, eid);
        }
        /* the node's destination */
        if (temp.key > searchKey) eid = midpoint;
        else eid = midpoint + 1;
        return 1;
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 *      Returns 1 if value does not exist.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{
        /* if empty, return error */
        if (getKeyCount() == 0) return 1;
        /* binary search */
        int r;
        r = binarySearch(searchKey, 0, getKeyCount() - 1, eid);
        //if (r) fprintf(stderr, "warning: unable to locate %d\n", searchKey);
        //else fprintf(stderr, "located %d at eid (%d)\n", searchKey, eid);
        return 0;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 *      Returns 1 if eid is out of range
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{
        /* Sanity check: eid is valid */
        if (eid >= getKeyCount()) {
                //fprintf(stderr, "read error: eid (%d) out of range\n", eid);
                return 1;
        }
        Pair temp;
        getPair(eid, temp);
        key = temp.key;
        rid = temp.rid;
        return 0;
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
        Next temp;
        getNext(temp);
        return temp.pid;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
        Next temp;
        temp.pid = pid;
        setNext(temp);
        return 0;
}

/* Constructs the leaf node */
BTNonLeafNode::BTNonLeafNode()
: m_nkeys(0)
{ }

/* Retrieves a handle to a Triplet */
int BTNonLeafNode::getPair(int eid, Pair &pair)
{
        if (eid > MAX_PAIRS) {
                //fprintf(stderr, "error: eid out of bounds\n");
                return 1;
        }
        //if (eid > getKeyCount()) fprintf(stderr, "warning: non-sequential pair access\n");
        pair = *(Pair *)(m_buffer + (eid * sizeof(Pair)) + sizeof(Prev));
        return 0;
}

int BTNonLeafNode::setPair(int eid, const Pair &pair)
{
        if (eid > MAX_PAIRS) {
                //fprintf(stderr, "error: eid out of bounds\n");
                return 1;
        }
        //if (eid > getKeyCount()) fprintf(stderr, "warning: non-sequential pair access\n");
        memcpy(m_buffer + (eid * sizeof(Pair)) + sizeof(Prev), &pair, sizeof(Pair));
        return 0;
}

/* Retrieves a handle to the Prev pointer */
int BTNonLeafNode::getPrev(Prev &prev)
{
        prev = *(Prev *)(m_buffer);
        return 0;
}

int BTNonLeafNode::setPrev(const Prev &prev)
{
        memcpy(m_buffer, &prev, sizeof(Prev));
        return 0;
}

/* Print out a node residency */
void BTNonLeafNode::print()
{
        fprintf(stderr, "keys (%d)\n", getKeyCount());
        Prev v;
        getPrev(v);
        fprintf(stderr, "|%d", v.pid);
        int eid;
        for (eid = 0; eid < getKeyCount(); eid++) {
                Pair p;
                getPair(eid, p);
                fprintf(stderr, "|%d,%d", p.key, p.pid);
        }
        for ( ; eid < MAX_PAIRS; eid++) {
                fprintf(stderr, "|........");
        }
        fprintf(stderr, "|\n");
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
        //fprintf(stderr, "reading a non-leaf node from disk\n");
        pf.read(pid, (void *)m_buffer);
        int n; for (n = 0; n < MAX_PAIRS; n++) { Pair temp; getPair(n, temp); if (temp.pid == -1) break; } m_nkeys = n;  /* find count */
        return 0;
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
        //fprintf(stderr, "writing a non-leaf node to disk\n");
        Pair temp; temp.pid = -1; if (getKeyCount() < MAX_PAIRS) { setPair(getKeyCount(), temp); } /* set end pair marker */
        pf.write(pid, (void *)m_buffer);
        return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
        return m_nkeys;
}

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 *      Returns 1 if node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
        /* sanity check: any room? */
        if (getKeyCount() >= MAX_PAIRS) return 1;
        /* locate where we need to insert */
        int eid;
        binarySearch(key, 0, getKeyCount() - 1, eid);
        //fprintf(stderr, "inserting %d,%d at eid (%d)\n", key, pid, eid);
        /* move entries up */
        for (int n = getKeyCount() - 1; n >= eid; n--) {
                Pair temp;
                getPair(n, temp);
                setPair(n + 1, temp);
        }
        Pair temp;
        temp.key = key;
        temp.pid = pid;
        setPair(eid, temp);
        m_nkeys++;
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
        /* sanity check: sibling is empty */
        if (sibling.getKeyCount() != 0) {
                //fprintf(stderr, "error: sibling node is non-empty on split\n");
                return 1;
        }
        /* locate where we need to insert */
        int eid;
        binarySearch(key, 0, getKeyCount() - 1, eid);
        //fprintf(stderr, "inserting %d,(%d,%d) at eid (%d)\n", key, pid, eid);
        /* insert the pair */
        insert(key, pid);
        int middle = ceil((double)getKeyCount() / 2.0) - 1;
        /* copy records over */
        for (int n = getKeyCount() - 1; n > middle; n--) {
                Pair temp;
                getPair(n, temp);
                if (n == middle + 1) {
                        Prev v;
                        v.pid = temp.pid;
                        sibling.setPrev(v);
                        midKey = temp.key;
                }
                /* add if not middle */
                else {
                        sibling.setPair(n - middle - 2, temp);
                        sibling.m_nkeys++;
                }
                m_nkeys--;
        }
        //fprintf(stderr, "new sibling created, split on key (%d)\n", midKey);
        return 0;
}

int BTNonLeafNode::binarySearch(int searchKey, int begin, int end, int &eid)
{
        if (getKeyCount() == 0) {
                eid = 0;
                return 0;
        }
        if (end >= getKeyCount()) {
                //fprintf(stderr, "error: search end (%d) out of bounds\n", end);
                return 1;
        }
        int midpoint = (end + begin) / 2;
        /* found it at midpoint? */
        Pair temp;
        getPair(midpoint, temp);
        if (temp.key == searchKey) {
                eid = midpoint;
                return 0;
        }
        /* lower? */
        else if (temp.key > searchKey && midpoint > begin) {
                return binarySearch(searchKey, begin, midpoint - 1, eid);
        }
        /* higher? */
        else if (temp.key < searchKey && midpoint < end) {
                return binarySearch(searchKey, midpoint + 1, end, eid);
        }
        /* the node's destination */
        if (temp.key > searchKey) eid = midpoint;
        else eid = midpoint + 1;
        return 1;
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
        int eid, r;
        r = binarySearch(searchKey, 0, getKeyCount() - 1, eid);
        //fprintf(stderr, "key (%d) belongs at node (%d) [%d]\n", searchKey, eid, r);
        /* is it in Prev? */
        if (r && (eid - 1) < 0) {
                Prev temp;
                getPrev(temp);
                pid = temp.pid;
        }
        else {
                if (!r) {
                        Pair temp;
                        getPair(eid, temp);
                        pid = temp.pid;
                }
                else {
                        Pair temp;
                        getPair(eid - 1, temp);
                        pid = temp.pid;
                }
        }
        //fprintf(stderr, "child pointer for (%d) located at (%d)\n", searchKey, pid);
        return 0;
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
        Prev v;
        v.pid = pid1;
        setPrev(v);
        Pair p;
        p.key = key;
        p.pid = pid2;
        setPair(0, p);
        m_nkeys++;
        return 0;
}

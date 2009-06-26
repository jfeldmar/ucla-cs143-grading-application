#include "BTreeNode.h"
#include "BTreeIndex.h"
#include "RecordFile.h"

using namespace std;

#define CONFIG_DFLAG 0
#define DEBUG if (CONFIG_DFLAG)

/* helper functions for node manipulation */

static nodeEntry* entryPtr(nodeEntry* nodepage, int eid);

/* calculate a pointer to the entry in this node
 * specified by the integer eid.
 * each Cursor is two ints, and each key is one int.
 * we skip the first int which is the number of used keys
 */
static nodeEntry* entryPtr(nodeEntry* nodepage, int eid)
{
				return &nodepage[eid];
}


// return whether node has room for more records
bool BTNode::nodeIsFull()
{
				int keycount = getKeyCount();

				// +1 for the keycount record
				// +1 for the "freespace" record (for splitting)
				if (keycount + 2 == numnodes) {
					return true;
				}

				return false;

}

/*
 * Read the content of the node into buffer 
 * from the page (pid) in the PageFile (pf).
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::read(PageId pid, const PageFile& pf)
{

				RC rc;

				rc = pf.read(pid, (char *)nodepage);
				return rc;

}
    
/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::write(PageId pid, PageFile& pf)
{ 
				RC rc;

				rc = pf.write(pid, &nodepage);

				return rc; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNode::getKeyCount()
{ 

				//memcpy(rc, &nodepage, sizeof(int));
				int keycount = nodepage[lastnode].key;
				//fprintf(stdout, "\tkeycount: %d\n", keycount);
				return keycount;
}

/* set the number of keys in the node */

int BTNode::setKeyCount(int newKeyCount)
{
				//memcpy(nodepage, &newKeyCount, sizeof(int));
				nodepage[lastnode].key = newKeyCount;
				return getKeyCount();

}


void BTNode::print()
{
	int i;
	nodeEntry* nent;

	DEBUG fprintf(stdout, "Node: %p (pid: %d)\n", nodepage, mypid);
	for (i = 0; i < numnodes; i++) {
		DEBUG fprintf(stdout, "% 4s  % 4s  ", "rid", "key");
	}
	DEBUG fprintf(stdout, "\n");
	for (i = 0; i < numnodes; i++) {
		nent = &nodepage[i];
		int pid, sid, key;
		pid = nent->rid.pid;
		sid = nent->rid.sid;
		key = nent->key;
		DEBUG fprintf(stdout, "% 4d  % 4d  ", pid, key);
	}
	DEBUG fprintf(stdout, "\n");
	for (i = 0; i < numnodes; i++) {
		nent = &nodepage[i];
		int pid, sid, key;
		pid = nent->rid.pid;
		sid = nent->rid.sid;
		key = nent->key;
		DEBUG fprintf(stdout, "% 4d        ", sid);
	}
	DEBUG fprintf(stdout, "\n\n");
}

/**
 * return the pid of the parent of this node
 */

int BTNode::getmyParent()
{
				return myParent;
}

/*
 * Set the parent of this node.
 * @param pid[IN] the PageId of the parent
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::setmyParent(PageId pid)
{

				myParent = pid;
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
RC BTNode::locate(int searchKey, int& eid)
{
	//fprintf(stdout, "In locate with searchKey %d...\n", searchKey);
	int keycount = getKeyCount(); // get num keys in node
	int i;
	if (keycount == 0) {
					eid = 0;
					DEBUG fprintf(stdout, "\tLocated eid %d\n", eid);
					return 0; 
	}
	for (i = 0; i <= keycount; i++) { // for each key

					/* get entry for this key */
					nodeEntry* entry = entryPtr(nodepage, i);
					if (entry == NULL) return RC_INVALID_CURSOR;

					if (entry->key >= searchKey) { // compare entry key
									eid = i; // found key
									DEBUG fprintf(stdout, "* Located eid %d for key %d\n", eid, searchKey);
									return 0; 
					}
	}

	DEBUG fprintf(stdout, "Did not locate an entry!\n");
	return RC_NO_SUCH_RECORD; // somehow, no key applied!

}

/*
 * Read the (key, rid) pair from the eid entry.
 * KEY: the search key
 * RID: the RecordId -- pid, sid
 * -- together these are my nodeEntry struct
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNode::readEntry(int eid, int& key, RecordId& rid)
{
				if (eid > numnodes) {
								return RC_INVALID_CURSOR;
				}

				nodeEntry* entry = entryPtr(nodepage, eid);
				if (entry == NULL) return RC_INVALID_CURSOR;
				key = entry->key;
				rid = (RecordId)entry->rid;
				return 0; 
}

RC BTNode::eraseEntry(int eid)
{
				if (eid > numnodes) {
								return RC_INVALID_CURSOR;
				}
				nodeEntry* entry = entryPtr(nodepage, eid);
				memset(entry, '\0', sizeof(nodeEntry));
				return 0;

}

/* write they (key, rid) pair into the specified eid
 * check for range, then get pointer to nodeEntry entry
 * set entry->key and copy values of new RecordId in
 * @param eid[IN]
 * @param key[IN]
 * @param rid[IN]
 */

RC BTNode::writeEntry(int eid, int key, RecordId rid)
{
				if (eid > numnodes) { // eid doesn't exist
								return RC_INVALID_CURSOR;
				} // otherwise

				nodeEntry* entry = entryPtr(nodepage, eid); // get entry
				entry->key = key; // set key
				entry->rid = rid; // set rid
				return 0; 

}

BTNode::BTNode()
{
				nodepage = (nodeEntry *)nodebuffer;
				memset(nodepage, '\0', CONFIG_NODESIZE);
				setKeyCount(0);
				mypid = -5;

}

BTLeafNode::BTLeafNode()
{
				//numnodes = PageFile::PAGE_SIZE / sizeof(nodeEntry);
				BTNode();
				setLeafFlag();
}

int BTLeafNode::setLeafFlag()
{
				int keycount;
				keycount = getKeyCount();
				RecordId rid;
				rid.pid = 0;
				rid.sid = -2;
				writeEntry(lastnode, keycount, rid);
				return 0;


}


BTNonLeafNode::BTNonLeafNode()
{
				//numnodes = PageFile::PAGE_SIZE / sizeof(nodeEntry);
				BTNode();
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{

				// we'll test for fullness at a higher level
				int rc;
				int keycount;
				keycount = getKeyCount();
				int eid;
				DEBUG fprintf(stdout, "In LN::insert with key %d.\n", key);
				if ((rc = locate(key, eid)) < 0) { // the eid we'll insert into
								if (rc == RC_NO_SUCH_RECORD) {
									eid = keycount; // add to end
								} else {
									return rc; // error
								}
				}

				if (eid < keycount) { // if eid is going to overwrite
								// shift array items over one (except for last)
								DEBUG fprintf(stdout, "Shifting entries...\n");
								nodeEntry nodetemp[numnodes];
								memcpy(&nodetemp[0], &nodepage[eid], (keycount - eid)*sizeof(nodeEntry));
								memcpy(&nodepage[eid+1], &nodetemp[0], (keycount - eid)*sizeof(nodeEntry));
								print();
				}
				
				// write into eid
				if (( rc = writeEntry(eid, key, rid)) < 0) {
								return rc; // error
				}
				setKeyCount(keycount + 1);
				DEBUG fprintf(stdout, "Insertion complete:\n");
				print();

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
				/* thanks to isNodeFull, there is always one slot free,
				 * even when a node is "full". This way, we don't need to 
				 * allocate temporary space for splitting 
				 * */
				
				DEBUG fprintf(stdout, "Inserting and Splitting (leaf)\n");
				DEBUG fprintf(stdout, "Original:\n");
				print();

				int rc;
				PageId nextSib;
				
				/* update sibling pointers:
				 * the new sibling gets our OLD sibling pointer
				 * and now WE point to the NEW sibling.
				 * */

				/* insert new key into "full" node
				 * */
				if ((rc = insert(key, rid)) < 0) {
								return rc;
				}

				/* split keys between orig and new nodes
				 * get number to split on
				 * copy nodeEntries to new sibling
				 * erase copied nodeEntries from orig
				 * */
				int split = numnodes / 2;
				DEBUG fprintf(stdout, "Splitting entries...\n");

				nodeEntry nodetemp[numnodes];
				
				memcpy(&nodetemp[0], &nodepage[split], 
							 sizeof(nodeEntry)*(numnodes - split));
				memcpy(&sibling.nodepage[0], &nodetemp[0], 
							 sizeof(nodeEntry)*(numnodes - split));
				
				// move special record to end
				int movekey;
				RecordId moverid;
				sibling.readEntry((numnodes - split - 1), movekey, moverid);
				sibling.writeEntry(lastnode, movekey, moverid);
				sibling.eraseEntry(numnodes - split - 1);

				memset(&nodepage[split], '\0', sizeof(nodeEntry)*(numnodes-split));
				
				//nextSib = getNextNodePtr(); // save current sib ptr
				setNextNodePtr(sibling.mypid); // pt to new sib
				//sibling.setNextNodePtr(nextSib); // new sib gets old ptr

				setKeyCount(split);
				setLeafFlag();
				
				sibling.setKeyCount(numnodes - split - 1);
				//sibling.setLeafFlag();

				DEBUG fprintf(stdout, "Original table:\n");
				print();
				DEBUG fprintf(stdout, "New table:\n");
				sibling.print();

				siblingKey = sibling.nodepage[0].key;

				return 0; 
}

/*
 * Output the pid of the next slibling node.
 * @return pid if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
				RC rc;
				RecordId rid;
				int key;
				if ((rc = readEntry(numnodes, key, rid)) < 0) {
								return rc; // error
				}
				return rid.pid; // page id 
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
				RC rc;
				RecordId rid;
				rid.pid = pid;
				int keycount;
				rc = readEntry(lastnode, keycount, rid);
				rid.pid = pid;
				rc = writeEntry(lastnode, keycount, rid);
				return rc; 
}

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
		int rc;
		int keycount;
		keycount = getKeyCount();
		int isNewRoot = 0;
		int eid;
		RecordId rid, rid2;
		rid.sid = rid2.sid = 0;
		rid.pid = pid;
	
		DEBUG fprintf(stdout, "Inserting key %d, pid %d into NL node...\n", key, pid);
		print();

		if ((rc = locate(key, eid)) < 0) {
						if (rc == RC_NO_SUCH_RECORD) {
										DEBUG fprintf(stdout, "No match, eid set to %d\n", keycount);
										eid = keycount;
						} else {
							return rc; // error
						}
		}

		if (eid < keycount) {
			// shift array items over one (except for last)
			DEBUG fprintf(stdout, "Shifting items...\n");
			print();
			nodeEntry nodetemp[numnodes];
			memcpy(&nodetemp[0], &nodepage[eid], (keycount - eid )*sizeof(nodeEntry));
			memcpy(&nodepage[eid + 1], &nodetemp[0], (keycount - eid)*sizeof(nodeEntry));

			print();
			if (( rc = writeEntry(eid, key, rid)) < 0) {
				return rc; //error
			}
		} else {
			/* eid >= keycount -- we don't need to make room
			 * but we will need to update the end pointer
			 * */

			// get last rid
			int xkeycount;
			if (( rc = readEntry(lastnode, xkeycount, rid2)) < 0) {
							return rc;
			}

			// assign old "last rid" to new entry
			rid.pid = rid2.pid;
			if ((rc = writeEntry(eid, key, rid)) < 0 ) {
							return rc;
			}
			
			// assign new pid to last rid
			rid2.pid = pid;
			rid2.sid = 0;
			if ((rc = writeEntry(lastnode, xkeycount, rid2)) < 0) {
							return rc;
			}
		}

		setKeyCount(keycount + 1);
		print();
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
				RC rc;

				if ((rc = insert(key, pid)) < 0) {
								return rc;
				}
				/* split keys between orig and new nodes
				 * get number to split on
				 * copy nodeEntries to new sibling
				 * erase copied nodeEntries from orig
				 * */
				int split = numnodes / 2; 
				nodeEntry nodetemp[numnodes];
				memcpy(&nodetemp[0], &nodepage[split],
							 sizeof(nodeEntry)*(numnodes - split));
				memcpy(&sibling.nodepage[0], &nodetemp[0],
							 sizeof(nodeEntry)*(numnodes - split));
				memset(&nodepage[split], '\0', sizeof(nodeEntry)*(numnodes - split));

				/* we need to move the end pointer out
				 * on the new sibling
				 * */
				int lastcopied = numnodes - split - 1; // last copied record
				sibling.nodepage[lastnode].rid.pid 
								= sibling.nodepage[lastcopied].rid.pid;
				sibling.nodepage[lastcopied].rid.pid = 0;


				/* we need to update the keycount in the new sibling
				 * and in the original node and erase the leftovers
				 * */
				sibling.setKeyCount(numnodes - split - 1);
				sibling.nodepage[lastcopied].key = 0;
				setKeyCount(split); // keys left in this node
				
				/* update new "last pointer" on original node
				 * to point to "lowest" child of new node
				 * and updated keycount on original node
				 * */
				writeEntry(lastnode, split, (RecordId)sibling.nodepage[0].rid);

				midKey = sibling.nodepage[0].key;

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

				RC rc;
				int eid, key;
				RecordId rid;

				DEBUG fprintf(stdout, "Locating pointer pid for key %d\n", searchKey);
				print();

				if ((rc = locate(searchKey, eid)) < 0) {
								if (rc == RC_NO_SUCH_RECORD) {
									/* if a key greater than searchKey is not found,
									 * we assume it must be greater and so get the
									 * last pointer
									 * */
									eid = lastnode; // get last pointer
								} else { 
									return rc; // error
								}
				}
	
				if ((rc = readEntry(eid, key, rid)) < 0) {
								return rc; // error
				}
				pid = rid.pid; // page id return value
				DEBUG fprintf(stdout, "Found pid %d for searchkey %d\n", pid, searchKey);
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

				RC rc;
				RecordId rid1, rid2;
				rid1.pid = pid1;
				rid1.sid = 0;
				rid2.pid = pid2;
				rid2.sid = 0; // sid of -100 denotes last rid

				if ((rc = writeEntry(0, key, rid1)) < 0) {
					return rc;
				}
				if ((rc = writeEntry(lastnode, 1, rid2)) < 0) { // last pointer
					return rc;
				}
				return 0; 
}

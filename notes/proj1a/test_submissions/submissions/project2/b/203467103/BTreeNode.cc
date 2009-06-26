#include "BTreeNode.h"
#include <iostream>
using namespace std;

// A DRAWING OF THE LEAF NODE
// ||
// || rid | key | rid | key | ... | padding | TYPE | NUMKEYS | NEXTNODE ||
// ||
//

// A DRAWING OF THE NONLEAF NODE
// WHY IS THIS THE OPPOSITE OF LEAF NODES?
// change it later maybe
// ||
// || key | pid | key | pid | ... | padding | TYPE | NUMKEYS | PREVNODE ||
// ||
//

BTLeafNode::BTLeafNode()
{
	m_numKeys = 0;
	m_debug = 0;
}

void BTLeafNode::setDebug(int num)
{
	if (num)
		m_debug = true;
	else
		m_debug = false;
}

void BTLeafNode::toggleDebug()
{
	if (!m_debug)
		m_debug = true;
	else
		m_debug = false;

}

void BTLeafNode::debugOn()
{
	m_debug = true;
}

void BTLeafNode::debugOff()
{
	m_debug = false;
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
	RC rc;
	// check for invalid page id
	
	rc = pf.read(pid, buffer);
	memcpy(&m_numKeys, buffer+NUMKEY_LOC, sizeof(int));
	return rc;
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
	RC rc;
	// check for invalid page id
	rc = pf.write(pid, buffer);
	return rc;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	return m_numKeys;
}

/*
 * Insert a (key, rid) pair to the node.
 * Keep the order of the keys
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
	// TODO: check for duplicates
	// check if node is full
	if (m_numKeys == MAX_ENTRIES)
		return RC_NODE_FULL;
	int eid = -1;
	// find the first key that is greater than key (eid should be correct)
	int probe = 0;
	do {
		// basically the same as readEntry
		eid++;
		// get the key
		memcpy(&probe, buffer+(eid*ENTRY_SIZE)+sizeof(RecordId), sizeof(int));
		if (m_debug) printf("EID %d, probe %d\n", eid, probe);
	} while (probe < key && eid < m_numKeys);
	if (m_debug) printf("inserting KEY: %d RecordId:%d,%d into %d\n",
		key, rid.pid, rid.sid, eid);
	
	// size of data to shift = ALL ENTRY DATA - STUFF BEFORE INSERTION - HEADER SIZE
	// HEADER SIZE because we don't want to overwrite the header info
	int dataSize = MAX_ENTRIES*ENTRY_SIZE - eid*ENTRY_SIZE - HEADER_SIZE;
	// if there's overflow, do stuff
	
	// shift everything to the right ENTRY_SIZE spaces (buffer+(eid*ENTRY_SIZE))
	char* temp;
	temp = new char[dataSize];
	memcpy(temp, buffer+(eid*ENTRY_SIZE), dataSize);
	memcpy(buffer+((eid+1)*ENTRY_SIZE), temp, dataSize);
	delete [] temp;
	// insert the record into eid
	memcpy(buffer+(eid*ENTRY_SIZE), &rid, sizeof(RecordId));
	// insert key into eid
	memcpy(buffer+(eid*ENTRY_SIZE)+sizeof(RecordId), &key, sizeof(int));
	// change the number of entries
	int numKeys = m_numKeys;
	numKeys++;
	m_numKeys = numKeys;
	// put that into the buffer
	memcpy(buffer+NUMKEY_LOC, &numKeys, sizeof(int));
	
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
	// TODO: check for duplicate keys
	// TEST: odd number, even number,  out of order
	if (m_debug) cerr << "SPLITSPLITSPLIT" << endl;
	char tempBuffer[PageFile::PAGE_SIZE+ENTRY_SIZE]; // temporarily overfilled buffer
	
	int eid = -1;
	// find the first key that is greater than key (eid should be correct)
	int probe = 0;
	do {
		// basically the same as readEntry
		eid++;
		// get the key
		memcpy(&probe, buffer+(eid*ENTRY_SIZE)+sizeof(RecordId), sizeof(int));
		if (m_debug) printf("EID %d, probe %d\n", eid, probe);
	} while (probe < key && eid < m_numKeys);
	if (m_debug) printf("inserting KEY: %d RecordId:%d,%d into %d\n",
		key, rid.pid, rid.sid, eid);
	
	int dataSize = PageFile::PAGE_SIZE - eid*ENTRY_SIZE; // size of the data we want to shift
	
	// put it all in the temp buffer
	memcpy(tempBuffer, buffer, PageFile::PAGE_SIZE);
	
	// shift everything to the right ENTRY_SIZE spaces (buffer+(eid*ENTRY_SIZE))
	char* temp;
	temp = new char[dataSize];
	memcpy(temp, tempBuffer+(eid*ENTRY_SIZE), dataSize);
	memcpy(tempBuffer+((eid+1)*ENTRY_SIZE), temp, dataSize);
	delete [] temp;
	
	// insert the record, key into eid
	memcpy(tempBuffer+(eid*ENTRY_SIZE), &rid, sizeof(RecordId));
	memcpy(tempBuffer+(eid*ENTRY_SIZE)+sizeof(RecordId), &key, sizeof(int));
	
	// change the number of entries
	int numKeys = m_numKeys;
	numKeys++;
	int firstKeys = numKeys / 2;
	int secondKeys = numKeys - firstKeys;
	
	// split
	int splitLocation = numKeys / 2;
	cerr << "SPLIT LOCATION: " << splitLocation << endl;
	print_buffer();
	
	// first half goes into current buffer
	dataSize = (splitLocation+1)*ENTRY_SIZE;
	memcpy(buffer, tempBuffer, dataSize);
	
	// second half goes into sibling buffer
	// sibling has already been constructed
	dataSize = PageFile::PAGE_SIZE - dataSize;
	memcpy(sibling.buffer, tempBuffer+(splitLocation*ENTRY_SIZE),
		dataSize);
	
	// update the number of keys
	m_numKeys = firstKeys;
	sibling.m_numKeys = secondKeys;
	// put that into the buffer
	memcpy(buffer+NUMKEY_LOC, &firstKeys, sizeof(int));
	memcpy(sibling.buffer+NUMKEY_LOC, &secondKeys, sizeof(int));
	
	// get the sibling key
	memcpy(&siblingKey, sibling.buffer+sizeof(RecordId), sizeof(int));
	
	// REMEMBER TO SET THE NExTNODE POINTER
	//setNextNodePtr(); // to what?
	
	// for sibling as well
	PageId myNextNodePtr;
	memcpy(&myNextNodePtr, buffer+NEXTNODE_LOC, sizeof(int));
	sibling.setNextNodePtr(myNextNodePtr);
	
	return 0;
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equality to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{
	// keep going until eid.key is greater than searchkey
	bool found = false;
	int probe;
	for (int i = 0; i < m_numKeys; i++) {
		memcpy(&probe, buffer+(i*ENTRY_SIZE)+sizeof(RecordId), sizeof(int));
		//printf("searchkey: %d, probe: %d, eid: %d\n", searchKey, probe, i);
		if (probe >= searchKey) {
			eid = i;
			found = true;
			break;
		}
	}
	if (found)
		return 0;
	return RC_NO_SUCH_RECORD;
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
	// TODO: if eid >= numEntries, error
	memcpy(&key, buffer+(eid*ENTRY_SIZE)+sizeof(RecordId), sizeof(int));
	memcpy(&rid, buffer+(eid*ENTRY_SIZE), sizeof(RecordId));
	return 0;
}

/*
 * Output the pid of the next sibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
	PageId nextNode;
	memcpy(&nextNode, buffer + NEXTNODE_LOC, sizeof(int));
	return nextNode;
}

/*
 * Set the pid of the next sibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	// REMEMBER TO WRITE INTO PAGEFILE BEFORE THING GOES AWAY
	memcpy(buffer + NEXTNODE_LOC, &pid, sizeof(int));
	return 0;
}

/**
    * Return the buffer string
    * @return 0 if successful. Return an error code if there is an error.
    */
char* BTLeafNode::get_buffer()
{
	return buffer;
}

// this is a debugging function
void BTLeafNode::print_buffer()
{
	int key;
	RecordId rid;
	for(int i = 0; i < m_numKeys; i++) {
		readEntry(i, key, rid);
		cout << "EID: " << i << "\tKEY: " << key;
		cout << "\tSID: " << rid.sid << "\tPID: " << rid.pid << endl;
	}
}

// this is a debugging function
void BTLeafNode::clear_buffer()
{
	// preserve type
	int type;
	memcpy(&type, buffer+TYPE_LOC, sizeof(int));
	
	// preserve parent node
	
	// preserve pointer to next
	PageId pid;
	memcpy(&pid, buffer+NEXTNODE_LOC, sizeof(int));
	for(int i = 0; i < PageFile::PAGE_SIZE; i++) {
		buffer[i] = 0;
	}
	// change numKeys
	m_numKeys = 0;
	
	// restore type
	memcpy(buffer+TYPE_LOC, &type, sizeof(int));
	
	// restore parent
	
	// restore next
	memcpy(buffer+NEXTNODE_LOC, &pid, sizeof(int));

}

// ================================================================
// ================================================================
// ================================================================
// ================================================================

BTNonLeafNode::BTNonLeafNode()
{
	m_numKeys = 0;
	m_debug = 0;
}

void BTNonLeafNode::setDebug(int num)
{
	if (num)
		m_debug = true;
	else
		m_debug = false;
}

void BTNonLeafNode::toggleDebug()
{
	if (!m_debug)
		m_debug = true;
	else
		m_debug = false;

}

void BTNonLeafNode::debugOn()
{
	m_debug = true;
}

void BTNonLeafNode::debugOff()
{
	m_debug = false;
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
	// read 1024 bytes into buffer from pf.pid
	RC rc;
	// check for invalid page id
	rc = pf.read(pid, buffer);
	return rc;
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
	// write 1024 bytes pf.pid from buffer
	RC rc;
	// check for invalid page id
	rc = pf.write(pid, buffer);
	return rc;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	return m_numKeys;
}

RC BTNonLeafNode::readEntry(int eid, int& key, int& pid)
{
	// TODO: if eid >= numEntries, error
	memcpy(&key, buffer+(eid*ENTRY_SIZE), sizeof(int));
	memcpy(&pid, buffer+(eid*ENTRY_SIZE)+sizeof(int), sizeof(int));
	return 0;
}

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
	// TODO: check for duplicates
	// we only want to insert into the non-leaf node
	// the leftmost pointer never changes
	// check if the nonLeafNode is full
	if (m_numKeys == MAX_ENTRIES)
		return RC_NODE_FULL;
	
	int eid = -1;
	//find the first key that is greater than key (eid should be correct)
	int probe = 0;
	do {
		//basically the same as readEntry
		eid++;
		//get the key
		memcpy(&probe, buffer+(eid*ENTRY_SIZE), sizeof(int));
		if (m_debug) printf("EID %d, probe %d\n", eid, probe);
	} while (probe < key && eid < m_numKeys);
	if (m_debug) printf("inserting KEY: %d PID:%d into %d\n",
		key, pid, eid);
	
	// size of data to shift = ALL ENTRY DATA - STUFF BEFORE INSERTION - HEADER SIZE
	// HEADER SIZE because we don't want to overwrite the header info
	int dataSize = MAX_ENTRIES*ENTRY_SIZE - eid*ENTRY_SIZE - HEADER_SIZE;
	
	//if there's overflow, do stuff?
	
	//shift everything to the right ENTRY_SIZE spaces (buffer+(eid*ENTRY_SIZE))
	char* temp;
	temp = new char[dataSize];
	memcpy(temp, buffer+(eid*ENTRY_SIZE), dataSize);
	memcpy(buffer+((eid+1)*ENTRY_SIZE), temp, dataSize);
	delete [] temp;
	//insert key into eid
	memcpy(buffer+(eid*ENTRY_SIZE), &key, sizeof(int));
	//insert the page into eid
	memcpy(buffer+(eid*ENTRY_SIZE)+sizeof(int), &pid, sizeof(int));
	
	//change the number of entries
	int numKeys = m_numKeys;
	numKeys++;
	m_numKeys = numKeys;
	//put the number of entries into the buffer
	memcpy(buffer+NUMKEY_LOC, &numKeys, sizeof(int));
	
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
	// TODO: check for duplicates
	// TEST: odd number, even number,  out of order
	if (m_debug) cerr << "SPLITSPLITSPLIT" << endl;
	char tempBuffer[PageFile::PAGE_SIZE+ENTRY_SIZE]; // temporarily overfilled buffer
	
	int eid = -1;
	// find the first key that is greater than key (eid should be correct)
	int probe = 0;
	do {
		// basically the same as readEntry
		eid++;
		// get the key
		memcpy(&probe, buffer+(eid*ENTRY_SIZE), sizeof(int));
		if (m_debug) printf("EID %d, probe %d\n", eid, probe);
	} while (probe < key && eid < m_numKeys);
	printf("inserting KEY: %d PID:%d into %d\n",
		key, pid, eid);
	
	int dataSize = PageFile::PAGE_SIZE - eid*ENTRY_SIZE; // size of the data we want to shift
	
	// put it all in the temp buffer
	memcpy(tempBuffer, buffer, PageFile::PAGE_SIZE);
	
	// shift everything to the right ENTRY_SIZE spaces (buffer+(eid*ENTRY_SIZE))
	char* temp;
	temp = new char[dataSize];
	memcpy(temp, tempBuffer+(eid*ENTRY_SIZE), dataSize);
	memcpy(tempBuffer+((eid+1)*ENTRY_SIZE), temp, dataSize);
	delete [] temp;
	
	// insert the record, key into eid
	memcpy(tempBuffer+(eid*ENTRY_SIZE), &key, sizeof(int));
	memcpy(tempBuffer+(eid*ENTRY_SIZE)+sizeof(int), &pid, sizeof(int));
	
	// change the number of entries
	int numKeys = m_numKeys;
	numKeys++;
	int firstKeys = numKeys / 2;
	int secondKeys = numKeys - firstKeys - 1;
	
	// split
	int splitLocation = numKeys / 2;
	if (m_debug) {
		cerr << "SPLIT LOCATION: " << splitLocation << endl;
		print_buffer();
	}
	
	// first half goes into current buffer
	// no middle entry
	dataSize = (splitLocation)*ENTRY_SIZE;
	memcpy(buffer, tempBuffer, dataSize);
	
	// get middle entry
	int midPid;
	memcpy(&midKey, tempBuffer+(splitLocation*ENTRY_SIZE), sizeof(int));
	memcpy(&midPid, tempBuffer+(splitLocation*ENTRY_SIZE)+sizeof(int), sizeof(int));
	
	// second half goes into sibling buffer
	// sibling has already been constructed
	// sibling's PREV pointer is middle's pid
	dataSize = PageFile::PAGE_SIZE - dataSize - ENTRY_SIZE; // we don't want the mid entry
	memcpy(sibling.buffer, tempBuffer+((splitLocation+1)*ENTRY_SIZE),
		dataSize);
	
	// update the number of keys
	m_numKeys = firstKeys;
	sibling.m_numKeys = secondKeys;
	
	// put that into the buffer
	memcpy(buffer+NUMKEY_LOC, &firstKeys, sizeof(int));
	memcpy(sibling.buffer+NUMKEY_LOC, &secondKeys, sizeof(int));
	
	// update the prevpointer of the sibling
	memcpy(sibling.buffer+PREVNODE_LOC, &midPid, sizeof(int));
	
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
	// TODO: NEEDS WORK NOT OPTIMAL
	bool found = false;
	// special case, searchKey is lower than the first pid
	int pKey, pKey2, pPid, pPid2;
	readEntry(0, pKey, pPid);
	if (searchKey < pKey) {
		memcpy(&pid, buffer+PREVNODE_LOC, sizeof(int));
		return 0;
	}
	
	// keep going until eid.key is less than searchkey
	// and greater than the next key
	for (int i = 0; i < m_numKeys; i++) {
		readEntry(i, pKey, pPid);
		if (m_debug) printf("searchkey: %d, probe: %d, eid: %d\n", searchKey, pKey, i);
		// pKey <= searchKey <= pKey2
		if (pKey <= searchKey) {
			if (i < m_numKeys - 1) {
				readEntry(i+1, pKey2, pPid2);
				if (m_debug) printf("searchkey: %d, probe2: %d, eid: %d\n", searchKey, pKey2, i+1);
				if (pKey2 > searchKey) {
					pid = pPid;
					return 0;
				}
			} // >=
			else {
				pid = pPid;
				return 0;
			}
			pid = pPid;
		}
	}
	return 0;
	// greater than
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
	memcpy(buffer+PREVNODE_LOC, &pid1, sizeof(int)); // set prevnode
	memcpy(buffer, &key, sizeof(int));
	memcpy(buffer+sizeof(int), &pid2, sizeof(int)); // set nextnode
	return 0;
}

/*
    * Return the buffer string
    * @return 0 if successful. Return an error code if there is an error.
    */
char* BTNonLeafNode::get_buffer()
{
	return buffer;
}

void BTNonLeafNode::print_buffer()
{
	int key;
	int pid;
	for(int i = 0; i < m_numKeys; i++) {
		readEntry(i, key, pid);
		cout << "EID: " << i << "\t\tKEY: " << key << "\t\tPID: " << pid << endl;
	}
}

void BTNonLeafNode::clear_buffer()
{
	// preserve type
	int type;
	memcpy(&type, buffer+TYPE_LOC, sizeof(int));
	
	// preserve parent node
	
	// preserve pointer to prev
	PageId pid;
	memcpy(&pid, buffer+PREVNODE_LOC, sizeof(int));
	
	// zero it out
	for(int i = 0; i < PageFile::PAGE_SIZE; i++) {
		buffer[i] = 0;
	}
	// change numKeys
	m_numKeys = 0;
	// it has already been changed within the buffer
	
	// restore type
	memcpy(buffer+TYPE_LOC, &type, sizeof(int));
	
	// restore parent
	
	// restore prev
	memcpy(buffer+PREVNODE_LOC, &pid, sizeof(int));
	
}

// what else is there to add?

//

//

//

//

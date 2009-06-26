#include <algorithm>
#include "BTreeNode.h"

using namespace std;

// Less than operator for BTLeafNodeElemets.  Used to sort the entries in the leaf node
bool operator < (const BTLeafNodeEntry& e1, const BTLeafNodeEntry& e2)
{
	return (e1.key < e2.key);
}

BTLeafNode::BTLeafNode()
{
	numEntries = 0;
	nextLeaf = -1;
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
	char buffer[PageFile::PAGE_SIZE];
	int rc;
	if ((rc = pf.read(pid, buffer)) < 0)
		return rc;
	memcpy(this, buffer, PageFile::PAGE_SIZE);
	
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
	
	char buffer[PageFile::PAGE_SIZE];
	int rc;
	memcpy(buffer, this, PageFile::PAGE_SIZE);
	if ((rc = pf.write(pid, buffer)) < 0)
		return rc;
	
	return 0; 
	
	
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
	return numEntries; 
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
	// Check to see if the node is full
	if (numEntries == maxEntries)
		return RC_NODE_FULL;
	
	
	BTLeafNodeEntry newEntry;
	newEntry.key = key;
	newEntry.rid = rid;
	entries[numEntries] = newEntry;
	numEntries++;
	sort(entries, entries + numEntries);
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
	
	BTLeafNodeEntry overflow[numEntries+1]; // Create a temperary array to hold all the entries plus the newly inserted entry
	int splitPoint;

	for (int i=0; i<numEntries; i++)
		overflow[i] = entries[i]; 
	// Place the new entry in the end of the array
	overflow[numEntries].key = key;
	overflow[numEntries].rid = rid;
	// Sort the array
	sort(overflow, overflow + numEntries+1);
	
	// Split the array in half and put the second half into sibling
	if ((numEntries+1) % 2 == 0)
		splitPoint = (numEntries+1)/2;
	else
		splitPoint = ((numEntries+1)/2) + 1;
	
	// Put the first half back into the origional node
	for (int i=0; i<splitPoint; i++)
	{
		BTLeafNodeEntry currentEntry = overflow[i];
		entries[i] = currentEntry;
	}
	// Put the second half into the sibling node
	for (int i=splitPoint; i<numEntries+1; i++)
	{
		BTLeafNodeEntry currentEntry = overflow[i];
		sibling.insert(currentEntry.key, currentEntry.rid);
	}
	
	numEntries = splitPoint; // Update the number of entries in the origional node
	
	// Set the key to output and set the sibling's nextLeaf value
	siblingKey = sibling.entries[0].key;
	sibling.nextLeaf = nextLeaf;

	
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
	for (int i=0; i<numEntries; i++)
	{
		BTLeafNodeEntry currentEntry = entries[i];
		if (currentEntry.key >= searchKey)
		{
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
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{ 
	BTLeafNodeEntry entry;
	entry = entries[eid];
	key = entry.key;
	rid = entry.rid;
	
	return 0; 
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	
	return nextLeaf;
	
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	nextLeaf = pid;
	return 0;
}

/**BTNonLeafNode function definitions */

// Less than operator for BTLeafNodeElemets.  Used to sort the entries in the leaf node
bool operator < (const BTNonLeafNodeEntry& e1, const BTNonLeafNodeEntry& e2)
{
	return (e1.key < e2.key);
}

BTNonLeafNode::BTNonLeafNode()
{
	numEntries = 0;
	lastPid = -1;
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
	char buffer[PageFile::PAGE_SIZE];
	int rc;
	if ((rc = pf.read(pid, buffer)) < 0)
		return rc;
	memcpy(this, buffer, PageFile::PAGE_SIZE);
	
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
	char buffer[PageFile::PAGE_SIZE];
	int rc;
	memcpy(buffer, this, PageFile::PAGE_SIZE);
	if ((rc = pf.write(pid, buffer)) < 0)
		return rc;
	
	return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	 return numEntries; 
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 	// Check to see if the node is full
	if (numEntries == maxEntries)
		return RC_NODE_FULL;
	
	BTNonLeafNodeEntry newEntry;
	newEntry.key = key;
	newEntry.pid = pid;
	entries[numEntries] = newEntry;
	numEntries++;
	sort(entries, entries + numEntries);
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
	BTNonLeafNodeEntry overflow[numEntries+1]; // Create a temperary array to hold all the entries plus the newly inserted entry
	int splitPoint;

	for (int i=0; i<numEntries; i++)
		overflow[i] = entries[i]; 
	// Place the new entry in the end of the array
	overflow[numEntries].key = key;
	overflow[numEntries].pid = pid;
	// Sort the array
	sort(overflow, overflow + numEntries+1);
	
	splitPoint = (numEntries+1)/2;
	
	// Put the first half back into the origional node
	for (int i=0; i<splitPoint; i++)
		entries[i] = overflow[i];
		
	// Put the second half into the sibling node, excluding the middle entry
	for (int i=splitPoint+1; i<numEntries+1; i++)
	{
		BTNonLeafNodeEntry currentEntry = overflow[i];
		sibling.insert(currentEntry.key, currentEntry.pid);
	}
	
	midKey = overflow[splitPoint].key;
	numEntries = splitPoint; // Update the number of entries in the origional node
	
	// Set the key to output and set the sibling's nextLeaf value


	
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
	
	for(int i = 0; i < numEntries; i++)
	{
		if (searchKey < entries[i].key)
		{
			pid = entries[i].pid;
			return 0;
		}
	}
	
	pid = lastPid;
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
	
	numEntries = 1;
	entries[0].key = key;
	entries[0].pid = pid1;
	lastPid = pid2;
	return 0; 
	
}

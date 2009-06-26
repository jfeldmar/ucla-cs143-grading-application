#include "BTreeNode.h"

using namespace std;

/**
	* Constructor: initializes the private members
	*/
BTLeafNode::BTLeafNode()
{
	keyCount = 0;
}

/**
 * Read the contents of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{
	// Read the page file at the specified pid
	return pf.read( pid, buffer);
}

/**
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{
	// Write the buffer to the page file at pid
	return pf.write( pid, buffer);
}

/**
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	return keyCount;
}

/**
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
	// Check if the node is already full
	if ( getKeyCount() >= MAX_KEYS )
		return RC_NODE_FULL;

	// Locate the correct entry
	int eid;
	if ( locate( key, eid) == RC_NO_SUCH_RECORD )
	{
		// Set the entry at the end of all the other keys
		setEntry( getKeyCount(), key, rid);

		// Increment the key count
		++keyCount;

		// Return success
		return 0;
	}

	// Create a backup buffer
	char backup[BLOCK_SIZE];

	// Copy the buffer into the backup
	memcpy( backup, buffer, BLOCK_SIZE);

	// Set the entry at the specified location
	setEntry( eid, key, rid);

	// Copy the necessary portion backup buffer into the buffer
	memcpy( buffer + 8 * eid + 8, 
			backup + 8 * eid, 
			(getKeyCount() - eid) * 8
		);

	// Increment the key count
	++keyCount;

	// Return successful
	return 0;
}

/**
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
	// Check if the node is already full
	if ( getKeyCount() >= MAX_KEYS )
		return RC_NODE_FULL;

	// Check if rid is valid
	if ( rid <= 0 )
		return RC_INVALID_RID;

	// Insert the (key, rid) pair to the node
	insert( key, rid);

	// Find the middle entry
	int middleEid = (int) ceil(getKeyCount() / 2.0);
	int lastEid = getKeyCount() - 1;

	// Delete the keys past the middle
	keyCount = keyCount - (lastEid - middleEid);

	// Temp variables
	int tempKey;
	RecordId tempRid;

	// Loop through each value and copy it to the new sibling
	for (int i = middleEid, int k = 0; i < lastEid; i++, k++)
	{
		readEntry( i, tempKey, tempRid);
		sibling.setEntry( k, tempKey, tempRid);
	}

	// Set the sibling key
	sibling.readEntry( 0, tempKey, tempRid);
	siblingKey = tempKey;

	return 0;
}

/**
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{
	// Since all the keys are sorted, consider using 
	// binary search to increase efficiency

	// Create pointers
	void *keyPtr = buffer + 4; // First key starts at 4
	void *endPtr = buffer + 8 * MAX_KEYS;

	// Go through each key
	for (; keyPtr < endPtr; keyPtr + 8)
	{
		if ( *keyPtr >= searchKey )
		{
			// Assign the eid
			eid = keyPtr - buffer;

			// Return successful
			return 0;
		}
	}

	// Return error: could not find such a key
	return RC_NO_SUCH_RECORD;
}

/**
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{
	// Get the location of the pair in the buffer
	void *entryPtr = buffer + 8 * eid;

	// Assign the key
	key = *(entryPtr + 4);

	// Assign the rid
	rid = *entryPtr;

	// Return successful
	return 0;
}

/**
 * Set the (key, rid) pair at the eid entry.
 * @param eid[IN] the entry number to set the (key, rid) pair as
 * @param key[OUT] the key for the entry
 * @param rid[OUT] the RecordId for the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setEntry(int eid, int key, RecordId rid)
{
	// Get the location of the pair in the buffer
	void *entryPtr = buffer + 8 * eid;

	// Assign the key
	*(entryPtr + 4) = key;

	// Assign the rid
	*entryPtr = rid;

	// Return successful
	return 0;
}

/**
 * Set the (key, rid) pair at the eid entry and shift 
 * any entries after the given eid to the right
 * @param eid[IN] the entry number to set the (key, rid) pair as
 * @param key[OUT] the key for the entry
 * @param rid[OUT] the RecordId for the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::shiftAndSetEntry(int eid, int key, RecordId rid)
{
	/// TODO: Verify this works in all cases

	// Backup the source
	char backup[BLOCK_SIZE];
	memcpy( backup, buffer, BLOCK_SIZE);

	// Get the start pointers
	void *bufferPtr = buffer + 8 * eid;
	void *backupPtr = backup + 8 * eid + 8; // 1 (key, rid) pair ahead

	// Get the end pointer
	void *endPtr = buffer + 8 * MAX_KEYS;

	// Shift all the elements in the backup after and including eid
	for (; bufferPtr < endPtr; bufferPtr++, backupPtr++)
	{
		*backupPtr = *bufferPtr;
	}

	// Copy the backup back into the buffer
	memcpy( buffer, backup, BLOCK_SIZE);

	// Set the (key, rid) pair at the eid
	setEntry( eid, key, rid);

	// Return successful
	return 0;
}

/**
 * Output the pid of the next slibling node.
 * @return the PageId of the next sibling node. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
	// Get the next pointer
	void *nextPtr = buffer + 8 * MAX_PTRS - 4;

	// Return the value of the next pointer
	return *nextPtr;
}

/**
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	// Check for an invalid PageId
	if ( pid < 0 )
		return RC_INVALID_PID;

	// Get the next pointer
	void *nextPtr = buffer + 8 * MAX_PTRS - 4;

	// Set the next pointer value
	*nextPtr = pid;

	// Return successful
	return 0;
}

/*****************************************************************************************/

/**
 * Read the contents of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{
	return pf.read( pid, buffer);
}
    
/**
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{
	return pf.write( pid, buffer);
}

/**
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	return keyCount;
}


/**
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
	// Check if the node is already full
	if ( getKeyCount() >= MAX_KEYS )
		return RC_NODE_FULL;

	// Create pointers
	void *curPtr = buffer;
	void *endPtr = buffer + 8 * MAX_KEYS;

	// Go through each key
	for (; curPtr < endPtr; curPtr + 8)
	{
		if ( key < *(curPtr + 4) )
		{
			break;
		}
	}

	// Create a backup buffer
	char backup[BLOCK_SIZE];

	// Copy the buffer into the backup
	memcpy( backup, buffer, BLOCK_SIZE);

	// Assign thekey and page id
	*curPtr = pid;
	*(curPtr + 4) = key;

	// Copy the necessary portion backup buffer into the buffer
	memcpy( curPtr + 8, 
			backup + (curPtr - buffer) * 8, 
			(endPtr - (curPtr + 8)) * 8
		);

	// Increment the key count
	++keyCount;

	// Return successful
	return 0;
}

/**
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
	// Check if the node is already full
	if ( getKeyCount() >= MAX_KEYS )
		return RC_NODE_FULL;

	// Insert the (key, rid) pair to the node
	insert( key, pid);

	// Find the middle entry
	int middleEid = (int) ceil(getKeyCount() / 2.0);
	int lastEid = getKeyCount() - 1;

	// Delete the keys past the middle
	keyCount = keyCount - (lastEid - middleEid);

	// Temp variables
	int tempKey;
	RecordId tempRid;

	// Set the midKey
	sibling.readEntry( middleEid, tempKey, tempRid);
	midKey = tempKey;
	++middleEid;

	// Loop through each value and copy it to the new sibling
	for (int i = middleEid, int k = 0; i < lastEid; i++, k++)
	{
		readEntry( i, tempKey, tempRid);
		sibling.setEntry( k, tempKey, tempRid);
	}

	return 0;
}

/**
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{
	// Create pointers
	void *keyPtr = buffer + 4; // First key starts at 4
	void *endPtr = buffer + 8 * MAX_KEYS;

	// Go through each key
	for (; keyPtr < endPtr; keyPtr + 8)
	{
		if ( searchKey < *keyPtr )
		{
			// Asssign the eid
			pid = *(keyPtr - 4);
	
			// Return success
			return 0;
		}
	}

	// Then the search key is greater than all the rest
	pid = *(keyPtr + 4);

	// return success
	return 0;
}

/**
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{
	if ( pid1 < 0 || pid2 < 0 )
		return RC_INVALID_PID;

	// Create the pointer
	void *rootPtr = buffer;

	// Set the values
	*(rootPtr + 0) = pid1;
	*(rootPtr + 4) = key;
	*(rootPtr + 8) = pid2;

	// return successful
	return 0; 
}

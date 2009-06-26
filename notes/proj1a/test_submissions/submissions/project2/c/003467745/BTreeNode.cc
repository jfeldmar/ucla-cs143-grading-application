#include "BTreeNode.h"

#include <iostream>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

BTLeafNode::BTLeafNode()
{
	MAX_KEYS = 82;
	MAX_PTRS = 83;

	// Initialize the size & next node ptr
	setNextNodePtr(-1);
	buffer[255] = 0;

	// Set no sibling nodes
	setNextNodePtr(-1);
}

// Print the Node if DEBUG equals 1
void BTLeafNode::print()
{
	if (DEBUG)
	{
		ofstream myfile ("output.txt", ios::app);
		if (!myfile.is_open())
			return;
	
		myfile << "[ keyCount:" << getKeyCount() << " ";
		for(int eid = 0; eid < getKeyCount() && eid < MAX_KEYS; eid++)
		{
			int *eidPtr = buffer + 3 * eid;
			myfile << "| rid.pid:" << *(eidPtr + 0) 
				<< " | rid.sid:" << *(eidPtr + 1) 
				<< " | key:" << *(eidPtr + 2) 
				<< " ";
		}
		myfile << " | sibling:" << getNextNodePtr() << " ]" << endl;
	
		myfile.close();
	}
}

/**
 * Read the content of the node into buffer 
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
	return buffer[255];
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
	locate( key, eid);
	
	int *curPtr = buffer;
	int *endPtr = buffer + 3 * MAX_KEYS;
	
	// Place the key,rid pair at the end
	if ( eid < 0 )
	{
		curPtr = buffer + getKeyCount() * 3;
		*(curPtr + 0) = rid.pid;
		*(curPtr + 1) = rid.sid;
		*(curPtr + 2) = key;
	}
	
	// Else Shift and insert the (key,rid) pair at entry eid
	// Backup the source
	int backup[BLOCK_SIZE / sizeof(int)];
	memcpy( backup, buffer, BLOCK_SIZE);
	
	// Insert the new node
	curPtr = buffer + 3 * eid;
	*(curPtr + 0) = rid.pid;
	*(curPtr + 1) = rid.sid;
	*(curPtr + 2) = key;
	
	memcpy(curPtr + 3,
		   backup + 3 * eid,
		   (endPtr-curPtr) * 4);
	
	// Increment the key count
	++buffer[255];
	
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
	// Inflate maximums
	MAX_KEYS++; MAX_PTRS++;

	// Insert the (key, rid) pair to the node
	insert( key, rid);
	
	// Find the middle key
	int middleEid = (int) ceil(getKeyCount() / 2.0);
	int *middlePtr = buffer + 3 * middleEid;
	int *endPtr = buffer + 3 * getKeyCount();
	
	// Set the sibling key
	siblingKey = *(middlePtr + 2);
	
	// Loop through each value and copy it to the new sibling
	RecordId tempRid;
	for(int eid = middleEid; eid < getKeyCount(); eid++)
	{
		int *eidPtr = buffer + 3 * eid;
		tempRid.pid = *(eidPtr + 0);
		tempRid.sid = *(eidPtr + 1);

		sibling.insert(*(eidPtr + 2), tempRid);
	}

	// Delete the keys past the middle
	buffer[255] = middleEid;

	// Return maximums to normal
	MAX_KEYS--; MAX_PTRS--;

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
	eid = 0;
	int *keyPtr = buffer + 2; // First key starts at 4
	int *endPtr = buffer + 3 * getKeyCount();
	
	// Go through each key
	for (eid = 0; eid < getKeyCount(); eid++)
	{
		int *eidPtr = buffer + 3 * eid;
		if ( *(eidPtr + 2) >= searchKey )
		{
			return 0;
		}
	}
	
	// Return error: could not find such a key
	return RC_NO_SUCH_RECORD; // error code?
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
	// Check for a bad eid provided
	if ( eid >= getKeyCount() )
	{
		return RC_NO_SUCH_RECORD;
	}

	// Get the location of the pair in the buffer
	int *entryPtr = buffer + 3 * eid;
	
	// Assign the key
	key = *(entryPtr + 2);
	
	// Assign the rid
	rid.pid = *(entryPtr + 0);
	rid.sid = *(entryPtr + 1);
	
	// Return successful
	return 0;
}

/**
 * Set the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[IN] the key from the slot
 * @param rid[IN] the RecordId from the slot
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setEntry(int eid, int& key, PageId& pid, int& sid)
{
	// Get the location of the pair in the buffer
	int *entryPtr = buffer + 3 * eid;
	
	// Assign the key
	*(entryPtr + 2) = key;
	
	// Assign the rid
	*(entryPtr + 0) = pid;
	*(entryPtr + 1) = sid;
	
	// Return successful
	return 0;
}

/**
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
	// Return the value of the next pointer
	return buffer[254];
}

/**
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	// Set the next pointer value
	buffer[254] = pid;

	// Check for bad pid
	if ( pid < 0 )
		return RC_INVALID_PID;

	// Return successful
	return 0;
}

/*############################################################################################*/
BTNonLeafNode::BTNonLeafNode()
{
	MAX_KEYS = 125;
	MAX_PTRS = 126;

	// Initialize the size
	buffer[255] = 0;
}

// Print the Node if DEBUG equals 1
void BTNonLeafNode::print()
{
	if (DEBUG)
	{
		ofstream myfile ("output.txt", ios::app);
		if (!myfile.is_open())
			return;
	
		for(int eid = 0; eid < getKeyCount() && eid < MAX_KEYS; eid++)
		{
			int *eidPtr = buffer + 2 * eid;
			myfile << "| p:" << *(eidPtr + 0) 
				<< " | k:" << *(eidPtr + 1) 
				<< " |";
		}
		int *lastPidPtr = buffer + 2 * getKeyCount();
		myfile << "| p:" << *lastPidPtr << " ]" << endl;
	}
}

/**
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{
	return pf.read(pid, buffer);
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
	return pf.write(pid, buffer);
}

/**
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	return buffer[255];
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
	int *curPtr = buffer;
	int *endPtr = buffer + 2 * getKeyCount() + 1;

	// Go through each key
	for (; curPtr < endPtr; curPtr=curPtr+2)
	{
		if ( key < *(curPtr + 1) )
		{
			break;
		}
	}

	// Increment the key count
	++buffer[255];

	// If its the last one, insert it at the end
	if ( curPtr >= (endPtr-1) )
	{
		*(endPtr + 1) = pid;
		*(endPtr + 0) = key;
		return 0;
	}

	// Else Shift and insert the (key,rid) pair at entry eid
	// Backup the source
	int backup[BLOCK_SIZE / sizeof(int)];
	memcpy( backup, buffer, BLOCK_SIZE);
	
	// Insert the new node
	if ( curPtr == buffer )
	{
		*(curPtr + 0) = pid;
		*(curPtr + 1) = key;
	}
	else
	{
		++curPtr;
		*(curPtr + 1) = pid;
		*(curPtr + 0) = key;
	}
	
	memcpy(curPtr + 2,
		   backup + (curPtr - buffer),
		   (endPtr-curPtr) * 4);

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
	// Inflate maximums
	MAX_KEYS++; MAX_PTRS++;

	// Insert the (key, rid) pair to the node
	insert( key, pid);

	// Find the middle key
	int *middlePtr = buffer + 2 * ((int) (getKeyCount() / 2.0)) + 1;
	int *endPtr = buffer + 2 * getKeyCount();

	// Delete the keys past the middle
	buffer[255] = (int) (getKeyCount() / 2.0);

	// Set the midKey
	midKey = *middlePtr;
	++middlePtr;

	// Initialize the root
	sibling.initializeRoot( *middlePtr, *(middlePtr+1), *(middlePtr+2));
	middlePtr = middlePtr + 3;

	// Loop through each value and copy it to the new sibling
	for( ; middlePtr < endPtr; middlePtr=middlePtr+2)
	{
		sibling.insert(*(middlePtr), *(middlePtr+1));
	}

	// Inflate maximums
	MAX_KEYS--; MAX_PTRS--;

	return 0;
}

void BTNonLeafNode::setValue(int location, int value)
{
	*(buffer + location) = value;
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
	int *keyPtr = buffer + 1; // First key starts after the first ptr
	int *endPtr = buffer + 2 * getKeyCount() + 1;

	// Go through each key
	for (; keyPtr < endPtr; keyPtr = keyPtr+2)
	{
		if ( searchKey < *keyPtr )
		{
			// Asssign the eid
			pid = *(keyPtr - 1);

			// Return success
			return 0;
		}
	}

	// Then the search key is greater than all the rest
	pid = *(keyPtr - 1);

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
	int *rootPtr = buffer;

	// Set the values
	*(rootPtr + 0) = pid1;
	*(rootPtr + 1) = key;
	*(rootPtr + 2) = pid2;

	// Set the key count
	buffer[255] = 1;

	// return successful
	return 0;
}

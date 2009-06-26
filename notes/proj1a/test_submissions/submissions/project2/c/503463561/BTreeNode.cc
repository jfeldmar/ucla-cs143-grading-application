#include <iostream>
#include <cstring>
#include "BTreeNode.h"
#include "PageFile.h"

using namespace std;

/*
 * Binary search helper function for use by insertion and search methods
 * @param base the array to search
 * @param item a temporary structure holding our key of interest
 * @param num the number of items to search; MUST be nonzero!
 * @param size the size of each item
 * @param comparator the function to use when making comparisons
 * @param result the variable to store the result of search, or
 * 		 index of the closest key with value less than the actual key
 * @return 0 if successful match, < 0 if the key is less than the index
 * 		  returned, and > 0 if the key is greater than the index returned.
 */
int binary_search_index( const void* base, const void* item, size_t num, 
	size_t size, int(*comparator)(const void*, const void*), int &result )
{
	char *array = (char*)base; // Convert pointer to char pointer
	int first = 0, last = num-1, mid; // First, last, and middle indices
	int retval; // Return value of comparator

	//cerr << "== PERFORMING BINARY SEARCH! ==" << endl;

	while ( first <= last ) {
		mid = (first + last) / 2; // Get midpoint

		retval = (*comparator)(item,array+mid*size); // Get comparison
		//cerr << "F: " << first << " L: " << last << " MID: " << mid
		//	  << " RV: " << retval << endl;

		// Is item > middle of array?
		if (retval > 0)
			first = mid+1; // Repeat search in top half
		// Is item < middle of array?
		else if (retval < 0)
			last = mid-1; // Repeat search in bottom half
		else
			break;
	}

	result = mid; // Set result to our middle index
	return retval; // Return the comparator's value
}

/*
 * Helper function to quickly insert an item in-place in a sorted array
 * and copy forward all other entries. PRECONDITION: sorted array has enough
 * room for one additional entry!
 * @param base the array to insert
 * @param item a temp structure holding the item of interest
 * @param num the number of items in the array
 * @param size the size of each item
 * @param comparator the function to use when making comparisons
 * @return 0 if successful, error code on error
 */

int quick_insert( void* base, const void* item, size_t num,
	size_t size, int(*comparator)(const void*, const void*))
{
	char *array = (char*)base; // Convert pointer to char pointer
	int index; // Index that results from binary search
	int retval;

	// If num = 0, we simply set index = 0
	if (num == 0)
		index = 0;
	else {
		retval = binary_search_index(base,item,num,
			size,comparator,index); // Perform a binary search on the array
		// Adjust eid appropriately: if retval >= 0, then key >= m_ents[index].key
		// and we want to insert *after* this entry.
		if (retval >= 0)
			index++;
		}
	// Shift the array right one entry
	for (int j = num; j > index; j--)
		memcpy(array+j*size,array+(j-1)*size,size);
	// Insert our new entry
	memcpy(array+index*size,item,size);

	// Everything should be OK!
	return 0;
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
   char page[PageFile::PAGE_SIZE]; // Buffer for holding our page
   char *pp; // Pointer to location in page
   RC rc; // Temporary storage for return value

   // Read in the page conaining our node
   if ( (rc = pf.read(pid, page)) < 0) return rc;

   pp = page; // Set the pointer to the beginning of our page

   // Read in the key count and next leaf node
   memcpy(&m_keyCount, pp, sizeof(int));
   pp += sizeof(int);

   // Read in the pointer to the next leaf node
   memcpy(&m_next, pp, sizeof(PageId));
   pp += sizeof(PageId);

   // Now read in the array of leaf node entries, up to the last node added
   memcpy(m_ents,pp,sizeof(LeafNodeEnt)*m_keyCount);

   return 0; // Success!
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
   char page[PageFile::PAGE_SIZE]; // Buffer for holding our page
   char *pp; // Pointer to location in page
   RC rc; // Temporary storage for return value

   pp = page; // Set the pointer to the beginning of our page

   // Write keycount to the buffer
   memcpy(pp, &m_keyCount, sizeof(int));
   pp += sizeof(int);

   // Write pointer to next leaf node in buffer
   memcpy(pp, &m_next, sizeof(PageId));
   pp += sizeof(PageId);

   // Now write in the array of leaf node entries, up to the last node added
   memcpy(pp, m_ents, sizeof(LeafNodeEnt)*m_keyCount); 

   // Write the page containing our node to disk
   if ( (rc = pf.write(pid,page)) < 0 ) return rc;

   return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ return m_keyCount; }

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
   // First check that node isn't full
   if (m_keyCount == N_KEYS)
      return RC_NODE_FULL;

	// Perform a quick insert of the new key
	LeafNodeEnt temp;
	temp.key = key;
	temp.rid = rid;
	
	quick_insert( m_ents, &temp, m_keyCount, sizeof(LeafNodeEnt), LeafNodeEnt::comparator);
   m_keyCount++;

   return 0; // Success!
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in midKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param midKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& midKey)
{
	// Create a temporary nonleaf node entry
	LeafNodeEnt temp;
	temp.key = key;
	temp.rid = rid;

	// Insert this node into our m_ents array
	quick_insert( m_ents, &temp, m_keyCount, sizeof(LeafNodeEnt), LeafNodeEnt::comparator);

	// Update key counts
   sibling.m_keyCount = (m_keyCount + 1) / 2;
   m_keyCount = (m_keyCount + 1) - sibling.m_keyCount;

   // Copy half of the results into the sibling array
   memcpy( sibling.m_ents, m_ents + m_keyCount, sizeof(LeafNodeEnt)*sibling.m_keyCount);

   // Save the first key of the sibling array in our midKey variable
   midKey = sibling.m_ents->key;

   // Now return 0, we're done!
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
	// Create a temporary entry to use for comparison
	LeafNodeEnt temp;
	temp.key = searchKey;
	// Temporary return value
	int retval;

	// If we don't have any keys in this node, return error
	if (!m_keyCount)
		return RC_NO_SUCH_RECORD;
	
	retval = binary_search_index( m_ents, &temp, m_keyCount, sizeof(LeafNodeEnt), 
		LeafNodeEnt::comparator, eid );
	
	if (retval) return RC_NO_SUCH_RECORD;
	else return 0;
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
   // Attempting to read an invalid  entry will result in error
   if (eid < 0 || eid >= m_keyCount)
      return RC_INVALID_CURSOR;
   key = m_ents[eid].key;
   rid = m_ents[eid].rid;
   return 0; // Done!
}

/*
 * Output the pid of the next sibling node.
 * @return m_next if successful.
 */
PageId BTLeafNode::getNextNodePtr()
{
   return m_next;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{  
   m_next = pid;
   return 0;
}

/*
 * Print the contents of our node for debugging.
 */

void BTLeafNode::print()
{
   cout << "KC: " << m_keyCount
        << " Next: " << m_next
        << " Ents: ";
   for (int i = 0; i < m_keyCount; i++)
      cout << "(" << m_ents[i].key << " => (" 
           << m_ents[i].rid.pid << "," << m_ents[i].rid.sid << ")) ";
   cout << endl;
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
   char page[PageFile::PAGE_SIZE]; // Buffer for holding our page
   char *pp; // Pointer to location in page
   RC rc; // Temporary storage for return value

   // Read in the page conaining our node
   if ( (rc = pf.read(pid, page)) < 0) return rc;

   pp = page; // Set the pointer to the beginning of our page

   // Read in the key count
   memcpy(&m_keyCount, pp, sizeof(int));
   pp += sizeof(int);

   // Read in the first page id
   memcpy(&m_first, pp, sizeof(PageId));
   pp += sizeof(PageId);

   // Now read in the array of non leaf node entries, up to the last node added
   memcpy(m_ents,pp,sizeof(NonLeafNodeEnt)*m_keyCount);

   return 0; // Success!
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
   char page[PageFile::PAGE_SIZE]; // Buffer for holding our page
   char *pp; // Pointer to location in page
   RC rc; // Temporary storage for return value

   pp = page; // Set the pointer to the beginning of our page

   // Write key count to the buffer
   memcpy(pp, &m_keyCount, sizeof(int));
   pp += sizeof(int);

   // Write first page id to the buffer
   memcpy(pp, &m_first, sizeof(PageId));
   pp += sizeof(PageId);

   // Now write in the array of non leaf node pointers, up to the last node added
   memcpy(pp, m_ents, sizeof(NonLeafNodeEnt)*m_keyCount); 

   // Write the page containing our node to disk
   if ( (rc = pf.write(pid,page)) < 0 ) return rc;

   return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
   return m_keyCount;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 
   // First check that node isn't full
   if (m_keyCount == N_KEYS)
      return RC_NODE_FULL;
   
	// Perform a quick insert of the new key
	NonLeafNodeEnt temp;
	temp.key = key;
	temp.pid = pid;

	quick_insert( m_ents, &temp, m_keyCount, sizeof(NonLeafNodeEnt), NonLeafNodeEnt::comparator );
	m_keyCount++;

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
	// Create a temporary nonleaf node entry
	NonLeafNodeEnt temp;
	temp.key = key;
	temp.pid = pid;

	// Insert this node into our m_ents array
	quick_insert( m_ents, &temp, m_keyCount, sizeof(NonLeafNodeEnt), NonLeafNodeEnt::comparator);

   // Update key counts
   sibling.m_keyCount = (m_keyCount + 1) / 2 - 1;
	m_keyCount = m_keyCount - sibling.m_keyCount;

   // Save the middle key in our midKey structure
   midKey = m_ents[m_keyCount].key;

   // Save the middle key's pointer to the m_first value of the sibling's node
   sibling.m_first = m_ents[m_keyCount].pid;

   // Copy half of the results into the sibling array
   memcpy( sibling.m_ents, m_ents + m_keyCount + 1, sizeof(NonLeafNodeEnt)*sibling.m_keyCount);

   // Now return 0, we're done!
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
	// Create a temporary entry to use for comparison
	NonLeafNodeEnt temp;
	temp.key = searchKey;
	// Temporary return value
   int retval;
	// Temporary entry id
	int eid;

	// If we have no key count, return an error
	if (!m_keyCount)
		return RC_NO_SUCH_RECORD;

	retval = binary_search_index( m_ents, &temp, m_keyCount, sizeof(NonLeafNodeEnt),
		NonLeafNodeEnt::comparator, eid );

	// Adjsut eid appropriately: if retval >= 0, then key >= m_ents[eid].key
	if (retval >= 0)
		eid++; // eid must point to the smallest entry *greater than* our key!

	// If eid is the first entry, then the page id must be m_first
	if (eid == 0)
		pid = m_first;
	// Otherwise, page id will be previous entry in node
	// This will be the last pointer if the key is larger than all of our
	// entries.
	else
		pid = m_ents[eid-1].pid;
	
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
   m_keyCount = 0; // Reset the key count
   m_first = pid1; // Set m_first to be our first page id
   // Insert the key with our second page id
   if ( (rc == insert(key,pid2)) < 0 ) return rc; 
   return 0;
}

/*
 * Print the contents of a node for debugging.
 */

void BTNonLeafNode::print()
{
   cout << "KC: " << m_keyCount
        << " First: " << m_first
        << " Ents: ";
   for (int i = 0; i < m_keyCount; i++)
      cout << "(" << m_ents[i].key << " => " 
           << m_ents[i].pid << ") ";
   cout << endl;
}

/*
 * Read the (key, pid) pair from the eid entry.
 */
RC BTNonLeafNode::readEntry(int eid, int& key, PageId& pid)
{ 
   // Attempting to read an invalid  entry will result in error
   if (eid < 0 || eid >= m_keyCount) return RC_INVALID_CURSOR;
   key = m_ents[eid].key; pid = m_ents[eid].pid;
   return 0; // Done!
}


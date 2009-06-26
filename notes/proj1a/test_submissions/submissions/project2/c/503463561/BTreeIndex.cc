/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <iostream>
#include <cstring>
#include <stack>
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

static const char BTREEINDEX_MAGIC[] = "B+TI"; // Magic sequence of chars to identify file format
static const int BTREEINDEX_MAGIC_LEN = strlen(BTREEINDEX_MAGIC);

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
	 m_isOpen = false;
    m_rootPid = -1;
}

BTreeIndex::~BTreeIndex()
{
	if (m_isOpen) {
		close(); // Make sure to close the index before you go!
		m_isOpen = false;
		}
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
	RC rc; // Return value for page file functions

	// If the index is associated with some other open file,
	// first close the other file
	if (m_isOpen)
		close();

	m_filename = indexname; // Save the filename for debugging

	// First attempt to physically open the file
	if ( (rc = m_pf.open(indexname,mode)) < 0) return rc;

	// If the file size is nonzero, try to retrieve metadata from file
	if ( m_pf.endPid() ) {
	  if ( (rc = loadMetadata()) < 0) return rc; // Try to get metadata
	  cerr << "Opening existing B+ tree" << endl;
	  }
	else { 
	  if ( (rc = drop()) < 0) return rc; // Otherwise, initialize a new index file
	  cerr << "Creating new B+ tree" << endl;
	  }

	m_isOpen = true;
	return 0; // Everything's OK!
}

RC BTreeIndex::loadMetadata()
{
	RC rc; // Return value for page file functions
	char page[PageFile::PAGE_SIZE]; // Buffer for holding our page
	char *pp; // Pointer to location in page
	int nlKeys, lKeys; // Number of keys per nonleaf, leaf node in file
   // Read in the page containing the file's metadata (0 by convention)
	if ( (rc = m_pf.read(0,page)) < 0 ) return rc;
 	// Explicitly set the pointer to the beginning of page
	pp = page;
	// Check our magic number to make sure this is the correct file format
	if (strncmp( BTREEINDEX_MAGIC, pp, BTREEINDEX_MAGIC_LEN ) != 0) return RC_INVALID_FILE_FORMAT;
	pp += BTREEINDEX_MAGIC_LEN; // Increment the pointer by the correct amount
	// Read in the appropriate metadata from file
	memcpy( &m_rootPid, pp, sizeof(PageId));
	pp += sizeof(PageId);
	memcpy( &m_treeHeight, pp, sizeof(int));
	pp += sizeof(int);
	memcpy( &nlKeys, pp, sizeof(int));
	pp += sizeof(int);
	memcpy( &lKeys, pp, sizeof(int));
	pp += sizeof(int);
	memcpy( &m_nEntries, pp, sizeof(size_t));
	pp += sizeof(size_t);
	// Check for data consistency with number of keys
	if ( nlKeys != m_nln.N_KEYS || lKeys != m_ln.N_KEYS) return RC_INVALID_FILE_FORMAT;
	return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	RC rc; // Return value
	// First save metadata
	if ( (rc = saveMetadata()) < 0) return rc;
	// If successful, close the file 
	if ( (rc = m_pf.close()) < 0) return rc;
	m_isOpen = false;
	return 0;
}

/*
 * Drop the contents of the current index file and initialize a new one
 * @return error code. 0 if no error
 */
RC BTreeIndex::drop()
{
	  RC rc; // Our return value
	  m_rootPid = 1; // Set root pid to 1
	  m_treeHeight = 0; // Tree height is initially 0
	  m_nEntries = 0; // Set number of entries in our tree to 0
	  if ( (rc = saveMetadata()) < 0) return rc; // Save the metadata
	  m_ln.drop(); // Clear out the current leaf node
	  m_ln.setNextNodePtr(-1); // Set next node pointer to invalid pid
	  									// which indcates the end of the tree
	  m_curPid = -1; // Invalidate current pid
	  // Write the leaf node to our root pid (tree is currently empty)
	  if ( (rc = m_ln.write(m_rootPid,m_pf)) < 0 ) return rc; 
	  return 0;
}

/*
 * Save metadata of the B+ tree onto disk.
 * @return error code. 0 if no error
 */
RC BTreeIndex::saveMetadata()
{
	RC rc; // Return value
	char page[PageFile::PAGE_SIZE]; // Page buffer
	char *pp; // Pointer in page
	int nlKeys = m_nln.N_KEYS;
	int lKeys = m_ln.N_KEYS;
	// Zero out the page first
	memset( page, 0, PageFile::PAGE_SIZE );
 	// Explicitly set the pointer to the beginning of page
	pp = page;
	// First copy over our magic number
	strncpy( pp, BTREEINDEX_MAGIC, BTREEINDEX_MAGIC_LEN );
	pp += BTREEINDEX_MAGIC_LEN;
	// Then write out the appropriate metadata to file
	memcpy( pp, &m_rootPid, sizeof(PageId)); // Save the root pid
	pp += sizeof(PageId);
	memcpy( pp, &m_treeHeight, sizeof(int)); // Save the tree height
	pp += sizeof(int);
	memcpy( pp, &nlKeys, sizeof(int)); // Save number of keys per nonleaf node
	pp += sizeof(int);
	memcpy( pp, &lKeys, sizeof(int)); // Save number of keys per leaf node
	pp += sizeof(int);
	memcpy( pp, &m_nEntries, sizeof(size_t)); // Save number of keys per leaf node
	pp += sizeof(size_t);
	// Finally save our page to disk
	if ( (rc = m_pf.write(0,page)) < 0) return rc;
	return 0; // Success!
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	RC rc; // Temporary storage for return value
	IndexCursor cursor; // Dummy cursor (we don't actually use this)
	PageId pid;

	rc = locate(key,cursor); // Locate the pid of the appropriate leaf node
	
	// Return if we get an error message other than RC_NO_SUCH_RECORD
	// We expect not to find the record since we're inserting it!
	if (rc < 0 && rc != RC_NO_SUCH_RECORD) return rc;

	pid = m_curPid; // Get the leaf node's pid

	m_nEntries++; // Increment the number of entries in our tree

	// NOTE: our index should now have the leaf node we seek after calling
	// locate(). See if we can insert the entry normally into the leaf node
	if (m_ln.getKeyCount() < m_ln.N_KEYS) {
		if ( (rc = m_ln.insert(key,rid)) < 0) return rc; // Insert the new item
		if ( (rc = m_ln.write(pid,m_pf)) < 0) return rc; // Save the leaf node
		return 0;
		}

	// If we can't insert normally, we'll need to do more work.
	BTLeafNode lns; // Sibling leaf node
	BTNonLeafNode nlns; // Sibling nonleaf node
	PageId prevPid,siblingPid; // Pid of sibling and previous sibling
	int prevKey,siblingKey; // Key of sibling and previous sibling

	// Perform a split insertion with the sibling
	if ( (rc = m_ln.insertAndSplit(key,rid,lns,siblingKey)) < 0 ) return rc;

	// Update next pointers for original and sibling nodes
	lns.setNextNodePtr(m_ln.getNextNodePtr());
	m_ln.setNextNodePtr(siblingPid = m_pf.endPid());

	// Save the original into the file
	if ( (rc = m_ln.write(pid,m_pf)) < 0) return rc;
	// Save the sibling into the file and store pid of sibling
	if ( (rc = lns.write(siblingPid,m_pf)) < 0 ) return rc;

	// Recursively try to insert pid into parents
	while (!m_pidStack.empty()) {
		prevPid = siblingPid; // Save the current sibling pid as previous pid
		prevKey = siblingKey; // Save the current sibling key as previous key
		pid = m_pidStack.top(); // Get the top of our pid stack

		// Get the current non-leaf node
		if ( (rc = m_nln.read( pid, m_pf )) < 0) return rc;

		// See if we can insert the entry normally within the nonleaf node
		if (m_nln.getKeyCount() < m_nln.N_KEYS) {
			if ( (rc = m_nln.insert(prevKey,prevPid)) < 0) return rc; // Insert
			if ( (rc = m_nln.write(pid,m_pf)) < 0 ) return rc; // Then save!
			return 0;
			}

		// Otherwise, we have to perform another split
		if ( (rc = m_nln.insertAndSplit(prevKey, prevPid, nlns, siblingKey)) < 0) return rc;

		// Save the original into the file
		if ( (rc = m_nln.write(pid,m_pf)) < 0 ) return rc;
		// Save the sibling into the file and store pid of sibling
		if ( (rc = nlns.write(siblingPid = m_pf.endPid(),m_pf)) < 0 ) return rc;

		m_pidStack.pop(); // Pop the top of the pid stack
		}
	
	cerr << "Splitting root node" << endl;
	// If we get to this point, we need to create a new root node.
	BTNonLeafNode root;
	if ( (rc = root.initializeRoot( pid, siblingKey, siblingPid )) < 0) return rc;
	if ( (rc = root.write( m_rootPid = m_pf.endPid(), m_pf)) < 0 ) return rc;
	m_treeHeight++; // Increment the tree height!
	
	return 0;
}

/*
 * Find the leaf-node index entry whose key value is larger than or 
 * equal to searchKey, and output the location of the entry in IndexCursor.
 * IndexCursor is a "pointer" to a B+tree leaf-node entry consisting of
 * the PageId of the node and the SlotID of the index entry.
 * Note that, for range queries, we need to scan the B+tree leaf nodes.
 * For example, if the query is "key > 1000", we should scan the leaf
 * nodes starting with the key value 1000. For this reason,
 * it is better to return the location of the leaf node entry 
 * for a given searchKey, instead of returning the RecordId
 * associated with the searchKey directly.
 * Once the location of the index entry is identified and returned 
 * from this function, you should call readForward() to retrieve the
 * actual (key, rid) pair from the index.
 * POSTCONDITION: m_pidStack will contain pids traversed by locate()
 * and m_ln will contain the leaf node reached by locate().
 * @param key[IN] the key to find.
 * @param cursor[OUT] the cursor pointing to the first index entry
 *                    with the key value.
 * @return error code. 0 if no error.
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
	 RC rc; // Return value for functions
	 int curHeight = 0; // The current height of the tree (initially 0)
	 // Start at the root
	 cursor.pid = m_rootPid;

	 // Clear out the pid stack for use
	 while (!m_pidStack.empty())
	 	m_pidStack.pop();
	
	 // While we haven't reached the leaf nodes yet
	 while (curHeight != m_treeHeight) {
	 	// Push the current pid onto the stack
		m_pidStack.push(cursor.pid);
		// Read in the non-leaf node corresponding to current pid
	 	if ( (rc = m_nln.read(cursor.pid,m_pf)) < 0) return rc; 
		// Get the child-node pointer to follow
		if ( (rc = m_nln.locateChildPtr(searchKey, cursor.pid)) < 0) return rc; 
 		// Increment the current height
		curHeight++;
	 }

	 // Save the current pid as m_curPid (no need to push onto stack)
	 m_curPid = cursor.pid;

	 // Read in the leaf node corresponding to current pid
	 if ( (rc = m_ln.read(cursor.pid,m_pf)) < 0) return rc; 
	 // Get the eid whose key value >= searchKey
	 if ( (rc = m_ln.locate(searchKey,cursor.eid)) < 0) return rc; 

    return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
	 // If m_ln.getNextNodePtr() < 0 we've reached the end of the tree
	 if (cursor.pid < 0)
		return RC_END_OF_TREE;
	 // Check: do we need to load a different leaf node in?
	 if (cursor.pid != m_curPid) {
	 	 m_ln.read(cursor.pid,m_pf);
		 m_curPid = cursor.pid; // Set the new current pid
		 }
	
	 // Read in the next entry
	 m_ln.readEntry(cursor.eid,key,rid);

	 // Increment the cursor
	 cursor.eid++;
	 if ( cursor.eid == m_ln.getKeyCount() ) {
	 		cursor.pid = m_ln.getNextNodePtr();
			cursor.eid = 0;
	 }
    return 0;
}

void BTreeIndex::print()
{
	IndexCursor cursor; // Cursor for use in traversing tree
	stack<IndexCursor> cursorStack; // Stack to hold previous cursors for recursion
	int curHeight = 0; // Current height of the cursor (initially 0)
	int key;   // Key and pid read in by readEntry()
	PageId pid;
	PageId next = 0; // Next pointer of leaf node; we terminate loop when next == -1 (our null value)
	// Print metadata information
	cerr << m_filename << ": root pid = " << m_rootPid << ", tree height = " 
		<< m_treeHeight << ", # of entries = " << m_nEntries << endl;
	cerr << "Max keys/nonleaf: " << m_nln.N_KEYS << " Max keys/leaf: " << m_ln.N_KEYS << endl;
	// Start at the root pid with eid = -1 (we read the pointer before first key)
	cursor.pid = m_rootPid;
	cursor.eid = -1;
	// Base case: next == -1 and we're at the last leaf node
	while ( next != -1 ) {
		// Base case: curHeight == m_treeHeight and we're at the bottom
		while (curHeight != m_treeHeight) {
			if ( m_nln.read(cursor.pid,m_pf) < 0) return;
			if (cursor.eid == -1) {
				cerr << "\033[22;34mNL: pid = " << cursor.pid << " \033[22;37m";
				m_nln.print();
				pid = m_nln.getFirst();
				cursor.eid++;
				cursorStack.push(cursor);
				cursor.pid = pid;
				cursor.eid = -1;
				curHeight++;
				}
			else if (cursor.eid < m_nln.getKeyCount()) {
				m_nln.readEntry(cursor.eid,key,pid);
				cursor.eid++;
				cursorStack.push(cursor);
				cursor.pid = pid;
				cursor.eid = -1;
				curHeight++;
				}
			else { 
				if (curHeight == 0) {
					cerr << "Fatal error: tried to pop root!" << endl;
					abort();
				}
				cursor = cursorStack.top();
				cursorStack.pop();
				curHeight--;
				}
			}
		cerr << "\033[22;31mL: pid = " << cursor.pid << " \033[22;37m";

		// Read in the leaf node and print it out
		m_ln.read(cursor.pid,m_pf);
		m_ln.print();
		next = m_ln.getNextNodePtr();
		
		// Move back up one level if this isn't the last nod3
		if (next != -1) {
			cursor = cursorStack.top();
			cursorStack.pop();
			curHeight--;
		}
	}

	// Invalidate current pid for readForward's use
	m_curPid = -1; 
}

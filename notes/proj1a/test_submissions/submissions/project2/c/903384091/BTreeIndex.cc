/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
	treeHeight = -1;
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
	RC rc = pf.open(indexname.c_str(), mode);
	
	if (rc == 0)
	{
		if (pf.endPid() == 0)
		{
			bool* buffer = new bool[PAGE_SIZE];
			rc = pf.read(0, buffer);
			if (rc == 0)
			{
				rootPid = btod(32, 40, buffer);
				treeHeight = btod(40, 48, buffer);
				delete buffer;
			}
		}
		else
		{
			bool* buffer = new bool[PAGE_SIZE];
			for (int i = 0; i < PAGE_SIZE; i++)
				buffer[i] = 0;
			rc = pf.write(0, buffer);
			delete buffer;
		}
	}
    return rc;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	bool* buffer = new bool[PAGE_SIZE];
	RC rc = pf.read(0, buffer);
	if (rc == 0)
	{
		dtob(32, 40, rootPid, buffer);
		dtob(40, 48, treeHeight, buffer);
		delete buffer;
		rc = pf.write(0, buffer);
		if (rc == 0)
			rc = pf.close();
	}
    return rc;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	// Initialize root
	if (pf.endPid() == 1)
	{
		BTLeafNode leaf;
		rootPid = 1;
		treeHeight = 1;
		leaf.insert(key, rid);
		leaf.write(1, pf);
	}
	// Root node is a leaf node
	else if (pf.endPid() == 2)
	{
		BTLeafNode leaf;
		leaf.read(rootPid, pf);
		if (leaf.insert(key, rid) != 0)
		{
			// Create new sibling node
			BTLeafNode leaf2;
			int siblingKey = 0;
			leaf.insertAndSplit(key, rid, leaf2, siblingKey);
			leaf.write(2, pf);
			
			// Create new root node
			BTNonLeafNode nonleaf;
			nonleaf.initializeRoot(1, key, 2);
			nonleaf.write(3, pf);
			
			rootPid = 3;
		}
	}
	else
	{		
		// Check if nothing has been traversed
		if (path.empty() && rid.sid != -1)
		{
			path.push(rootPid);
			BTNonLeafNode nonleaf;
			PageId pid;
			nonleaf.read(rootPid, pf);
			nonleaf.locateChildPtr(key, pid);
			path.push(pid);
		}
		
		// Leaf nodes
		if (path.size() == treeHeight && rid.sid != -1)
		{
			BTLeafNode leaf;
			leaf.read(path.top(), pf);
			
			// Attempt to insert the key and rid into the leaf node
			if (leaf.insert(key, rid) != 0)
			{
				// Leaf overflow
				BTLeafNode leafSibling;
				int siblingKey;
				
				leaf.insertAndSplit(key, rid, leafSibling, siblingKey);
				
				// Write the leaf node to the page file
				leaf.write(path.top(), pf);
				leafSibling.write(pf.endPid(), pf);
				path.pop();
				RecordId rid2;
				rid2.pid = pf.endPid()-1;
				rid2.sid = -1;
				
				insert(siblingKey, rid);
			}
		}
		
		// Non-leaf nodes
		else if (path.size() > 1 && rid.sid == -1)
		{
			BTNonLeafNode nonleaf;
			nonleaf.read(path.top(), pf);
			
			// Attempt to insert the sibling key and pid into the parent node
			if (nonleaf.insert(key, rid.pid) != 0)
			{
				// Nonleaf overflow
				// Create a new sibling node
				BTNonLeafNode nonleaf2;
				int midKey = 0;
				nonleaf.insertAndSplit(key, pf.endPid()-1, nonleaf2, midKey);
				nonleaf.write(path.top(), pf);
				nonleaf2.write(pf.endPid(), pf);
				RecordId rid2;
				rid2.pid = pf.endPid()-1;
				rid2.sid = -1;
				path.pop();
				
				// Insert into parent
				insert(midKey, rid);
			}
		}
		
		// Root node
		else if (path.size() == 1 && rid.sid == -1)
		{
			BTNonLeafNode nonleaf;
			nonleaf.read(path.top(), pf);
			
			// Attempt to insert the sibling key and pid into the parent node
			if (nonleaf.insert(key, rid.pid) != 0)
			{
				// Root overflow
				// Create new sibling node
				BTNonLeafNode nonleaf2;
				int midKey = 0;
				nonleaf.insertAndSplit(key, rid.pid, nonleaf2, midKey);
				nonleaf.write(path.top(), pf);
				nonleaf2.write(pf.endPid(), pf);
				
				// Create new root node
				BTNonLeafNode nonleaf;
				nonleaf.initializeRoot(path.top(), midKey, pf.endPid()-1);
				nonleaf.write(pf.endPid(), pf);
				
				rootPid = pf.endPid()-1;
			}
			// Non-leaf overflow
		}
		
		// Find leaf node
		else if (path.size() > 1)
		{
			BTNonLeafNode nonleaf;
			PageId pid;
			nonleaf.read(path.top(), pf);
			nonleaf.locateChildPtr(key, pid);
			path.push(pid);
			insert(key, rid);
		}
	}
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
 * @param key[IN] the key to find.
 * @param cursor[OUT] the cursor pointing to the first index entry
 *                    with the key value.
 * @return error code. 0 if no error.
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
	RC rc = -1;
	if (path.empty())
	{
		path.push(rootPid);
	}
	else if (path.size() == treeHeight)
	{
		BTLeafNode leaf;
		leaf.read(path.top(), pf);
		int eid = 0;
		RC rc = leaf.locate(searchKey, eid);
		if (rc == 0)
		{
			cursor.pid = path.top();
			cursor.eid = eid;
		}
	}
	else
	{
		BTNonLeafNode nonleaf;
		nonleaf.read(path.top(), pf);
		PageId pid = 0;
		nonleaf.locateChildPtr(searchKey, pid);
		path.push(pid);
		locate(searchKey, cursor);
	}
    return rc;
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
	BTLeafNode leaf;
	leaf.read(cursor.pid, pf);
	leaf.readEntry(cursor.eid, key, rid);
	if (cursor.eid == 14)
	{
		cursor.pid = leaf.getNextNodePtr();
	}
	else
		cursor.eid += 1;
	
    return 0;
}

// Convert eights bits into decimal starting at position pos
// and return it
int BTreeIndex::btod(unsigned int pos, unsigned  int endPos, bool* buffer)
{
  unsigned int div = 1;
  for (int i = 0; i < endPos - pos - 1; i++)
    div *= 2;
  int max = pos+endPos;
  int total = 0;
  for (pos; pos < max; pos++)
  {
    total += div * buffer[pos];
	div = div>>1;
  }
  return total;  
}

// Convert decimal to binary and put it in the eight bits
// starting at position pos
void BTreeIndex::dtob(unsigned int pos, unsigned  int endPos, int n, bool* buffer)
{
  unsigned int div = 1;
  for (int i = 0; i < endPos - pos - 1; i++)
    div *= 2;
  for (int i = pos; i < endPos; i++)
  {
    if (n >= div)
	{
	  buffer[i] = 1;
	  n -= div;
	} 
	else
	  buffer[i] = 0;
	  
	div = div>>1;
  }
}

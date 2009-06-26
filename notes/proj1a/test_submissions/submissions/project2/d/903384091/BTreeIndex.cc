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
	
	////printf("Number of pages is %d\n", pf.endPid());
	
	if (rc == 0)
	{
	////printf("rc is 0\n");
		// There exists a page id of 0
		if (pf.endPid() > 0)
		{
			bool* buffer = new bool[PAGE_SIZE];
			rc = pf.read(0, buffer);
			if (rc == 0)
			{
				// Get the root pid and tree height
				rootPid = btod(32, 40, buffer);
				treeHeight = btod(40, 48, buffer);
				////printf("rootPid is %d, treeHeight is %d\n", rootPid, treeHeight);
			}
			delete buffer;
		}
		// Create a new page
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
	//printf("Inserting %d, rid.pid is %d, rid.sid is %d\n", key, rid.pid, rid.sid);
	RC rc = -1;
	// Initialize root
	if (pf.endPid() == 1 && rid.sid != -1)
	{
		BTLeafNode leaf;
		rootPid = 1;
		treeHeight = 1;
		leaf.insert(key, rid);
		leaf.write(1, pf);
	}
	// Root node is a leaf node
	else if (pf.endPid() == 2 && rid.sid != -1)
	{
		//printf("Inserting %d into %d\n", key, rootPid);
		BTLeafNode leaf;
		leaf.read(rootPid, pf);
		rc = leaf.insert(key, rid);
		//leaf.debug();
		////printf("key is %d, rid.pid is %d, rid.sid is %d, rootPid is %d\n", key, rid.pid, rid.sid, rootPid);
		
		if (rc < 0)
		{
			//printf("Splitting first leaf/root node!\n");
			// Create new sibling node
			BTLeafNode leaf2;
			int siblingKey = 0;
			leaf.insertAndSplit(key, rid, leaf2, siblingKey);
			leaf2.write(2, pf);
			leaf.setNextNodePtr(2);
			leaf.write(1, pf);
			
			// Create new root node	
			BTNonLeafNode nonleaf;
			nonleaf.initializeRoot(1, key, 2);
			//nonleaf.debug();
			nonleaf.write(3, pf);
			
			rootPid = 3;
			treeHeight++;
		}
		else
		{
			rc = leaf.write(rootPid, pf);
			while (!path.empty())
				path.pop();
		}
	}
	else
	{		
		//printf("path.size() is %d!\n", path.size());
		// Check if nothing has been traversed
		if (path.empty() && rid.sid != -1)
		{
			////printf("Pushing root node, which is %d!\n", rootPid);
			path.push(rootPid);
			BTNonLeafNode nonleaf;
			PageId pid;
			nonleaf.read(rootPid, pf);
			nonleaf.locateChildPtr(key, pid);
			//printf("Child pointer to follow from root is %d\n", pid);
			path.push(pid);
		}
		
		// Leaf nodes
		if (path.size() == treeHeight && rid.sid != -1)
		{
			//printf("Leaf node: Inserting %d into %d\n", key, path.top());
			BTLeafNode leaf;
			leaf.read(path.top(), pf);
		
			//printf("Number of keys in leaf is %d\n", leaf.getKeyCount());
			// Attempt to insert the key and rid into the leaf node
			if (leaf.insert(key, rid) != 0)
			{
				//printf("Splitting leaf node into %d\n", pf.endPid());
				// Leaf overflow
				BTLeafNode leafSibling;
				int siblingKey;
				
				leaf.insertAndSplit(key, rid, leafSibling, siblingKey);
				leafSibling.setNextNodePtr(leaf.getNextNodePtr());
				leaf.setNextNodePtr(pf.endPid());
				
				// Write the leaf node to the page file
				leaf.write(path.top(), pf);
				leafSibling.write(pf.endPid(), pf);
				path.pop();
				RecordId rid2;
				rid2.pid = pf.endPid()-1;
				rid2.sid = -1;
				
				//printf("siblingKey is %d\n", siblingKey);
				
				insert(siblingKey, rid2);
			}
			else
			{
				rc = leaf.write(path.top(), pf);
				while (!path.empty())
					path.pop();
			}
		}
		
		// Non-leaf nodes
		else if (path.size() > 1 && rid.sid == -1)
		{
			//printf("Nonleaf node: Inserting %d into %d\n", key, path.top());
			BTNonLeafNode nonleaf;
			nonleaf.read(path.top(), pf);
			
			// Attempt to insert the sibling key and pid into the parent node
			if (nonleaf.insert(key, rid.pid) != 0)
			{
				//printf("Splitting nonleaf node\n");
				// Nonleaf overflow
				// Create a new sibling node
				BTNonLeafNode nonleaf2;
				int midKey = 0;
				nonleaf.insertAndSplit(key, pf.endPid()-1, nonleaf2, midKey);
				int tempkey = 0;
				PageId temppid = 0;
				nonleaf.readEntry(nonleaf.getKeyCount()-1, tempkey, temppid);
				nonleaf2.dtob(8, 40, temppid);
				nonleaf.write(path.top(), pf);
				nonleaf2.write(pf.endPid(), pf);
				RecordId rid2;
				rid2.pid = pf.endPid()-1;
				rid2.sid = -1;
				path.pop();
				
				// Insert into parent
				insert(midKey, rid2);
			}
			else
			{
				//nonleaf.write(path.top(), pf);
				while (!path.empty())
					path.pop();
			}
		}
		
		// Root node
		else if (path.size() == 1 && rid.sid == -1)
		{
			//printf("Root node: Inserting %d into %d\n", key, path.top());
			//printf("The pid is %d\n", rid.pid);
			BTNonLeafNode nonleaf;
			nonleaf.read(path.top(), pf);
			
			// Attempt to insert the sibling key and pid into the parent node
			if (nonleaf.insert(key, rid.pid) != 0)
			{
				//printf("Splitting root node\n");
				// Root overflow
				// Create new sibling node
				BTNonLeafNode nonleaf2;
				int midKey = 0;
				nonleaf.insertAndSplit(key, rid.pid, nonleaf2, midKey);
				int tempkey = 0;
				PageId temppid = 0;
				nonleaf.readEntry(nonleaf.getKeyCount()-1, tempkey, temppid);
				nonleaf2.dtob(8, 40, temppid);
				nonleaf.write(path.top(), pf);
				nonleaf2.write(pf.endPid(), pf);
				
				// Create new root node
				BTNonLeafNode nonleaf3;
				nonleaf3.initializeRoot(path.top(), midKey, pf.endPid()-1);
				path.pop();
				nonleaf3.write(pf.endPid(), pf);
				
				rootPid = pf.endPid()-1;
				treeHeight++;
			}
			else
			{
				nonleaf.write(path.top(), pf);
				while (!path.empty())
					path.pop();
			}
			
		}	
		
		// Find leaf node
		else if (path.size() > 1)
		{
			//printf("Searching for leaf node!\n");
			BTNonLeafNode nonleaf;
			PageId pid;
			//if (path.size() < 200)
				////printf("treeHeight is %d, path.size() is %d, and path.top() is %d", treeHeight, path.size(), path.top());
			nonleaf.read(path.top(), pf);
			//nonleaf.debug();
			nonleaf.locateChildPtr(key, pid);
			////printf("Child Ptr is %d\n", pid);
			path.push(pid);
			insert(key, rid);
		}
	}
    return rc;
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
	
	//if (!path.empty())
		//printf("%d, treeHeight is %d\n", path.top(), treeHeight);
	if (path.empty())
	{
		////printf("rootPid is %d\n", rootPid);
		path.push(rootPid);
		rc = locate(searchKey, cursor);
	}
	else if (path.size() == treeHeight)
	{
		BTLeafNode leaf;
		leaf.read(path.top(), pf);
		int eid = 0;
		rc = leaf.locate(searchKey, eid);
		if (rc == 0)
		{
			int key = 0;
			RecordId rid;
			leaf.readEntry(eid, key, rid);
			//printf("eid is %d, key is %d\n", eid, key);
			cursor.pid = path.top();
			cursor.eid = eid;
		}
		while (!path.empty())
			path.pop();
	}
	else
	{
		BTNonLeafNode nonleaf;
		PageId pid = 0;
		if (rc = nonleaf.read(path.top(), pf) == 0)
		{
			//nonleaf.debug();
			if (rc = nonleaf.locateChildPtr(searchKey, pid) == 0)
			{
				////printf("pid returned is %d\n", pid);
				path.push(pid);
				rc = locate(searchKey, cursor);
			}
		}
		
		while (!path.empty())
			path.pop();
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
	//printf("cursor.pid is %d\n", cursor.pid);
	leaf.read(cursor.pid, pf);
	leaf.readEntry(cursor.eid, key, rid);
	//printf("readForward: key is %d, rid.pid is %d, rid.sid is %d, cursor.eid is %d, keycount is %d\n", key, rid.pid, rid.sid, cursor.eid, leaf.getKeyCount());
	//leaf.debug();
	if (cursor.eid >= leaf.getKeyCount()-1)
	{
		////printf("Going to next leaf!\n");
		cursor.pid = leaf.getNextNodePtr();
		//printf("cursor.pid is %d!\n", cursor.pid);
		if (cursor.pid < 0)
			return cursor.pid;
		cursor.eid = 0;
	}
	else
	{
	
		//printf("cursor.pid is %d!\n", cursor.pid);
		cursor.eid += 1;
		}
	
    return 0;
}

// Convert eight bits into decimal starting at position pos
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

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
		treeHeight = 0;		
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
	//Open the pagefile...if error, return error code
	int rc = pf.open(indexname,mode);
	if (rc != 0)	return rc;

	//Check if the pagefile is empty
	//And if it is, make it non empty
	if (pf.endPid() == 0)	{
		int* buffer = new int[2];
		buffer[0] = rootPid;
		buffer[1] = treeHeight;
		pf.write(0,buffer);
	}

	return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	//Make sure some vital data is saved
	int* buffer = new int[2];
	buffer[0] = rootPid;
	buffer[1] = treeHeight;
	pf.write(0,buffer);

	pf.close();
	
	return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	//If this is the first key of the index, create a leaf object
	if (treeHeight == 0)	{
		rootPid = 1;
		treeHeight = 1;
		BTLeafNode node;
		node.insert(key,rid);
		node.write(rootPid,pf);
		cerr << "Creating the Tree with Key:" << key << " at pid=" << rootPid << endl;
		return 0;
	}
	//Otherwise call recursive function insert_helper
	else	{
		int r_pid, r_key;
		int rc = insert_helper(key, rid, 0, rootPid, r_pid, r_key);
		//If new root is needed
		if (rc == 1)	{
			BTNonLeafNode newroot;
			newroot.initializeRoot(rootPid,r_key,r_pid);
			newroot.write(pf.endPid(),pf);
			//Update values
			cerr << "New Root with key=" << r_key << " at pid=" << pf.endPid() - 1 << endl;
			cerr << "	initialized with pid1=" << rootPid << " and pid2=" << r_pid << endl;
			rootPid = pf.endPid() - 1;
			treeHeight++;
			return 0;
		}

		return rc;
	}
}

//Recursive helper function for insert
RC BTreeIndex::insert_helper(int key, const RecordId& rid, int height, int pid, int& r_pid, int& r_key)
{
	int rc;	//return code

	//If at leaf level
	if (height == treeHeight - 1)	{
		BTLeafNode leaf;
		rc = leaf.read(pid,pf);	if (rc != 0)	return rc;
		//If the leaf is full
		if (leaf.getKeyCount() == MAX_NODE_SIZE)	{
			//Set up a new leaf node
			BTLeafNode sibling;
			int siblingkey;
			//Insert and split
			rc = leaf.insertAndSplit(key,rid,sibling,siblingkey);	if (rc != 0)	return rc;
			//Set this node to point to the sibling
			rc = leaf.setNextNodePtr(pf.endPid());	if (rc != 0)	return rc;
			//Write the sibling and leaf to memory	
			rc = sibling.write(pf.endPid(),pf);	if (rc != 0)	return rc;
			rc = leaf.write(pid,pf);	if (rc != 0)	return rc;
			//Return the siblingkey and the pid of the new leaf node
			r_pid = pf.endPid() - 1;
			r_key = siblingkey;
			cerr << "Leaf Insertion and Split with key=" << key << " at pid=" << pid << endl;
			cerr << "	w/ siblingkey=" << r_key << " and newpid=" << r_pid << endl;
			return 1;
		}
		//If the leaf is not full
		else	{
			//Insert and write
			rc = leaf.insert(key,rid);	if (rc != 0)	return rc;
			rc = leaf.write(pid,pf);	if (rc != 0)	return rc;
			cerr << "Leaf Insertion with key=" << key << " at pid=" << pid << ". Size now " << leaf.getKeyCount() << endl;
			return 0;
		}
	}
	//Otherwise
	else	{
		BTNonLeafNode nonleaf;
		rc = nonleaf.read(pid,pf);	if (rc != 0)	return rc;
		//Find the next node to explore
		int pid_below;
		rc = nonleaf.locateChildPtr(key,pid_below);	if (rc != 0)	return rc;
		//Recursively call insert_helper
		int pid_r;
		int key_r;
		rc = insert_helper(key,rid,height+1,pid_below,pid_r,key_r);
		//If there is no need for insertion to this node
		if (rc == 0)
			return 0;
		//If there is insertion to this node
		else if (rc == 1)	{
			//If we need to split a node
			if (nonleaf.getKeyCount() == MAX_NODE_SIZE)	{
				//Set up a new sibling
				BTNonLeafNode sibling;
				int midKey;				
				rc = nonleaf.insertAndSplit(key_r,pid_r,sibling,midKey);
				if (rc != 0)	return rc;
				//Write
				rc = sibling.write(pf.endPid(),pf);	if (rc != 0)	return rc;
				rc = nonleaf.write(pid,pf);	if (rc != 0)	return rc;
				//Set return values (the pid of the new node and the midkey)
				r_pid = pf.endPid() - 1;
				r_key = midKey;
				cerr << " Nonleaf Insertion and Split with key=" << key_r << " with pid=" << pid << endl;
				cerr << "		w/ midkey=" << midKey << " and newpid=" << r_pid << endl;
				return 1;
			}
			//Otherwise simply insert
			else	{
				rc = nonleaf.insert(key_r,pid_r);	if (rc != 0)	return rc;
				rc = nonleaf.write(pid,pf);	if (rc != 0)	return rc;
				cerr << "	Nonleaf Insertion with key=" << key_r << " at pid=" << pid << ". Size now " << nonleaf.getKeyCount() <<endl;
				return 0;
			}

		}
	}
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
	int pid = rootPid;
	int rc;	//return value
	//Traversing most of the tree
	for (int i = 0; i < treeHeight-1; ++i)	{
		BTNonLeafNode nonleaf;
		rc = nonleaf.read(pid,pf);	if (rc != 0)	return rc;
		rc = nonleaf.locateChildPtr(searchKey,pid);	if (rc != 0)	return rc;
	}
	//At the leaf level
	BTLeafNode leaf;
	cursor.pid = pid;
	rc = leaf.read(pid,pf);	if (rc != 0)	return rc;
	rc = leaf.locate(searchKey, cursor.eid);	if (rc != 0)	return rc;
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
	//Copy node cursor pointing to to leaf
	BTLeafNode leaf;
	int rc;
	rc = leaf.read(cursor.pid,pf);	if (rc != 0)	return rc;
	//Read entry to key and rid
	rc = leaf.readEntry(cursor.eid, key, rid);	return rc;
	//If next entry exceeds node, jump to next node
	if (++cursor.eid == leaf.getKeyCount())	{
		cursor.pid = leaf.getNextNodePtr();
		cursor.eid = 0;
	}
	return 0;
}

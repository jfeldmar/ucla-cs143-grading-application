/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include <iostream>
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = 0;
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
	if (mode == 'w')
	{
		pf.open(indexname,mode);
		treeHeight = 0;
		rootPid = -1;

		char info[1024];
		InfoEntry *ent = (InfoEntry*)info;
		ent->height = treeHeight;
		ent->root = rootPid;
		pf.write((PageId)0,info);
	}

	else
	{
		if (pf.open(indexname,mode) != 0)
			return -1;

		char info[1024];
		pf.read((PageId)0,info);
		InfoEntry *ent = (InfoEntry*)info;
		treeHeight = ent->height;
		rootPid = ent->root;
	}
    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
		char info[1024];
		InfoEntry *ent = (InfoEntry*)info;
		ent->height = treeHeight;
		ent->root = rootPid;
		pf.write((PageId)0,info);
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
	if (treeHeight == 0)
	{
		BTLeafNode *btln = new BTLeafNode();
		btln->data[1023] = pf.endPid();
		btln->data[1022] = 0;
		btln->data[1021] = pf.endPid();

		btln->insert(key,rid);
		btln->write((PageId)btln->data[1023],pf);
		
		treeHeight++;
		rootPid = (PageId)btln->data[1023];

		char info[1024];
		InfoEntry *ent = (InfoEntry*)info;
		ent->height = treeHeight;
		ent->root = rootPid;
		pf.write((PageId)0,info);

		return 0;
	}
    
	if (treeHeight == 1)
	{
		BTLeafNode *btln = new BTLeafNode();
		btln->read(rootPid,pf);

		if (btln->getKeyCount() >= 50)
		{
			BTLeafNode *sib = new BTLeafNode();
			sib->data[1023] = pf.endPid();
			sib->data[1022] = 0;
			sib->data[1021] = pf.endPid();

			int sibkey;

			btln->insertAndSplit(key,rid,*sib,sibkey);

			btln->write((PageId)btln->data[1023],pf);
			sib->write((PageId)sib->data[1023],pf);

			BTNonLeafNode *parent = new BTNonLeafNode();
			parent->data[1023] = pf.endPid();

			parent->initializeRoot((PageId)btln->data[1023],sibkey,(PageId)sib->data[1023]);

			parent->write((PageId)parent->data[1023],pf);
			
			treeHeight++;
			rootPid = (PageId)parent->data[1023];

			char info[1024];
			InfoEntry *ent = (InfoEntry*)info;
			ent->height = treeHeight;
			ent->root = rootPid;
			pf.write((PageId)0,info);

			return 0;
		}

		else
		{
			btln->insert(key,rid);
			btln->write((PageId)btln->data[1023],pf);
			
			return 0;
		}	
	}

	int i = 0;
	PageId tracker[100];
	BTNonLeafNode *current = new BTNonLeafNode();
	
	current->read(rootPid,pf);
	tracker[i] = rootPid;
	i++;
	
	PageId pid;

	for(int j = treeHeight - 1; j > 0; j--)
	{
		current->locateChildPtr(key, pid);
		tracker[i] = pid;
		//i++;

		if (j > 1)
		{
			i++;
			current->read(pid,pf);
		}
	}

	//i--;
	BTLeafNode *btln = new BTLeafNode();
	btln->read(pid,pf);

	if (btln->getKeyCount() >= 50)
	{
		BTLeafNode *sib = new BTLeafNode();
		sib->data[1023] = pf.endPid();
		sib->data[1022] = 0;
		sib->data[1021] = pf.endPid();

		int sibkey;

		btln->insertAndSplit(key,rid,*sib,sibkey);
		btln->write((PageId)btln->data[1023],pf);
		sib->write((PageId)sib->data[1023],pf);

		PageId overflow = (PageId)sib->data[1023];

		BTNonLeafNode *parent = new BTNonLeafNode();

		for(int j = i - 1; j >= 0; j--)
		{
			parent->read(tracker[j],pf);

			if (parent->getKeyCount() < 50)
			{
				parent->insert(sibkey,overflow);
				parent->write((PageId)parent->data[1023],pf);
			
				return 0;
			}
			
			BTNonLeafNode *parentSib = new BTNonLeafNode();
			parentSib->data[1023] = pf.endPid();
			parentSib->data[1022] = 0;
			parentSib->data[1021] = overflow;
	
			int midKey;

			parent->insertAndSplit(sibkey,overflow,*parentSib,midKey);

			sibkey = midKey;
			overflow = (PageId)parentSib->data[1023];
			parent->write((PageId)parent->data[1023],pf);
			parentSib->write((PageId)parentSib->data[1023],pf);

			if (j == 0)		// Root overflow has occurred.
			{
				BTNonLeafNode *newRoot = new BTNonLeafNode();
				newRoot->data[1023] = pf.endPid();
				newRoot->initializeRoot(rootPid,sibkey,overflow);

				newRoot->write((PageId)newRoot->data[1023],pf);				

				rootPid = (PageId)newRoot->data[1023];
				treeHeight++;

				char info[1024];
				InfoEntry *ent = (InfoEntry*)info;
				ent->height = treeHeight;
				ent->root = rootPid;
				pf.write((PageId)0,info);

				return 0;
			}
		}
	}

	else
	{
		btln->insert(key,rid);
		btln->write((PageId)btln->data[1023],pf);
		
		return 0;
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
	if (treeHeight <= 0)
		return 0;

	if (treeHeight == 1)
	{
		BTLeafNode *current = new BTLeafNode();
		current->read(rootPid,pf);

		cursor.pid = rootPid;
		current->locate(searchKey,cursor.eid);

		return 0;
	}

	BTNonLeafNode *current = new BTNonLeafNode();
	current->read(rootPid,pf);

	PageId pid;

	for(int j = treeHeight - 1; j > 0; j--)
	{	
		current->locateChildPtr(searchKey, pid);

		if (j > 1)
			current->read(pid,pf);
	}

	BTLeafNode *btln = new BTLeafNode();
	btln->read(pid,pf);
	cursor.pid = pid;
	btln->locate(searchKey,cursor.eid);
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
	BTLeafNode *btln = new BTLeafNode();
	btln->read(cursor.pid,pf);

	btln->readEntry(cursor.eid,key,rid);
	
	if (cursor.eid < btln->getKeyCount())
	{
		cursor.eid++;
		return 0;
	}
	
	if(btln->getNextNodePtr() < 0)
		return -1;

	cursor.pid = btln->getNextNodePtr();
	cursor.eid = 1;

	return 0;
}

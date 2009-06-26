/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include "BTreeIndex.h"

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
	int open = pf.open("couch1", 'w');
    rootPid = -1;
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
	int opencheck = pf.open(indexname, mode);
    	return(opencheck);
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	int closecheck = pf.close();
    return(closecheck);
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	ntree.read(0, pf);
	rootPid = ntree.buffer[0];
	treeHeight = ntree.buffer[1];

	//printf("ROOOT: %i", rootPid);

	// FIRST INSERTION INTO THE TREE(CREATE ROOT AND 2 LEAF NODES)
	if(rootPid == 0)
	{
		ntree.initializeRoot(2, key, 3);
		int check = ntree.write(1, pf);
		ltree.buffer[0] = 0;
		ltree.buffer[4] = 3;
		ltree.write(2, pf);
		ltree.buffer[0] = 1;
		ltree.buffer[1] = rid.pid;
		ltree.buffer[2] = rid.sid;
		ltree.buffer[3] = key;
		ltree.buffer[4] = 20;
		ltree.write(3, pf);
		rootPid = 1;
		treeHeight = 2;
	}	
	else
	{
		BTLeafNode newleaf;
		IndexCursor ind;
		locate(key, ind);
		printf("IND: %i", ind.pid);

		int end = 0;
		int more = 0;
		int track = 1;
		int curpid = ind.pid;
		int sibkey = 0;
		int sibpid = 0;
		ltree.read(curpid, pf);
		int valid = ltree.insert(key, rid);
		if(valid == -1010)
		{
			ltree.insertAndSplit(key, rid, newleaf, sibkey);
			end = pf.endPid();
			ltree.setNextNodePtr(end);
			ltree.write(curpid, pf);
			newleaf.write(end, pf);	
		}
		else
		{
			//printf("CURPID: %i", curpid);
			ltree.write(curpid, pf);
			more = 1;
		}
		curpid = tracer[treeHeight-track-1];
		sibpid = end;
		track++;
		int newkey;
		BTNonLeafNode newnonleaf;
		while(more == 0)
		{
			ltree.read(curpid, pf);
			int valid = ntree.insert(sibkey, sibpid);
			if(valid == -1010)
			{
				printf("FUCKKK!!");
				ntree.insertAndSplit(sibkey, sibpid, newnonleaf, newkey);
				int end = pf.endPid();
				int setend = ntree.buffer[0];
				setend = (setend*2)+1;
				ntree.buffer[setend] = end;
				ntree.write(curpid, pf);
				newnonleaf.write(end, pf);	
			}
			else
			{
				ntree.write(curpid, pf);
				more = 1;
			}
			if(track > treeHeight)
			{
				treeHeight++;
				//rootPid = ;
			}
			curpid = tracer[treeHeight-track-1];
			sibpid = end ;	
			sibkey = newkey;
			track++;
		}

	}

	ntree.read(0, pf);
	ntree.buffer[0] = rootPid;
	ntree.buffer[1] = treeHeight;
	ntree.write(0, pf);

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
	ntree.read(0, pf);
	rootPid = ntree.buffer[0];
	treeHeight = ntree.buffer[1];
	int nextpid = rootPid;
	tracer[0] = rootPid;
	int x = 1;
	while(x < treeHeight)
	{
		ntree.read(nextpid, pf);
		ntree.locateChildPtr(searchKey, nextpid);
		tracer[x] = nextpid;
		x++;
	}

	int z = 3;
	int entryid = 0;
	ltree.read(nextpid, pf);
	ltree.locate(searchKey, entryid);		
	cursor.pid = nextpid;
	cursor.eid = entryid;
	
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
    return 0;
}

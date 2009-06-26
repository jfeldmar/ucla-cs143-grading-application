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


PageFile pf;
RC rc;
BTNonLeafNode& nonNode(nonNode);
BTLeafNode& leafNode(leafNode);
PageId pid = 0;
string& newIndex(newIndex);


/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
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
	newIndex=indexname;
	
	if(mode = 'w')
	{
		rc = pf.open(indexname, 'w');
		if(rc < 0)
			printf("ERROR, Cant open the file");
		else{
			//while(pid <= (pf.endPid()-1))
			//{
				leafNode.read(pid,pf);
				//pid++;
			//}
			return 0;
		}
	}
	else if(mode = 'r')
	{
		rc = pf.open(indexname, 'r');
		if(rc < 0)
			printf("ERROR, Cant open the file");
		else{
			//while(pid <= (pf.endPid()-1))
			//{
				leafNode.read(pid,pf);
				//pid++;
			//}
			return 0;
		}
	}
	else
		printf("Error, please use a write or read mode");
	
	return -1;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    if(pf.close()==0)
		return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, RecordId& rid)
{
    int &test = key;
	int& sibling(sibling);
	BTLeafNode& newLeafNode(newLeafNode);

	if(leafNode.getKeyCount() == 99)
	{
		if(leafNode.insertAndSplit(test,rid,newLeafNode,sibling) == 0)
			return 0;
		else
			return -1;
	}
	else if(leafNode.getKeyCount() < 99){
		if(leafNode.insert(test,rid)==0)
			return 0;
	}
	else
		printf("Error inserting data");
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
    int& eid(eid);
	int test = 0;
	char mode = 'r';
	
	while(test == 0)
	{
		if(leafNode.locate(searchKey,eid)==0)
		{
			cursor.eid = eid;
			cursor.pid = pid;
			test = -1;
			return 0;
		}
		else
		{
			pid++;
			open(newIndex,mode);
		}
	}

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
	if(leafNode.readEntry(cursor.eid,key,rid)==0)
	{
		if(cursor.pid == rid.pid)
		{
			printf("Correct match");
			return 0;
		}
		else
			printf("NO MATCH");
	}
	else
		return -1;

}

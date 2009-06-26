/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"


int BTreeIndex::nSize = 80;

/*
 * BTreeIndex constructor
 */

void BTreeIndex::printTree()
{
	int i = 0;
	int jo=0;
	int ace;
	do
	{
	char buffer[PageFile::PAGE_SIZE];
	pf.read(jo,buffer);
	ace = jo;
	int* temp = (int*) buffer;
	fprintf(stdout,"RootPid = %d\n",temp[0]);
	fflush(stdout);
	fprintf(stdout,"TreeHeight = %d\n",temp[1]);
	fflush(stdout);
	pf.read(1,buffer);
	SubNodeLeaf* leaf;
	int i=8;
	leaf = (SubNodeLeaf*)(buffer+i);
	i=0;
	int count = *(int*)(buffer);
	fprintf(stdout, "count: %d\n", count);
	fflush(stdout);
	while(i<count){
		fprintf(stdout, "key: %d\n",leaf[i].key);
		fflush(stdout);
		i++;
	}
	int* arr = (int*) buffer;
	jo = arr[1];
	fprintf(stdout,"J = %d\n",jo);
	}while(jo>0 && jo!=ace);

}

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
RC BTreeIndex::open(const std::string& indexname, char mode)
{
	pf.open(indexname, mode);
	/*
	const char *fileName;
	fileName=indexname.c_str();
	fstream infile;
	infile.open(fileName, ios::in);
	if(!infile.is_open())
    {
      if( mode == 'r')
		  return 0;
    }
	fstream outfile;
	infile.open(fileName, ios::out);
	inIndex=infile;
	outIndex=outfile;
	*/
	rootPid = 1;
	treeHeight = 1;
	if(pf.endPid() > 1){
		char buffer[PageFile::PAGE_SIZE];
		pf.read(0,buffer);
		PageId* temp = (PageId*)buffer;
		rootPid = temp[0];
		int* temp2 = (int*)buffer;
		treeHeight = temp2[1];
	}
	else{
			BTLeafNode leaf;
			leaf.write(rootPid,pf);
	}
		return 0;
}


/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	int buffer[PageFile::PAGE_SIZE/sizeof(int)];
	buffer[0] = rootPid;
	buffer[1] = treeHeight;
	pf.write(0,buffer);
	//printTree();
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
	if(treeHeight>1)
		return 0;
	else
	{
		BTLeafNode leaf;
		leaf.read(rootPid,pf);
		if(leaf.getKeyCount()<=nSize)	
		{
			fprintf(stdout,"Inserting:  %d\n",key);
			leaf.insert(key,rid);
			leaf.write(rootPid,pf);
		}
		else{
			BTLeafNode sibling;
			BTNonLeafNode parent;
			int sibKey;
			leaf.insertAndSplit(key,rid,sibling,sibKey);
			parent.initializeRoot(rootPid,sibKey,pf.endPid()-1);
			rootPid = pf.endPid()-1;
			treeHeight=2;

		}
	}
	printTree();
	/*
	index.eid=rid.sid;
	index.pid=rid.pid;
	outIndex<<index.pid<<" "<<index.eid<<" "<<key<<" ";
    */
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
	/*
	int key;
	while(searchKey<key){
		inIndex>>key;
		inIndex>>key;
		inOndex>>key;
		cursor.eid++;
		if(eid>=RecordFile::RECORDS_PER_PAGE){
			cursor.eid=0;
			cursor.pid++;
		
		}
	}
		*/
	BTNonLeafNode root;
	root.read(rootPid,pf);
	int i=0;
	SubNodeNonLeaf* finder = (SubNodeNonLeaf*) (root.buffer+8);
	int levelcount = treeHeight;
	while(levelcount>1){
		while(finder[i].key > searchKey)
			i++;
		root.read(finder[i-1].value2,pf);
		i=0;
		finder = (SubNodeNonLeaf*) (root.buffer+8);
	}
	SubNodeLeaf* finds = (SubNodeLeaf*) (root.buffer+8);
	while(finds[i].key > searchKey)
		i++;
	cursor.eid=i;
	cursor.pid=finds[i].value.pid;
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
	/*IndexCursor myCursor;
	*inIndex>>myCursor.pid;
	do{
		rid.pid=cursor.pid;
		rid.sid=cursor.eid;
		PageId tempID;
		*inIndex>>myCursor.eid;
		*inIndex>>key;
	}while(*inIndex>>myCursor.pid||(myCursor.pid!=cursor.pid&&myCursor.eid!=cursor.eid));
    */

	return 0;
}

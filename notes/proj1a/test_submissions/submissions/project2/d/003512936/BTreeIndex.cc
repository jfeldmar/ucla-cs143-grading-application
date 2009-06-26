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
#include <iostream>

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
	char temp[PageFile::PAGE_SIZE];
	if (!pf.open(indexname.c_str(), mode))
	{
		// If there is something written to the file then index was created earlier
		// extract rootPid and treeHeight info
		if (pf.endPid() != 0/*mode == 'r'*/)
		{
			pf.read(0, temp);
			memcpy(&rootPid, temp, 4);
			memcpy(&treeHeight, temp+4, 4);
			return 0;
		}
		else 
		{
			memcpy(temp, &rootPid, 4);
			memcpy(temp+4, &treeHeight, 4);
			pf.write(0, temp);
			return 0;
		}
		
	}
	else
		return RC_FILE_OPEN_FAILED;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	char temp[PageFile::PAGE_SIZE];
	memcpy(temp, &rootPid, 4);
	memcpy(temp+4, &treeHeight, 4);
	pf.write(0, temp);

	if(!pf.close())
	{
		return 0;
	}
	else
		return RC_FILE_CLOSE_FAILED;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	IndexCursor cursor; //Where new pair should go
	// Index file is empty case, create leaf root.
	if(rootPid == -1)
	{
		//cout<<"Index is empty\n";
		char tempRoot[PageFile::PAGE_SIZE];
		BTLeafNode root;
		root.insert(key, rid);
		memcpy(tempRoot, root.leafArray, sizeof(root.leafArray));
		pf.write(1, tempRoot);
		rootPid = 1;
		treeHeight = 1;

		// write changes to file
		char temp[PageFile::PAGE_SIZE];
		memcpy(temp, &rootPid, 4);
		memcpy(temp+4, &treeHeight, 4);
		pf.write(0, temp);
		return 0;
	}
	
	char tempBuffer[PageFile::PAGE_SIZE]; //to hold leaf L info
	BTLeafNode L;
	// Find position where new key should be inserted and populate parents stack
	locate(key, cursor);  
	int currentParent = parents.top();
	//parents.pop();	// pop the parent
	//cout<<"CURSOR PID: "<<cursor.pid<<endl;
	pf.read(cursor.pid, L.leafArray);
	//cout <<"NUMKEYS: "<<numKeys<<endl;
	
	if(!L.insert(key, rid)) //Leaf has enough room, insert in leaf
	{
		/* //DELETE ME
		if(key == 3754)
		{
			cout<<"Inserted Simpatico inside Leaf has enough room\n";
			cout<<"Rid.pid is: "<<rid.pid<<" and rid.sid is: "<<rid.sid<<endl;
		} */
		pf.write(cursor.pid, L.leafArray);
		return 0;
	}
	else //Leaf is full, gotta split it.
	{
		// Root case
		if(treeHeight == 1)
		{
			BTNonLeafNode newRoot;
			BTLeafNode rootSibling;
			int siblingKey;
			L.insertAndSplit(key, rid, rootSibling, siblingKey);
			newRoot.insert(siblingKey, currentParent);
			newRoot.setPid(1, pf.endPid());

			// commit writes
			int pidToInsert = pf.endPid();
			L.setNextNodePtr(pidToInsert);
			rootSibling.setNextNodePtr(-1);

			pf.write(currentParent, L.leafArray);
			pf.write(pidToInsert, rootSibling.leafArray);
			pf.write(pidToInsert + 1, newRoot.nonLeafArray);

			// set new rootPid and treeHeight
			rootPid = pidToInsert + 1;
			treeHeight = 2;
			return 0;
		} // end root case

		//Non-root leaf is full case
 		BTLeafNode sibling;	// new sibling node
		int siblingKey;		// first key of sibling
		PageId siblingPid = pf.endPid();  // Sibling's PID [endPid]
		//cout <<"SIBLING PID: "<<siblingPid<<endl;  // REMOVE ME!
		PageId leafPid = parents.top();  // leafPid is the top stack's pid
		//cout<<"leafPid = "<<leafPid<<endl;
		
		L.insertAndSplit(key, rid, sibling, siblingKey);
		//DELETE ME
		//if(key == 3754) cout<<"Inserted Simpatico after insertAndSplit\n";

		//cout <<"SIBLING'S first key is: "<<siblingKey<<endl;
		
		sibling.setNextNodePtr(L.getNextNodePtr());
		L.setNextNodePtr(siblingPid);
		pf.write(siblingPid,sibling.leafArray);
		pf.write(leafPid, L.leafArray);
		parents.pop(); //Poping the leafnode
		
		bool overflow = true;
		do 
		{
		//Now insert siblingKey in parent
			PageId parentPid = parents.top();
			parents.pop();
			BTNonLeafNode parentNode;
			pf.read(parentPid, parentNode.nonLeafArray);
			if (!parentNode.insert(siblingKey, siblingPid))
			{
				//cout<<"Nonleaf had room and inserted"<<endl;
				pf.write(parentPid, parentNode.nonLeafArray);
				overflow = false;
			}
			else
			{
				BTNonLeafNode parentSibling;				
				parentNode.insertAndSplit(siblingKey, siblingPid, parentSibling, siblingKey);
				pf.write(siblingPid, parentSibling.nonLeafArray);
				// Root Overflow case
				if(parentPid == rootPid)
				{	
					//cout<<"Root overflow"<<endl;
					treeHeight++;
					BTNonLeafNode newRoot;
					PageId newRootPid = pf.endPid();
					//cout<<"b4 newRoot.insert"<<endl;
					newRoot.insert(siblingKey, parentPid);
					//cout<<"after newRoot.insert"<<endl;
					newRoot.setPid(1,siblingPid);
					rootPid = newRootPid;
					//cout<<"Before the writes"<<endl;
					pf.write(parentPid, parentNode.nonLeafArray);
					pf.write(siblingPid, parentSibling.nonLeafArray);
					pf.write(rootPid, newRoot.nonLeafArray);
					//cout<<"After writes"<<endl;
					overflow = false;
					//cout<<"End of Root overflow"<<endl;
				}
			}
			//pf.write(parentPid, parentNode.nonLeafArray);		
			//cout<<"In loop"<<endl;
		}while (overflow);
		
		char temp[PageFile::PAGE_SIZE];
		memcpy(temp, &rootPid, 4);
		memcpy(temp+4, &treeHeight, 4);
		pf.write(0, temp);
		//cout<<"Im out"<<endl;
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
	char tempBuffer[PageFile::PAGE_SIZE];
	PageId C = this->rootPid;
	
	//Empty the stack
	while (!parents.empty())
	{
		parents.pop();
	}
	
	parents.push(C);
	int i = 1;
	while(i <= treeHeight) // While C is not a leaf node
	{	
		// NonLeafNodes
		if(i != treeHeight)
		{
			BTNonLeafNode temp;
			pf.read(C, temp.nonLeafArray);
			temp.locateChildPtr(searchKey, C);
			parents.push(C);
		}
		// LeafNode reached
		if(i == treeHeight)
		{
			BTLeafNode temp;
			int eid;
			pf.read(C, temp.leafArray);

			if(!temp.locate(searchKey, eid))
			{
				if(eid >= temp.getKeyCount()) 
				{
					cursor.pid = C;		//This is the IndexExtry where the record would go
					cursor.eid = eid;
					return RC_NO_SUCH_RECORD;
				}
				else
				{				
					cursor.pid = C;
					cursor.eid = eid;
					return 0;
				}
			}
		}// end if i == treeHeight
		i++;  // increment i
	}// end while
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

    if (cursor.eid > N_LEAF || cursor.eid < 0)
        return RC_INVALID_CURSOR;
    
    BTLeafNode temp;
    pf.read(cursor.pid, temp.leafArray);
    
    rid.pid = temp.getRecordPid(cursor.eid);
    rid.sid = temp.getRecordSid(cursor.eid);
	//DELETE ME
	//cout<<"inside READFORWARD: rid.pid: "<<rid.pid<<" rid.sid: "<<rid.sid<<endl;
    key = temp.getKey(cursor.eid);
    
    if (cursor.eid == temp.getKeyCount() - 1)
    {
        cursor.eid = N_LEAF-1;
    }
    
    if (cursor.eid == N_LEAF-1)
    {
        //Set cursor to sibling
        //memcpy(&cursor.pid, temp.leafArray + 1020, 4);
        cursor.pid = temp.getNextNodePtr();
        if (cursor.pid == -1)
            return RC_INVALID_CURSOR;
        cursor.eid = 0;
    }
    else
        cursor.eid++;
            
    return 0;
}

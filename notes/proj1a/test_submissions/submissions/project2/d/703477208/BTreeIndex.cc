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
	char root_buffer[PageFile::PAGE_SIZE];
	PageId epid;
	RC rc;
	rc = pf.open(indexname + ".idx", mode);
	if(rc < 0)
		return rc;
	else
	{
		epid = pf.endPid(); //check the first pagefile
		if(epid == 0) //its a new file! set the stuff to -1!
		{
			treeHeight = -1;
			memcpy(root_buffer, &rootPid,  sizeof(rootPid));
			memcpy(root_buffer + sizeof(rootPid), &treeHeight, sizeof(treeHeight));
			pf.write(0,root_buffer);
			
		}
		else
		{
			pf.read(0, root_buffer);
			memcpy(&rootPid, root_buffer, sizeof(rootPid));
			memcpy(&treeHeight, root_buffer + sizeof(rootPid), sizeof(treeHeight));
		}
	}
	return rc;
    //return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	RC rc;
	char root_buffer[PageFile::PAGE_SIZE];
	memcpy(root_buffer, &rootPid,  sizeof(rootPid));
	memcpy(root_buffer + sizeof(rootPid), &treeHeight, sizeof(treeHeight));
	pf.write(0,root_buffer);
	rc = pf.close();
    return rc;
}
//make the root!

PageId BTreeIndex::parent(const PageId& pid)
{
	PageId parent_pid;
	char buffer2[PageFile::PAGE_SIZE];
	pf.read(pid, buffer2);
	memcpy(&parent_pid, buffer2 + 4 + 4, sizeof(int));
	return parent_pid;
}

////////////
RC BTreeIndex::insert_in_parent(PageId N, int key, PageId Nchild)
{
	if(N == rootPid)
	{	
		RC rc;
		char Nbuffer[PageFile::PAGE_SIZE];
		char Nchildbuffer[PageFile::PAGE_SIZE];
		BTNonLeafNode nonleaf_node;
		int endPid;
		
		endPid = pf.endPid();
		if((rc = nonleaf_node.initializeRoot(N, key, Nchild)) < 0)
			{return rc;}
		nonleaf_node.setparentPtr(-1);
		nonleaf_node.setownPtr(endPid); 	//set its own PageId. Set ParentPid is -1 as root.
				
		pf.read(N, Nbuffer);				//read the buffer of the child and sibbling, and set their parent pointer.
		pf.read(Nchild, Nchildbuffer);
		
		memcpy(Nbuffer + 4 + 4, &endPid, sizeof(int));
		memcpy(Nchildbuffer + 4 + 4, &endPid, sizeof(int));		//set the parent pid to the new endPid
		
		if((rc = nonleaf_node.write(endPid, pf)) < 0)							//write the three nodes back.
		{return rc;}
		if((rc = pf.write(N, Nbuffer)) < 0)
		{ return rc;}
		if((rc = pf.write(Nchild, Nchildbuffer)) < 0)
		{return rc;}
		
		rootPid = endPid;										//update tree height and rootPid
		treeHeight++;
		return 0;		
	}
	RC rc;
	BTNonLeafNode nonleaf_node;
	BTNonLeafNode nonleaf_sibling;
	int midKey;
	
	PageId parent_pid;
	parent_pid = parent(N);

	nonleaf_node.read(parent_pid, pf);						//read the parent into the object!
	
	rc = nonleaf_node.insert(key, Nchild);					//insert the key value pair into the parent and check insert!
	if(rc == 0)
	{
		nonleaf_node.write(parent_pid, pf);
		return 0;
	}
	if(rc == RC_INVALID_ATTRIBUTE)
	{ return rc; }
	if(rc == RC_NODE_FULL) //setnextnodepntr for leaf and sibling, initialize root, write sibling, write root, treeHeight 
	{
		int endPid;
		endPid = pf.endPid();
		//to do - Make a sibling, insert the values into it, WRITE THE child and sibbling before u call the function again
		if((rc = nonleaf_node.insertAndSplit(key, Nchild, nonleaf_sibling, midKey)) < 0)
		{return rc;}
		nonleaf_sibling.setownPtr(endPid);
		nonleaf_sibling.setparentPtr(nonleaf_node.getparentPtr());
		
			/////write ALL the changes to the harddisk
		if((rc = nonleaf_sibling.write(endPid,pf)) < 0)						//write the sibbling iinto the disk before next function call.
		{return rc;}
		if((rc = nonleaf_node.write(parent_pid,pf)) < 0)		//write the parent to to commit the changes!
		{return rc;}
		
		
		/////NOTE! since half the values transferred to a new parent, we need to go thru this new parent, and set every parent pid that lies beneath this node!
		
		//int getKeyCount = 0;
		char *nonleaf_bufpntr = nonleaf_sibling.get_buffer();
		char temp_buffer[PageFile::PAGE_SIZE];
		int ij = 0;
		int temp_int;
		int temp_int2;
		int index_tochange[80];
		int size;
		
		
		size = nonleaf_sibling.getKeyCount();
		
		for(ij = 0; ij <= size; ij ++)
		{
			memcpy(&temp_int, nonleaf_bufpntr + 16 + 8*ij, sizeof(int) );
			index_tochange[ij] = temp_int;
		}
		nonleaf_bufpntr = NULL;
		for(temp_int = 0; temp_int < ij; temp_int++)	//ij is one mroe than size
		{
			temp_int2 = index_tochange[temp_int];
			if((rc = pf.read(temp_int2, temp_buffer)) < 0)
			{
				return rc;
			}
			
			memcpy(temp_buffer + 4 + 4, &endPid, sizeof(int));
			
			if((rc = pf.write(temp_int2, temp_buffer)) < 0)
			{
				return rc;
			}
		}
		
		///////now call insert in parent to insert values into the upper parent!
		
		return insert_in_parent(parent_pid, midKey, endPid);		
	}
}

///////////////////////////////////////////////
//remember! after each insert and insert and spilt, you need to write the node!

////////////////////////////////////////

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	RC rc;
	BTLeafNode leaf_node;
	leaf_node.setparentPtr(-1);
	leaf_node.setNextNodePtr(-1);
	
	BTLeafNode leaf_sibling;
	leaf_sibling.setparentPtr(-1);
	leaf_sibling.setNextNodePtr(-1);
	
	BTNonLeafNode nonleaf_node;
	nonleaf_node.setparentPtr(-1);
	
	BTNonLeafNode nonleaf_sibling;
	nonleaf_sibling.setparentPtr(-1);
	
	int siblingKey;
	int endPid;
	
	PageId nodepid;
	
	if(rootPid == -1)	//no record! root is same as leaf!
	{
		rootPid = pf.endPid();
		treeHeight = 1; 
		
		leaf_node.insert(key, rid);
		leaf_node.setownPtr(rootPid);
		leaf_node.setparentPtr(-1);
		leaf_node.setNextNodePtr(-1);
		
		if((rc = leaf_node.write(rootPid,pf)) < 0)
			return rc;							//error!

		return 0;
	}
	else
	{
		if(treeHeight == 1)
		{
			//PageId nodepid;
			leaf_node.read(rootPid, pf);
			rc = leaf_node.insert(key, rid);
			if(rc == RC_INVALID_ATTRIBUTE)
			{
				return rc;
			}
			if(rc == RC_NODE_FULL) //setnextnodepntr for leaf and sibling, initialize root, write sibling, write root, treeHeight = 2
			{
				rc = leaf_node.insertAndSplit(key, rid, leaf_sibling, siblingKey);
				endPid = pf.endPid(); //has the to be page id of sibling. Parents pid is going to be endPid + 1
				
				leaf_sibling.setparentPtr((endPid + 1));
				leaf_sibling.setownPtr(endPid);
				leaf_node.setparentPtr((endPid + 1));
				leaf_node.setNextNodePtr(endPid);
				nodepid = leaf_node.getownPtr();
				
				nonleaf_node.initializeRoot(nodepid, siblingKey, endPid); 	//write all value
				nonleaf_node.setownPtr((endPid + 1));						//initialize the pointers!
				nonleaf_node.setparentPtr(-1);
				
				if((rc = leaf_node.write(nodepid, pf)) < 0)
					return rc;
				if((rc = leaf_sibling.write(endPid, pf)) < 0)
					return rc;
				if((rc = nonleaf_node.write((endPid + 1), pf)) < 0)
					return rc;
				
				treeHeight = treeHeight + 1;
				rootPid = endPid + 1;
				
				return 0;
			}
			//no need to update tree height and such stuff.
			nodepid = leaf_node.getownPtr();
			leaf_node.write(nodepid, pf);
			
			return 0;			
		}
		//now to take care of the recursive cases!
		IndexCursor cursor;
		rc = locate(key, cursor); 
		if(rc == RC_NO_SUCH_RECORD)
		{
			rc = 0;
		}
		if(rc < 0)
		{return rc;}

		leaf_node.read(cursor.pid, pf);
		rc = leaf_node.insert(key, rid);
		if( rc == 0)		//write if insert is succesful
		{
			leaf_node.write(cursor.pid, pf);
			return 0;
		}
		if(rc == RC_INVALID_ATTRIBUTE)
			return rc;
		if( rc == RC_NODE_FULL)
		{
			rc = leaf_node.insertAndSplit(key, rid, leaf_sibling, siblingKey);
			if(rc < 0)
				return rc;
			
			endPid = pf.endPid(); //has the to be page id of sibling.
			leaf_sibling.setownPtr(endPid);
			leaf_sibling.setparentPtr(leaf_node.getparentPtr());
			leaf_node.setNextNodePtr(endPid);
			
			///write the changes of the leaf_node and leaf_sibling to the disk.
			if((rc = leaf_node.write(cursor.pid, pf)) < 0)
				return rc;
			if((rc = leaf_sibling.write(endPid, pf)) < 0)
				return rc;
			
			return insert_in_parent(leaf_node.getownPtr(), siblingKey, leaf_sibling.getownPtr());	//return whatever inseart_in_parent returns
			
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
	RC rc;
	int eid;
	PageId leaf_pid;
	BTLeafNode leaf_node;
	leaf_pid = locate_helper(searchKey, rootPid);
	if((rc = leaf_node.read(leaf_pid, pf)) < 0)
	{
		return rc;
	}
	if((rc = leaf_node.locate(searchKey, eid)) < 0)
	{
		cursor.pid = leaf_pid;
		cursor.eid = eid;	//BS HACK here!This is to make insert work. If attribute not found, eid has current key count.checked & inserted accordingly
		return rc;
	}
	//success! return the pid, eid pair!
	cursor.pid = leaf_pid;
	cursor.eid = eid;
    return 0;
}

//will recursively call till it reaches a leaf
PageId BTreeIndex::locate_helper(int key, PageId pid)
{
	//BTNonLeafNode nonleaf_node;
	//nonleaf_node.read(pid, pf);
	RC rc;
	char buffer2[PageFile::PAGE_SIZE];
	PageId look_pid;
	if((rc = pf.read(pid, buffer2)) < 0)
	{
		return rc;
	}
	if(buffer2[0] == '0')	//you have a non leaf. call locateChildPtr
	{
		BTNonLeafNode nonleaf_node;
		if((rc = nonleaf_node.read(pid, pf)) < 0)
		{
			return rc;
		}
		nonleaf_node.locateChildPtr(key, look_pid);
		return locate_helper(key, look_pid);		//change here!
	}
	else
		//if(buffer2[0] == '1')
		//{
	  return pid;
		//}
	//return look_pid;				//change here!
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
	RC rc;
	BTLeafNode leaf_node;
	//PageId next_pid;
	//int eid;
	//int max_elements;
	if((rc = leaf_node.read(cursor.pid, pf)) < 0)
	{
		return rc;
	}
	if((rc = leaf_node.readEntry(cursor.eid, key, rid)) < 0)
	{
		return rc;
	}
	//read succesful. advance the eid and return.
	if(cursor.eid < leaf_node.getKeyCount() - 1)
	{
		cursor.eid = cursor.eid + 1;
		return 0;
	}
	if(cursor.eid >= leaf_node.getKeyCount() - 1)
	{
		//eid points to the last pair. if we forward it, we must go to the next node!
		if(leaf_node.getNextNodePtr() < 0)
		{
			return RC_END_OF_TREE;
		}
		else
		{
			cursor.pid = leaf_node.getNextNodePtr();
			cursor.eid = 0;
			return 0;
		}
	}
    return 0;
}

PageId BTreeIndex::getrootPid()
{
	return rootPid;
}

long int BTreeIndex::gettreeHeight()
{
	return treeHeight;
}



/////////////////////////////////// PLEASE NOTE. THE FOLLOWING FUNCTIONS WERE PROVIDED BY SEUNG HYO CHOI for debugging purporses.//////

/* call this function from the test file. since we shouldn't have 
 * direct access to rootPid, this function calls another function 
 * that does have access to rootPid */
int BTreeIndex::print_tree()
{
	return print_tree_helper(rootPid);
}

// this is a modified locate()
// we had to have a helper and print_tree because our test 
// program doesn't have access to rootPid
int BTreeIndex::print_tree_helper(PageId pid)
{	
	int enteries = 0;
	RC rc = 0;
	// read the buffer of pid
	char buf[PageFile::PAGE_SIZE];
	rc = pf.read(pid, buf);

	// get the type of the pid we're currently at
	// 0 = non-leaf node, 1 = leaf node
	int type = 0;
	if(buf[0] == '0')
	{
		type = 0;
	}
	else
	{
		type = 1;
	}
	//memcpy(&type, buf, sizeof(int));

	if(type == 0)	// non-leaf node, so recurse
	{
		//RC rc = 0;

		// get the node at pid
		//char buf[PageFile::PAGE_SIZE];
		//rc = pf.read(pid, buf);
		int size = 0;
		memcpy(&size, buf+4, sizeof(int));

		char next[PageFile::PAGE_SIZE];
		int next_index = 0;

		cout << endl;
		cout << "Non Leaf Node " << endl;
		cout << "pid: \t" << pid << endl;
		cout << "count: \t" << size << endl;
		cout << endl;
		
		int par = 0;
		memcpy(&par, buf+8, sizeof(int));
		cout << "parent: " << par << endl;

		for(int i = 0; i < 2*size+1; i++)
		{
			int elem = 0;
			memcpy(&elem, buf+16+(4*i), sizeof(int));
			if(i%2 == 0)
			{
				next[next_index] = elem;
				next_index++;
			}
			if(i%2 == 0)
			{
				if(i == 2*size )
				{
					cout << " PageId :  " << elem;
				}
				else
				{
					cout << " PageId :  " << elem << " | ";
				}
			}
			else
			{
			cout <<" Key : "<< elem << " | ";
			}
		}
		cout << endl <<endl;

		for(int i = 0; i < next_index; i++)
		{
			enteries =  enteries + print_tree_helper(next[i]);
		}
	}

	else		// leaf node, so find the record and return
	{
		//RC rc = 0;

		// get the node at pid
		//char buf[PageFile::PAGE_SIZE];
		//rc = pf.read(pid, buf);
		int size = 0;
		memcpy(&size, buf+4, sizeof(int));
		
		cout << endl;
		cout << " Type Leaf Node "<<endl;
		cout << "pid: \t" << pid << endl;
		cout << "count: \t" << size << endl;
		enteries = size;
		cout << endl;
		
		int par = 0;
		memcpy(&par, buf+8, sizeof(int));
		cout << "parent: " << par << endl;

		//int which = 0;

		for(int i = 0; i < size; i++)
		{
			int elem = 0;
			RecordId rid;

			memcpy(&rid, buf+16+(12*i), 8);
			memcpy(&elem, buf+16+(12*i) + 8 , sizeof(int));
			if( i >= size - 1)
			{
				cout << " Record PageId : " << rid.pid << " | ";
				cout << " Key value : " << elem;
			}
			else
			{
				cout << " Record PageId : " << rid.pid << " | ";
				cout << " Key value : " << elem << " | ";
			}
		}
		int nextpage;
		memcpy(&nextpage, buf+1020, sizeof(int));
		cout << endl;
		cout << " Next Pageid Id: " << nextpage ;
		
		cout << endl;
		cout << endl;
	}
	return enteries;
}

void BTreeIndex::print_nodes()
{
    cout<<"number of nodes : " << pf.endPid() - 1 <<endl;

    char     page[PageFile::PAGE_SIZE];
    char     nodeType;
    int     entryCount;
    PageId     parentPtr, nextNodePtr;
   
    for(int i = 1; i < pf.endPid() ; i++)
    {
        pf.read(i, page);
        //memcpy(&nodeType, page, sizeof(char));
        memcpy(&entryCount, page+4, sizeof(int));
        memcpy(&parentPtr, page+8, sizeof(int));
   
		nodeType = page[0];
        //nodeType = nodeType + '0';
   
        if(nodeType == '0')
        {
            cout<<"pid: " << i << "\t node type: nonLeaf \t parent: "     << parentPtr << "\t entryCount: " << entryCount << endl;
        }
        else
        {
            memcpy(&nextNodePtr, page+1020, sizeof(int));
            cout<<"pid: " << i << "\t node type: leaf     \t parent: "     << parentPtr << "\t entryCount: " << entryCount
                <<"\t nextNodePtr: " << nextNodePtr << endl;
		}
	}
}

PageFile BTreeIndex::getpf()
{
	return pf;
}


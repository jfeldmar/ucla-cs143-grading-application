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
    RC rc = 0;
    rc = pf.open(indexname+".idx", mode);
    if(rc >= 0)
    {
    	// there's something in the file
	if(pf.endPid() != 0)	// don't do anything
	{
		// read the contents of tmp_buf to index[0]
		char tmp_buf[PageFile::PAGE_SIZE];
		pf.read(0, tmp_buf);
		memcpy(&rootPid, tmp_buf, sizeof(int));
		memcpy(&treeHeight, tmp_buf+4, sizeof(int));
	}
	else	// there's nothing in the file, so make a buffer and fill
	{
		char root_buf[PageFile::PAGE_SIZE];
		treeHeight = -1;	// set the height to -1
		memcpy(root_buf, &rootPid, sizeof(int));
		memcpy(root_buf+4, &treeHeight, sizeof(int));
		pf.write(0, root_buf);	// write the root buffer to position 0
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
    // write the final values of treeHeight and rootPid
    char tmp_buf[PageFile::PAGE_SIZE];
    memcpy(tmp_buf+4, &treeHeight, sizeof(int));
    memcpy(tmp_buf, &rootPid, sizeof(int));
    pf.write(0, tmp_buf);	// write the root buffer to position 0
    RC rc = 0;
    rc = pf.close();
    return rc;
}

void BTreeIndex::test_readForward(int searchGreater)
{
  IndexCursor ic;
  int counter = 0;
  int key;
  RecordId rid;

  cout<<"-----------------------------------------------"<<endl;
 
  cout<<"looking for greater than " << searchGreater << endl;
  locate(searchGreater,ic);

  while(  readForward(ic, key, rid) >= 0)
  {
        cout<<"key: " << key << endl;
        counter++;
  }
  cout<< counter << " keys found "<< endl;

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
        memcpy(&nodeType, page, sizeof(char));
        memcpy(&entryCount, page+4, sizeof(int));
        memcpy(&parentPtr, page+8, sizeof(int));
   
        nodeType = nodeType + '0';
   
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

/* returns the parent of a given node */
PageId BTreeIndex::get_parent(const PageId& pid)
{
    char buf[PageFile::PAGE_SIZE];
    pf.read(pid, buf);

    PageId par;
    memcpy(&par, buf+8, sizeof(int));
    return par;
}

RC BTreeIndex::insert_into_parent(PageId pid1, int key, PageId pid2)
{
   if(pid1 == rootPid)
   {
	BTNonLeafNode new_root;
	// set the parent of this root to -1
	new_root.set_parent_pid(-1);
	new_root.set_own_pid(pf.endPid());	// this is the new root
	rootPid = new_root.get_own_pid();			// update the pid of the root
	treeHeight++;				// since new root, height++

	// set the new root
	RC rc = new_root.initializeRoot(pid1, key, pid2);
	if(rc < 0)
		return rc;

	// set the parent of the children to the new root
	char buf1[PageFile::PAGE_SIZE];
	pf.read(pid1, buf1);
	memcpy(buf1+8, &rootPid, sizeof(int));
	rc = pf.write(pid1, buf1);
	
	if(rc < 0)
		return rc;

	char buf2[PageFile::PAGE_SIZE];
	pf.read(pid2, buf2);
	memcpy(buf2+8, &rootPid, sizeof(int));
	rc = pf.write(pid2, buf2);

	if(rc < 0)
		return rc;

	new_root.write(rootPid, pf);		// write this to the PageFile

	if(rc < 0)
		return rc;
	
	return 0;
   }
   else		// pid1 is not rootPid
   {
	PageId parent_pid = get_parent(pid1);	// get the parent of the two children
	BTNonLeafNode parent;
	parent.read(parent_pid, pf);		// read the content of the parent into memory

	RC rc = parent.insert(key, pid2);	// insert values into the parent
	
	if(rc == 0)
	{
		parent.write(parent_pid, pf);
		return 0;
	}

	else if(rc == RC_NODE_FULL)		// need to do insert and split
	{
		BTNonLeafNode sib;
		int sibkey = 0;
		rc = parent.insertAndSplit(key, pid2, sib, sibkey);
		if(rc < 0)
			return rc;

		rc = parent.write(parent_pid, pf);
		if(rc < 0)
			return rc;

		int epid = pf.endPid();
		// set the values within sibling and stuff
		sib.set_own_pid(epid);			// insert into next available space
		sib.set_parent_pid(parent.get_parent_pid());
		rc = sib.write(epid, pf);
		
		if(rc < 0)
			return rc;

		// need to update the parent pid of every child in sib
		char buf[PageFile::PAGE_SIZE];		// store a buffer to get the size
		rc = pf.read(sib.get_own_pid(), buf);

		if(rc < 0)
			return rc;

		int size = sib.getKeyCount();

		int next[PageFile::PAGE_SIZE];		// store the new pids here
		int next_index = 0;

		char* sib_buf = sib.get_buffer();

		for(int i = 0; i <= size; i++)	// copy the pid of each child
		{
		    int elem = 0;
		    memcpy(&elem, sib_buf+16+(8*i), sizeof(int));
		    next[i] = elem;
		}

		char buf2[PageFile::PAGE_SIZE];
		for(int i = 0; i <= size; i++)
		{
			rc = pf.read(next[i], buf2);
			if(rc < 0)
				return 0;
			memcpy(buf2+8, &epid, sizeof(int));
			rc = pf.write(next[i], buf2);
			if(rc < 0)
				return 0;
		}

		return insert_into_parent(parent_pid, sibkey, sib.get_own_pid());
	}
	else
		return rc;
   }
}

/* return the rootPid */
PageId BTreeIndex::getrootPid()
{
	return rootPid;
}

/* return the height of the tree */
int BTreeIndex::gettreeHeight()
{
	return treeHeight;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
    BTLeafNode ln, sib;
    BTNonLeafNode nln;
    int res = 0;		// for checking errors on write
    ln.setNextNodePtr(-1);	// set the next pointer to nothing
    ln.set_parent_pid(-1);	// this has no parent
    ln.set_own_pid(-1);		// this is just a safe
    sib.setNextNodePtr(-1);	// set the next pointer to nothing
    sib.set_parent_pid(-1);	// this has no parent
    sib.set_own_pid(-1);	// this is a safe
    nln.set_parent_pid(-1);	// this has no parent
    nln.set_own_pid(-1);	// this is a safe

    // if nothing is inserted; index is empty
    if(rootPid == -1)
    {
    	rootPid = pf.endPid();		// the next available pointer 
	ln.set_own_pid(rootPid);	// set the pointer of itself to rootPid
	res = ln.insert(key, rid);	// insert it into the leaf node
	if(res < 0)
		return res;

	res = ln.write(rootPid, pf);	// write this node to the pagefile
	if(res < 0)
		return res;
	treeHeight = 1;			// since we have a level
    }

    // there's something in the root already
    else
    {
	// there's only one leafnode in the index so we just insert into that
	if(treeHeight == 1)
	{
		ln.read(rootPid, pf);	// get the new root node
		res = ln.insert(key, rid);
		if(res == RC_INVALID_ATTRIBUTE)
		{
			return res;
		}

		if(res == RC_NODE_FULL)
		{
			int sib_key = 0;
			res = ln.insertAndSplit(key, rid, sib, sib_key);
			rootPid = pf.endPid()+1;	// 3
			sib.set_own_pid(pf.endPid());
			sib.set_parent_pid(rootPid); // sibling's parent's pid = ln's parent
			
			ln.set_parent_pid(rootPid); // the parent is the root
			nln.initializeRoot(ln.get_own_pid(), sib_key, sib.get_own_pid());
			nln.set_own_pid(rootPid); // is parent/root now
			nln.set_parent_pid(-1);
			sib.setNextNodePtr(ln.getNextNodePtr());
			ln.setNextNodePtr(sib.get_own_pid()); // set sibling to sib
			//sib.setNextNodePtr(ln.getNextNodePtr());
			//ln.setNextNodePtr(pf.endPid()); // set sibling to sib
			// since leafnode ln was already written before, no need to 
			// set self pointers or anything

			res = sib.write(sib.get_own_pid(), pf);
			if(res < 0)
				return res;
			res = ln.write(ln.get_own_pid(), pf);
			if(res < 0)
				return res;
			res = nln.write(nln.get_own_pid(), pf);
			if(res < 0)
				return res;

			// we now have a parent and two sibling nodes
			treeHeight = 2;
			return 0;
		}
		else	// not full node, just insert
			ln.write(ln.get_own_pid(), pf);

		return 0;
	}
	else	// the index is a fully fledged index, so need to do recursive calls
	{
		// locate the leaf to insert into
		IndexCursor ic;
		PageId insert_pid;
	
		// problem: locate returns RC_INVALID ATTRIBUTE
		// since attribute is larger than largest attribute available
		// and 6 is greater than 5 so it will look for it but it 
		// can't find it, so it returns the RC. if it returns this RC, go 
		// ahead with insert.
		// when you're returning the RC, return the pid. so set the pid 
		// before it sends the RC.
		RC rc = 0;
		rc = locate(key, ic);

		if(rc == RC_NO_SUCH_RECORD)
			rc = 0;

		if(rc < 0)
			return rc;

		// insert into the leaf
		BTLeafNode ln;

		ln.read(ic.pid, pf);
		rc = ln.insert(key, rid);

		// if insert successful, return done
		if(rc == 0)
		{
			rc = ln.write(ic.pid, pf);
			return 0;
		}

		else	// insert into parent
		{
			if(rc == RC_NODE_FULL)
			{
				BTLeafNode sib;
				int sibkey = 0;
				rc = ln.insertAndSplit(key, rid, sib, sibkey);
				if(rc < 0)
					return rc;
				
				sib.set_own_pid(pf.endPid());
				sib.set_parent_pid(ln.get_parent_pid());
				sib.setNextNodePtr(ln.getNextNodePtr());

				ln.setNextNodePtr(pf.endPid());

				rc = sib.write(sib.get_own_pid(), pf);
				if(rc < 0)
					return rc;

				rc = ln.write(ln.get_own_pid(), pf);
				if(rc < 0)
					return rc;

				/*bleh = ln.getNextNodePtr(); 
				cout << "next: " << bleh << endl;

				cout << "sib" << ln2.get_own_pid() << endl;
				cout << "ln" << ln.get_own_pid() << endl;*/
				return insert_into_parent(ln.get_own_pid(), sibkey, sib.get_own_pid());
			}
			else
				return rc;
		}
	}
    }
    return 0;
}

/* call this function from the test file. since we shouldn't have
 * direct access to rootPid, this function calls another function
 * that does have access to rootPid */
int BTreeIndex::print_tree()
{
    //cout << gettreeHeight() << endl;
    return print_tree_helper(rootPid);
}

// this is a modified locate()
// we had to have a helper and print_tree because our test
// program doesn't have access to rootPid
// originally created by me, updated by tarun solanki
int BTreeIndex::print_tree_helper(PageId pid)
{   
    int num_entries = 0;
    RC rc = 0;
    // read the buffer of pid
    char buf[PageFile::PAGE_SIZE];
    rc = pf.read(pid, buf);

    // get the type of the pid we're currently at
    // 0 = non-leaf node, 1 = leaf node
    int type = 0;
    memcpy(&type, buf, sizeof(int));

    if(type == 0)    // non-leaf node, so recurse
    {
        RC rc = 0;

        // get the node at pid
        char buf[PageFile::PAGE_SIZE];
        rc = pf.read(pid, buf);
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
	    num_entries += print_tree_helper(next[i]);
        }
    }

    else        // leaf node, so find the record and return
    {
        RC rc = 0;

        // get the node at pid
        char buf[PageFile::PAGE_SIZE];
        rc = pf.read(pid, buf);
        int size = 0;
        memcpy(&size, buf+4, sizeof(int));
       
        cout << endl;
        cout << " Type Leaf Node "<<endl;
        cout << "pid: \t" << pid << endl;
        cout << "count: \t" << size << endl;
        cout << endl;
       
        num_entries = size;

        int par = 0;
        memcpy(&par, buf+8, sizeof(int));
        cout << "parent: " << par << endl;

        int which = 0;

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

	BTLeafNode ln;
	ln.read(pid, pf);
        cout << " Next Pageid Id: " << ln.getNextNodePtr();
       
        cout << endl;
        cout << endl;
    }
    return num_entries;
}

// do we need to do anything special for a new root that has 
// only a few entries?
PageId BTreeIndex::locate_pid(int key, PageId pid)
{
	RC rc = 0;
	// read the buffer of pid
	char buf[PageFile::PAGE_SIZE];
	rc = pf.read(pid, buf);
	
	if(rc < 0)
		return rc;

	// get the type of the pid we're currently at
	// 0 = non-leaf node, 1 = leaf node
	int type = 0;
	memcpy(&type, buf, sizeof(int));

	if(type == 0)	// non-leaf node, so recurse
	{
		RC rc = 0;

		// get the node at pid
		BTNonLeafNode nln;
		rc = nln.read(pid, pf);	// get the new root node

		if(rc < 0)
			return rc;
		
		// get the child that we need to go to
		PageId m_pid;
		nln.locateChildPtr(key, m_pid);
		
		return locate_pid(key, m_pid);
	}

	else		// leaf node, so find the record and return
	{
		return pid;
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
    RC rc = 0;
    // recursively find the location of the recordid
    // by calling locate_rid. locate_rid returns the pid of 
    // the node
    PageId found_pid;
    found_pid = locate_pid(searchKey, rootPid);	// this is a leaf node of some sort

    // read the leaf node
    BTLeafNode ln;
    rc = ln.read(found_pid, pf);
    
    if(rc < 0)
    	return rc;

    // locate the proper entry id in the pid with the searchKey
    int m_eid = 0;
    rc = ln.locate(searchKey, m_eid);

    if(rc < 0)
    {
	cursor.pid = found_pid;
	cursor.eid = m_eid;
    	return rc;
    }

    // set the cursor's member variables
    cursor.pid = found_pid;
    cursor.eid = m_eid;

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
    BTLeafNode ln;
    RC rc = 0;
    rc = ln.read(cursor.pid, pf);

    if(rc < 0)
    	return rc;

    rc = ln.readEntry(cursor.eid, key, rid);

    if(rc < 0)
    	return rc;

    if(cursor.eid >= ln.getKeyCount() - 1)	// go to the next pid
    {
    	if(ln.getNextNodePtr() < 0)
		return RC_END_OF_TREE;

	cursor.pid = ln.getNextNodePtr();
	cursor.eid = 0;
    }
    else	// the page still has space, so just need to go to next entry
    	cursor.eid++;

    return 0;
}

PageFile BTreeIndex::getpf()
{
	return pf;
}

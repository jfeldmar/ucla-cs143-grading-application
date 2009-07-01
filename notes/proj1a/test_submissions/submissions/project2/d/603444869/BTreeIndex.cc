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
BTreeIndex::BTreeIndex(string pf_file)
{
    rootPid = -1;
    treeHeight = 0;
    indexname = pf_file;
    indexnameN = pf_file+".idx";
}

/*
 * open_helper the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open_helper(const string& indexnam, char mode)
{
    return pf.open(indexnam,mode);
}

RC BTreeIndex::open(const string& indexname, char mode)
{
    char tmp[5000];
    if(!open_helper(indexname,'r')) {
    pf.read(99999,tmp);
    //cout << "opened up and read in: " << tmp << endl;
    int i = 0;
    int arr[500];
    int count=0;
    while(tmp[i] != '\0') {
        while(tmp[i] == ' ')
            i++;
        int j = i;
        while(tmp[j] != ' ' && tmp[j] != '\0') {
            j++;
        }
        int size = j-i;
        int multiplier;
        if(tmp[i] == '-') {
            multiplier = -1;
            i++;
        } else
        multiplier = 1;
        for(int m = 1; m < size; m++)
            multiplier *= 10;
        int get = 0;
        while(tmp[i] != ' ') {
            if(tmp[i] == '\0')
                break;
            int digit = tmp[i]-'0';
            get += digit*multiplier;
            multiplier /= 10;
            i++;
        }
        arr[count] = get;
        count++;
    }
    treeHeight = arr[0];
    //cout << "treeHeight " << treeHeight << endl;
    rootPid = arr[1];
    //cout << "rootPid " << rootPid << endl;
    close_helper();
    }
    return 0;
    //return pf.open(indexname,mode);
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close_helper()
{
    return pf.close();
}

RC BTreeIndex::close()
{
    for(map<int,RecordId>::iterator mi = my_tree.begin(); mi != my_tree.end(); mi++) {
        insert_real(mi->first,mi->second);
    }
    char s[5000]; 
    int tmp;
    int count = 0;
    bool iszero = true;
    bool isneg = false;
    tmp = rootPid;
    if (tmp < 0) {
        isneg= true;
        tmp*=-1;
    }
    while(tmp > 0) {
        iszero = false;
            s[count] = (tmp % 10) + '0';
            count++;
            tmp /= 10;
    }
    if(isneg) {
        s[count] ='-';
        count++;
    }
    if(iszero)
    {
        s[count] = '0';
        count++;
    }
        s[count] = ' ';
        count++;
    tmp = treeHeight;
    iszero = true;
    while(tmp > 0) {
        iszero = false;
            s[count] = (tmp % 10) + '0';
            count++;
            tmp /= 10;
    }
    if(iszero)
    {
        s[count] = '0';
        count++;
    }
        s[count] = ' ';
        count++;
    char* rev =new char[500];
    for(int i = 0; i < count; i++) {
        rev[i] = s[count-i-1];
    }
    open_helper(indexname,'w');
    pf.write(99999,rev); //use 999999 for root pid
    //cout << "closed and wrote in: " << rev << endl;
    close_helper();
    return 0;
    //return pf.close();
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert_split(int key, const RecordId& rid, PageId curr_pid, int curr_level)
{
    //we want to start at leaf level and work our way up
   if(curr_level == treeHeight) { //curr_level = treeHeight means we're at a leaf node
       //cout << "cur_level == treeHeight\n";
        BTLeafNode* leaf2 = new BTLeafNode();
        open_helper(indexname,'r');
        leaf2->read(curr_pid, pf);
        close_helper();
        if(!(leaf2->getKeyCount() >= MAX_RECORDS-1)) {
            //not full, did normal insert
            leaf2->insert(key, rid);
            //cout << "doing normal insert into pid: " << curr_pid << "\n";
            open_helper(indexname,'w');
            leaf2->write(leaf2->recordid[0].pid, pf);
            if(treeHeight <= 1)
                rootPid = leaf2->recordid[0].pid;
            close_helper();
        } else { //it's full,do insert split (and recursively insert into parent node)
            //cout << "it's full!\n";
            BTLeafNode* new_leaf=new BTLeafNode();
            int new_leaf_key;
            //cout << "inserting and splitting key: " << key << " pid: " << rid.pid << '\n';
            leaf2->insertAndSplit(key,rid,*new_leaf,new_leaf_key);
            open_helper(indexname,'w');
            leaf2->write(leaf2->recordid[0].pid,pf);
            new_leaf->write(new_leaf->recordid[0].pid,pf); 
            //cout << "new leaf2 pid: " << leaf2->recordid[0].pid << endl;
            close_helper();
            if(treeHeight == 1) { //create the parent node
                //cout << "creating parent node\n";
                open_helper(indexnameN,'w');
                BTNonLeafNode* new_parent = new BTNonLeafNode(leaf2->recordid[0].pid,new_leaf_key,new_leaf->recordid[0].pid);
                treeHeight++;
                new_parent->write(new_parent->first_pid+(treeHeight-curr_level+1)*1000+MAX_RECORDS,pf);
                rootPid = new_parent->first_pid;
                close_helper();
            } else {
                //else the parent node exists so we recursively call insert_split starting with the parent node
                //todo: check if old parent is full
                //cout << "using old parent\n";
                //cout << "current level is " << curr_level << "\n";
                //cout << "treeHeight is " << treeHeight<< "\n";
                IndexCursor index;
                index.pid = rootPid;
                PageId non_leaf_pid = non_leaf_locate(new_leaf_key,index,1,curr_level-1);
                //cout << "  ..with pid.." << non_leaf_pid << '\n';
                //in case the smallest leaf key was changed...
                BTNonLeafNode* nonleaf = new BTNonLeafNode();
                open_helper(indexnameN,'r');
                nonleaf->read(non_leaf_pid+(treeHeight-curr_level+2)*1000+MAX_RECORDS, pf);
                close_helper();
                open_helper(indexnameN,'w');
                nonleaf->write(non_leaf_pid+(treeHeight-curr_level+2)*1000+MAX_RECORDS, pf);
                close_helper();
                return insert_split(new_leaf_key,new_leaf->recordid[0],non_leaf_pid,curr_level-1);
            }
            close_helper();
        }
   } else {
       //we're working with a parent node
       //cout << "working on parent node with pid " << curr_pid << "\n";
        BTNonLeafNode* nonleaf = new BTNonLeafNode();
        open_helper(indexnameN,'r');
        nonleaf->read(curr_pid+(treeHeight-curr_level+1)*1000+MAX_RECORDS, pf);
        close_helper();
        if(!(nonleaf->getKeyCount() >= MAX_RECORDS-1)) {
            //cout << "inserting key " << key << " directly into parent node\n";
            nonleaf->insert(key, rid.pid);
            /*
            */
        open_helper(indexnameN,'w');
        nonleaf->write(nonleaf->first_pid+(treeHeight-curr_level+1)*1000+MAX_RECORDS,pf);
        close_helper();
        if(curr_level == 1)
        {
            rootPid = nonleaf->first_pid;
        }
        /*
        open_helper(indexnameN, 'r');
        nonleaf->read(rootPid+11000*MAX_RECORDS+MAX_RECORDS,pf);
        close_helper();
        //cout << "  numkeys= " << nonleaf->numKeys << '\n';
        //cout << "  first_pid= " << nonleaf->first_pid<< '\n';
        */
        } else {
            //parent is full
            //do insertAndSplit with nonleaf node
            //cout << "at curr level: " << curr_level << '\n';
            //cout << "parent is full!\n";
            BTNonLeafNode* new_non_leaf=new BTNonLeafNode();
            int new_non_leaf_key;
            nonleaf->insertAndSplit(key,rid.pid,*new_non_leaf,new_non_leaf_key);
            open_helper(indexnameN,'w');
            new_non_leaf->write(new_non_leaf->first_pid+(treeHeight-curr_level+1)*1000+MAX_RECORDS,pf);
            nonleaf->write(nonleaf->first_pid+(treeHeight-curr_level+1)*1000+MAX_RECORDS,pf); 
            close_helper();
            
            if(curr_level == 1) { //create the parent node
                //cout << "parent doesn't exist creating new root node\n";
                open_helper(indexnameN,'w');
                BTNonLeafNode* new_parent = new BTNonLeafNode(nonleaf->first_pid,new_non_leaf_key,new_non_leaf->first_pid);
                treeHeight++;
                new_parent->write(new_parent->first_pid+(treeHeight)*1000+MAX_RECORDS,pf);
                //cout << "writing to: " << new_parent->first_pid+(treeHeight)*1000+MAX_RECORDS << "\n";
                rootPid = new_parent->first_pid;
                close_helper();
            } else {
                //parent exists do insert_split
                RecordId* my_rid = new RecordId();
                my_rid->pid = new_non_leaf->first_pid;
                my_rid->sid = 0;
                IndexCursor index;
                index.pid = rootPid;
                PageId non_leaf_pid = non_leaf_locate(new_non_leaf_key,index,1,curr_level-1);
                //cout << "parent exists...doing insert_split again, on level: " << curr_level-1 << " with pid: " << non_leaf_pid<<" \n";
                //making sure the smallest first_pid gets updated
                BTNonLeafNode* nonleaf = new BTNonLeafNode();
                open_helper(indexnameN,'r');
                //cout << "reading now...\n";
                nonleaf->read(index.pid+(treeHeight-curr_level+2)*1000+MAX_RECORDS, pf);
                close_helper();
                open_helper(indexnameN,'w');
                nonleaf->write(index.pid+(treeHeight-curr_level+2)*1000+MAX_RECORDS, pf);
                close_helper();
                return insert_split(new_non_leaf_key,*my_rid,non_leaf_pid,curr_level-1);
            }
        } 
   } 
    return 0;
}

RC BTreeIndex::insert_real(int key, const RecordId& rid)
{
    if ( rootPid == -1) { //no root node exists
        BTLeafNode* leaf = new BTLeafNode();
        leaf->insert(key,rid);
        rootPid = leaf->recordid[0].pid;
        open_helper(indexname,'w');
        leaf->write(rootPid,pf);
        close_helper();
        //cout << "first numKeys: " << leaf->numKeys << '\n';
        treeHeight++;
    } else {
        //use locate to find the pid of the leaf node, change 1 below to treeHeight
        IndexCursor cursor;
        locate(key,cursor);
        //cout << "located for key " << key << " pid: " << cursor.pid << '\n';
        insert_split(key,rid,cursor.pid,treeHeight);
    }
    //cout << "debuggin...\n";
    //Debug(rootPid,1);
    return 0;
}

RC BTreeIndex::insert(int key, const RecordId& rid)
{
    my_tree[key] = rid;
    return 0;
}

/*
RC BTreeIndex::insert(int key, const RecordId& rid)
{
    my_tree[key] = rid;
}
*/

RC BTreeIndex::Debug(PageId curr, int curr_level)
{
    if(curr_level > treeHeight) {
        return 0;
    } 
    if(curr_level < treeHeight) {
        BTNonLeafNode* nonleaf = new BTNonLeafNode();
        open_helper(indexnameN, 'r');
        nonleaf->read(curr+(treeHeight-curr_level+1)*1000+MAX_RECORDS,pf);
        //cout << "nonleaf reading: " << curr+(treeHeight-curr_level+1)*1000+MAX_RECORDS << "\n";
        close_helper();
        cout << "level= " << curr_level << '\n';
        cout << "  numkeys= " << nonleaf->numKeys << '\n';
        cout << "  first_pid= " << nonleaf->first_pid<< '\n';
        cout << "level= " << curr_level << '\n';
        for(int i = 0; i < nonleaf->numKeys; i++) {
            cout << "  " << i << ". " << nonleaf->key[i] << "|" <<  nonleaf->pageid[i] << '\n';
        }
        //Debug(nonleaf->first_pid,curr_level+1);
        //
        //for(int i = 0; i < nonleaf->numKeys; i++) {
        //    Debug(nonleaf->pageid[i],curr_level+1);
        //}
        close_helper();
    }
    if(curr_level == treeHeight) {
        BTLeafNode* leaf = new BTLeafNode();
        open_helper(indexname, 'r');
        leaf->read(curr,pf);
        close_helper();
        cout << "level= " << curr_level << '\n';
        cout << "  numkeys= " << leaf->numKeys << '\n';
        cout << "  next_node= " << leaf->next_node<< '\n';
        for(int i = 0; i < leaf->numKeys; i++) {
            cout << "  " << i << ". " << leaf->key[i] << "|" << leaf->recordid[i].pid<< "|" << leaf->recordid[i].sid << '\n';
        }
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
PageId BTreeIndex::leftmost_change(int searchKey, PageId left_pid, IndexCursor& cursor, int curr_level)
{
    cout << "doing leftmost change at: " << curr_level << " with " << left_pid << endl;
        BTNonLeafNode* parent = new BTNonLeafNode();
        open_helper(indexnameN,'r');
        parent->read(cursor.pid+(treeHeight-curr_level+1)*1000+MAX_RECORDS, pf);
        close_helper();
        cout << "parent key old: " << parent->key[0] << " " << "searchKey: " << searchKey << endl;
        if(parent->key[0] > searchKey) {
            //parent->key[0] = searchKey;
            parent->first_pid = left_pid;
            open_helper(indexnameN,'w');
            parent->write(parent->first_pid+(treeHeight-curr_level+1)*1000+MAX_RECORDS, pf);
            close_helper();
            cout << "new pid is: " << parent->first_pid+(treeHeight-curr_level+1)*1000+MAX_RECORDS;
            if(curr_level == 1) {
                rootPid = left_pid;
            } else {
                return leftmost_change(searchKey, left_pid, cursor, curr_level-1); 
            }
        } 
    return 1;
}
PageId BTreeIndex::non_leaf_locate(int searchKey, IndexCursor& cursor, int curr_level, int stop_level)
{
    if(curr_level == stop_level) //at stop level
    {
        return cursor.pid; //cursor should be at the leaf level already
    } else {
        BTNonLeafNode* parent = new BTNonLeafNode();
        open_helper(indexnameN,'r');
        parent->read(cursor.pid+(treeHeight-curr_level+1)*1000+MAX_RECORDS, pf);
        close_helper();
        parent->locateChildPtr(searchKey, cursor.pid);
        return non_leaf_locate(searchKey, cursor, curr_level+1, stop_level); 
    }
    return 1;
}

RC BTreeIndex::locate_helper(int searchKey, IndexCursor& cursor, int curr_level)
{
    if(curr_level == treeHeight) //at leaf level
    {
        BTLeafNode found;
        open_helper(indexname,'r');
        found.read(cursor.pid, pf);
        close_helper();
        found.locate(searchKey,cursor.eid);//get the eid and store it in cursor
        return 0; //cursor should be at the leaf level already
    } else {
        //cout << "read from " << indexnameN << endl;
        open_helper(indexnameN,'r');
        BTNonLeafNode parent; //this causes a segfualt?
        parent.read(cursor.pid+(treeHeight-curr_level+1)*1000+MAX_RECORDS, pf);
        close_helper();
        //cout << "parent id: " << parent.key[0] << endl;
        parent.locateChildPtr(searchKey, cursor.pid);
        //cout << "located child: " << cursor.pid << endl;
        return locate_helper(searchKey, cursor, curr_level+1); 
    }
    return 1;
}

RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
    cursor.pid = rootPid;
    //cout << "using rootPid: " << rootPid << endl;
    locate_helper(searchKey, cursor, 1);
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
    BTLeafNode leaf2;
    open_helper(indexname,'r');
    leaf2.read(cursor.pid, pf);
    close_helper();
    leaf2.readEntry(cursor.eid,key,rid);
    if(cursor.eid < leaf2.numKeys-1) {
        cursor.eid = cursor.eid+1;
    } else {
        cursor.eid = 0;
        cursor.pid = leaf2.next_node;
    }
    return 0;
}

RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid, bool getnext)
{
    BTLeafNode leaf2;
    open_helper(indexname,'r');
    leaf2.read(cursor.pid, pf);
    close_helper();
    leaf2.readEntry(cursor.eid,key,rid);
    return 0;
}
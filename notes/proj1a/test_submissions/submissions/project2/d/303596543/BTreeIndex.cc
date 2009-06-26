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
const int LeafNode = 0;
const int RootNode = 1;
const int NonleafNode = 2;
int rawcount=0;
/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
//	fprintf(stdout, "BTreeI tNodes %d treelist %d bytes array %d bytes %d nNodes nodesize %d\n",
//		tNodes,sizeof(TreeListType),sizeof(treelist),nNodes,NodeSize);
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
	if (pf.open(indexname,mode) != 0)
	{
		fprintf(stdout, "Unable to open file: %s\n",indexname.c_str());
		return RC_FILE_OPEN_FAILED;
	}
	int last=pf.endPid();
	treecount = 0;
	rawcount = 0;
	if (last) // meaning there is an existing index
	{
		for(PageId i=0; i < StartPid; i++) {
			pf.read(i,&treelist[i*tNodes/StartPid]); // first read in how the index is organized
		}
		for(PageId i=StartPid; i < last; i++)
		{
			if (treelist[i].nodeid == LeafNode) {
				treelist[i].lnode = new BTLeafNode();
				treelist[i].lnode->read(i,pf);
			}
			else if (treelist[i].nodeid == RootNode){
				treelist[i].nnode = new BTNonLeafNode();
				treelist[i].nnode->read(i,pf);
				rootPid = i;
			}
			else {
				treelist[i].nnode = new BTNonLeafNode();
				treelist[i].nnode->read(i,pf);
			}
			treecount++;
		} // endfor loop of pageid's
	} else
	{	// array index 0 is never used since page 0 is used for the treelist
		treelist[StartPid].nnode = new BTNonLeafNode();
		treelist[StartPid].nodeid = RootNode;
		treelist[StartPid].nnode->initializeRoot(StartPid+1,nNodes*3,StartPid+2);

		treelist[StartPid+1].lnode = new BTLeafNode();
		treelist[StartPid+1].nodeid = LeafNode;
		treelist[StartPid+1].lnode->setNextNodePtr(StartPid+2);

		treelist[StartPid+2].lnode = new BTLeafNode();
		treelist[StartPid+2].nodeid = LeafNode;

		treecount = StartPid+2;
		rootPid = StartPid;
		treeHeight =2;
	} // endif last == 0
	return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	//fprintf(stdout, "Rawcount=%d\n",rawcount);
	int cc=0;
	if (treecount == 0) return 0;
	for(PageId i=0; i < StartPid; i++) {
		pf.write(i,&treelist[i*tNodes/StartPid]); // first write out how the index is organized
	}
	for(PageId i=StartPid; i <= treecount; i++)
	{
		if (treelist[i].nodeid == LeafNode) {
			treelist[i].lnode->write(i,pf);
			cc += treelist[i].lnode->getKeyCount();
			delete treelist[i].lnode;
		}
		else {
			treelist[i].nnode->write(i,pf);
			cc += treelist[i].nnode->getKeyCount();
			delete treelist[i].nnode;
		}
	} // endfor loop of pageid's
	pf.close();
	//fprintf(stdout, "key total=%d\n",cc);
    return 0;
}
/* getParent
param: key[IN] the key to locate
param: cpid[IN] the child pid of the leaf node
*/
PageId BTreeIndex::getParent(int key,PageId cpid)
{
	PageId	lastpid,pid=rootPid;
	RC	rc;
	do // walk thru the list.
	{
		// Each loop updates the pid until we get to the leaf node equal to pid
		if (treelist[pid].nodeid == NonleafNode || treelist[pid].nodeid == RootNode)
		{
			lastpid = pid;
			rc = treelist[pid].nnode->locateChildPtr(key, pid); // nonleaf
		}
		// this loop runs only on Nonleaf nodes and stop at finding 'cpid' or a leaf node
	} while((treelist[pid].nodeid == NonleafNode || treelist[pid].nodeid == RootNode)
		&& cpid != pid);
//	fprintf(stdout, "Parent of %d is %d\n",cpid,lastpid);
	return lastpid;
}
/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 *
 *Algorithm cases:
1. If leaf node is full, then create a new leaf and split the old leaf.
	The new split node's first key (midkey) is inserted into root/parent.
2. If root/parent is full while inserting midkey, then create a new nonleaf node and 
	split the root/parent. Insert the midkey of old nonleaf node midkey2 into parent.
3. The troublesome part is if the root/parent key is the root of the tree, another nonleaf
	node needs to be created and gets midkey2 as its initial key while the 2 other nonleaf
	nodes become children of the new root.
*/
RC BTreeIndex::incTreecount()
{
	treecount++;
	RC rc=0;
//	fprintf(stdout, "treecount %d\n",treecount);
	if (treecount >= tNodes-1)
	{
		fprintf(stdout, "Error: treecount %d exceeds tnodes %d. rawcount %d\n",treecount,tNodes,rawcount);
		rc = RC_NODE_FULL;
		treecount = tNodes;
	}
	return rc;
}
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	IndexCursor csr;
	RC	rc=0;
	int		midkey,midkey2;
	PageId	newpid;
	BTLeafNode	*oldleaf,*newleaf;
	rc = locate(key,csr);	// returns the start of leaf node
	oldleaf = treelist[csr.pid].lnode;
	rc = oldleaf->insert(key,rid);
	rawcount++;
	if (rc != 0 && treelist[csr.pid].nodeid == LeafNode)
	{
		// case 1:  csr.pid leaf node full
		rc = incTreecount();
//		fprintf(stdout, "Leaf Node %d is full. Created node %d\n",csr.pid,treecount);
//		oldleaf->printNode();
		treelist[treecount].lnode = new BTLeafNode();
		treelist[treecount].nodeid = LeafNode;
		newleaf = treelist[treecount].lnode;
		rc = oldleaf->insertAndSplit(key, rid, 
							  treelist[treecount].lnode, midkey);
		PageId linknode = oldleaf->getNextNodePtr();
		oldleaf->setNextNodePtr(treecount);
		newleaf->setNextNodePtr(linknode);
		PageId ppid = getParent(key,csr.pid);
		newleaf->write(treecount,pf);
		// insert midkey into parent. Parent is always a nonleaf node
		rc = treelist[ppid].nnode->insert(midkey,treecount);
		//treelist[ppid].nnode->printNode();
		newpid = treecount;
		if (rc != 0)
		{
			// case: 2 nonleaf node (ppid) is full so split it
			rc = incTreecount();
//			fprintf(stdout, "Nonleaf Node %d is full. Created node %d\n",ppid,treecount);
//			treelist[ppid].nnode->printNode();
			treelist[treecount].nnode = new BTNonLeafNode();
			treelist[treecount].nodeid = NonleafNode;
			rc = treelist[ppid].nnode->insertAndSplit(midkey, newpid, 
								  treelist[treecount].nnode, midkey2);
			PageId pxpid = getParent(midkey2,ppid);
			treelist[treecount].nnode->write(treecount,pf);
			// insert midkey2 into parent but if this node is root, then create a new root.
			//rc = treelist[ppid].nnode->insert(midkey2,treecount);
			newpid = treecount;
			if (ppid == rootPid && treelist[ppid].nodeid == RootNode)
			{
				// case 3:  the nonleaf node just split is the root! Now the 2 nodes descend
				//			as nonleafs and now create a new root.
//				fprintf(stdout, "Index:insert: Creating new root from %d to %d\n",rootPid,treecount);
				rc = incTreecount();
				treeHeight++;
				treelist[treecount].nnode = new BTNonLeafNode();
				treelist[treecount].nodeid = RootNode;
				treelist[ppid].nodeid = NonleafNode;
				treelist[treecount].nnode->initializeRoot(ppid,midkey2,newpid);
				treelist[treecount].nnode->write(treecount,pf);
				rootPid=treecount;
			} else
			{
				// insert midkey2 into parent. Parent is always a nonleaf node
				rc = treelist[pxpid].nnode->insert(midkey2,treecount);
			}
		} /// endif nonleaf node is full
	} // endif rc on leafnode is full
	else if (rc != 0)
	{
		fprintf(stdout, "Index::insert Error: %d on node %d--Houston, big problem!\n",rc,csr.pid);
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
	PageId	pid=rootPid, eid;
	RC	rc;
	do // walk thru the list.
	{
		// Each loop updates the pid until we get to the leaf node equal to pid
		if (treelist[pid].nodeid == NonleafNode || treelist[pid].nodeid == RootNode)
			rc = treelist[pid].nnode->locateChildPtr(searchKey, pid); // pid is returned
	
	} while((treelist[pid].nodeid == NonleafNode || treelist[pid].nodeid == RootNode) && rc == 0);

	if (rc == 0)
	{
		// now we have reached a leaf node, so get the eid
		//treelist[pid].lnode->printNode();
		rc = treelist[pid].lnode->locate(searchKey, eid); //leaf
		cursor.eid = eid;
		cursor.pid = pid;
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
	RC	rc=0;
	if (treelist[cursor.pid].nodeid == LeafNode)
	{
		treelist[cursor.pid].lnode->readEntry(cursor.eid, key, rid); // rid is returned
		rc = treelist[cursor.pid].lnode->readNextEntry(cursor.eid, cursor.pid);
	}
	else
	{
		fprintf(stdout, "readFoward: Not at leaf node for page %d entry %d\n",
			cursor.pid,cursor.eid);
		rc = RC_END_OF_TREE;
	}
    return rc;
}
void BTreeIndex::print()
{
	for(int i=1; i < treecount; i++)
	{
		if (treelist[i].nodeid == LeafNode)
			treelist[i].lnode->printNode();
		else
			treelist[i].nnode->printNode();
	}
}

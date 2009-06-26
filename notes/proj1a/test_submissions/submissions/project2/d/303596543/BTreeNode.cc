#include "BTreeNode.h"

using namespace std;
BTLeafNode::BTLeafNode()
{
	root = new NodeType;
	root->count =0;
	//root->theroot = 0;
}
/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{
	RC rc = pf.read(pid,root);
	root->nodepid = pid;
	return rc;
}
/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{
/*	if (pid != root->nodepid)
	{
		fprintf(stdout, "Error: Requested pageid %d is not the same as node pageid %d\n",
			pid,root->nodepid);
	}*/
	RC rc=pf.write(pid,root);
	root->nodepid = pid;
	return rc;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ return root->count; }

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
	RC rc=0;
	int idx=0;
	// Assuming count is less than n-1 (root->count == n-1)
	if (root->count == nNodes-1)
		rc=  RC_NODE_FULL;
	else if (root->count == 0)
	{
		root->node[0].key = key;
		root->node[0].rid = rid;
		root->count = 1;
	}
	else
	{
		// search for position in node for new key
		// There are two cases: 1.There is a value that key is <=
		//						2.There is no value <= thus if end of list, then add it to the end.
		for(int i=0; i < root->count; i++)
		{
			if (key <= root->node[i].key){
				idx=i;	// put into this slot
				break;
			}
			else if (i == root->count-1){
				idx = root->count;	// put at end of list
				break;
			}
		} // endfor loop
		// Use idx and up the count and shift the nodes in order to insert new key
		root->count++;
		for(int k=root->count; k > idx; k--)
		{
			// if count=4 and k=0, then 4=3,3=2,2=1,1=0
			root->node[k] = root->node[k-1];
			if ((k-1) == idx)
			{
				root->node[k-1].key = key;
				root->node[k-1].rid = rid;
//				fprintf(stdout, "Leaf:insert[%d]: idx %d key %d\n",root->count,idx,key); 
			}
		} // endfor loop
	} // endif node full check
	//fprintf(stdout, "Leaf:insert[%d]: idx %d key %d\n",root->count,idx,key);
	return rc;
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode *sibling, int& siblingKey)
{
	int x=root->count/2+1;
	// split means copy/insert half of this node into the next sibling
	siblingKey = root->node[x].key;
	//fprintf(stdout, "Leaf:insertSplit: count %d key %d split %d mmidkey %d\n",root->count,key,x,siblingKey);
	for(int i=x; i < root->count; i++)
	{ 
		sibling->insert(root->node[i].key,root->node[i].rid);		
	}
	// change count to the split value
	root->count = x;
	if (key >= siblingKey)
		sibling->insert(key,rid);
	else
		this->insert(key,rid);

	return 0;
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equal to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{
	for(int i=0; i < root->count; i++)
	{
		if (root->node[i].key >= searchKey)
		{
			eid = i;
			return 0;
		}
	} // endfor loop
	return RC_NO_SUCH_RECORD; 
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{ 
	if (eid < root->count)
	{
		key = root->node[eid].key;
		rid = root->node[eid].rid;
		return 0;
	}
	else
	{
		fprintf(stdout, "readEntry Error: Invalid id %d\n",eid);
		return RC_INVALID_CURSOR;
	}
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
	return root->nextNode; 
}
/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	root->nextNode = pid;
	return 0;
}
RC BTLeafNode::readNextEntry(int& eid,PageId& pid)
{
	// increment to next entry
	eid++;
	// if the next entry exceeds count then point to next node
	if (eid >= root->count){
		eid=0;
		pid = getNextNodePtr();
		if (pid == 0) return RC_END_OF_TREE;
	}
	return 0;
}
/*---------------------------------------------------------------------------------*/
BTNonLeafNode::BTNonLeafNode()
{
	root = new NodeType;
	root->count = 0;
	//root->theroot = 0;
	//fprintf(stdout, "nNodes %d NodeType struct %d\n",
	//	nNodes,NodeSize);
}
 /* Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{
	return (pf.read(pid,root));
}
    
/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{
/*	if (pid != root->nodepid)
	{
		fprintf(stdout, "NonleafWrite Error: Requested pageid %d is not same as node pageid %d\n",
			pid,root->nodepid);
	}*/
	root->nodepid = pid;
	RC rc=pf.write(pid,root);
	return rc;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
	return root->count;
}

/* structure of a node where there are n entries of NodeEntry
	  -------------------
	  | count | nodepid |
	  ---------------------
	  | key | rid.pid |...|
	  |     |    .sid |...|
	  ---------------------
	  | nextNode |
	  ------------
	key orientations:
	  -----------------------
	  |i0|k0| i0 = key < k0
	  |i1|k1| i1 = key < k1 
	  |i2|k2| i2 = key < k2
	  |.....|
	  -----------------------

 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
	RC rc=0;
	int idx=0;
	// Assuming count is less than n-1 (root->count == n-1)
	if (root->count == nNodes-1)
		rc=  RC_NODE_FULL;
	else
	{
		// search for position in node for new key
		// There are two cases: 1.There is a value that key is <=
		//						2.There is no value <= thus if end of list, then add it to the end.
		for(int i=0; i < root->count; i++)
		{
			if (key <= root->node[i].key){
				idx=i;	// use this existing slot
				break;
			}
			else if (i == root->count-1){
				idx = root->count;	// put at end of list
				break;
			}
		} // endfor loop
		/* Use idx to shift the nodes in order to insert new key
		count=3, then next node holds pointer pid
		---------------------
		|.|0 |.|1 |.|2 |.|  |
		---------------------
		 move last pointer first
		*/
		// if adding at the end then
		if (idx == root->count)
		{	
			root->node[idx].key = key;
			root->node[idx+1].rid.pid = pid;
			root->count++;
		}
		else
		{
			// root->node[root->count+1] = root->node[root->count];
			root->count++;
			for(int k=root->count; k > idx; k--)
			{
				// if count=3 and idx=0, then 4=3,3=2,2=1,1=0
				root->node[k] = root->node[k-1];
				if ((k-1) == idx)
				{
					root->node[k-1].key = key;
					root->node[k-1].rid.pid = pid;
					//fprintf(stdout, "NonLeaf:insert[%d]: idx %d key %d\n",root->count,idx,key);
				}
			} // endfor loop
		}
	} // endif node full check
	//fprintf(stdout, "NonLeaf:insert[%d]: idx %d key %d\n",root->count,idx,key);
	return rc;
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode *sibling, int& midKey)
{
	int x=root->count/2+1;
	// split means copy/insert half of this node into the next sibling
	midKey = root->node[x].key;
	sibling->initializeRoot(root->node[x].rid.pid,midKey,root->node[x+1].rid.pid);
	//fprintf(stdout, "NonLeaf:insertSplit: count %d key %d split %d midkey %d\n",root->count,key,x,midKey);
	for(int i=x+1; i < root->count; i++)
	{
		sibling->insert(root->node[i].key,root->node[i].rid.pid);	
	}
	// change count to the split value
	root->count = x;
	if (key >= midKey)
		sibling->insert(key,pid);
	else
		this->insert(key,pid);
	return 0;
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{
	for(int i=0; i < root->count; i++)
	{
		if (searchKey < root->node[i].key){
				pid = root->node[i].rid.pid;
				//fprintf(stdout, "Locate1 Node[%d] pid %d\n",i,pid);
				return 0;
		}
		if (root->count == 1 || i == (root->count-1)) // beginning of list
		{
			if (root->node[i].key <= searchKey){
				pid = root->node[i+1].rid.pid;
				//fprintf(stdout, "Locate2 Node[%d] pid %d\n",i,pid);
				return 0;
			} // endif one element in list
		}
		else
		{
			if (root->node[i].key <= searchKey && searchKey < root->node[i+1].key){
				pid = root->node[i+1].rid.pid;
				//fprintf(stdout, "Locate3 Node[%d] pid %d\n",i,pid);
				return 0;
			}
		} // endif head oof list check
	} // endfor loop
	fprintf(stdout, "NonLLocate4 Error: No match for key %d [this node %d]\n",searchKey,
		root->nodepid);
	return RC_NO_SUCH_RECORD; 
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{
	//root->theroot = 1;
	root->node[0].key = key;
	root->node[0].rid.pid = pid1;
	root->node[1].rid.pid = pid2;
	root->count = 1;
	return 0;
}
void BTNonLeafNode::printNode()
{
	fprintf(stdout, "Node %d Count %d\n",root->nodepid,root->count);
	//if (root->theroot) fprintf(stdout, " (root)\n");

	for(int i=0; i < root->count+1; i++)
	{
		fprintf(stdout, " |%d|%d|",root->node[i].key,root->node[i].rid.pid);
		if ((i % 8) == 0) fprintf(stdout, "\n");
	}
	fprintf(stdout, "\n");
}
void BTLeafNode::printNode()
{
	fprintf(stdout, "LNode %d Count %d\n",root->nodepid,root->count);
	for(int i=0; i < root->count; i++)
	{
		fprintf(stdout, " |%d|%d.%d|",root->node[i].key,root->node[i].rid.pid,
			root->node[i].rid.sid);
		if ((i % 8) == 0) fprintf(stdout, "\n");
	}
	fprintf(stdout, "\n|next %d|\n",root->nextNode);
}

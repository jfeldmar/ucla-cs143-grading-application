#include "BTreeNode.h"
#include <math.h>
using namespace std;

/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */


RC BTLeafNode::read(PageId pid, const PageFile& pf)
{
	char buffer[PageFile::PAGE_SIZE];
	RC rc =  pf.read(pid,&buffer);
	leafData* lp = (leafData*)buffer;
	lData = * lp;
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
	return pf.write(pid,&lData);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ return lData.numOfEntries; }

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
	if (lData.numOfEntries >= N-1)
		return RC_NODE_FULL;
	
	leafpair a;
	a.key = key;
	a.rid = rid;
	lData.entryPair[lData.numOfEntries] = a;
	lData.numOfEntries++;

	//insertion sort
	int i;
	for(int j=1;j<lData.numOfEntries;j++)
	{
		a=lData.entryPair[j];
		i=j-1;
		while(lData.entryPair[i].key>a.key && i>=0)
		{
			lData.entryPair[i+1]=lData.entryPair[i];
			i--;
		}
		lData.entryPair[i+1]=a;
	}
	return 0;
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
                              BTLeafNode& sibling, int& siblingKey)
{ 
	int eid;
	int rKey;
	RecordId rRid;
	
	for(int i = 0; i < lData.numOfEntries/2; i++)
		sibling.insert(lData.entryPair[lData.numOfEntries-i-1].key,lData.entryPair[lData.numOfEntries-i-1].rid);

	lData.numOfEntries = (int)ceil(double(lData.numOfEntries/2));

	sibling.insert(key,rid);

	sibling.readEntry(0,rKey,rRid);
	siblingKey = rKey;
	
	return 0; 
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
	for(int i = 0; i < lData.numOfEntries; i ++)
	{
		if(lData.entryPair[i].key >= searchKey)
		{
			eid = i;
			return 0;
		}
	}

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
	if(eid >= lData.numOfEntries || eid <0)
		return RC_NO_SUCH_RECORD;

	key = lData.entryPair[eid].key;
	rid = lData.entryPair[eid].rid; 

	return 0; 
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	return lData.nextPID;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	lData.nextPID = pid;
	return 0; 
}

/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ 	
	char buffer[PageFile::PAGE_SIZE];
	RC rc =  pf.read(pid,&buffer);
	nLeafData* lp = (nLeafData*)buffer;
	nlData = * lp;
	return rc;
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
	return pf.write(pid,&nlData); 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
	return nlData.numOfEntries;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 
	if (nlData.numOfEntries >= N)
		return RC_NODE_FULL;
	
	nleafpair a;
	a.key = key;
	a.pid = pid;
	nlData.entryPair[nlData.numOfEntries] = a;
	nlData.numOfEntries++;

	//insertion sort
	int i;
	for(int j=1;j<nlData.numOfEntries;j++)
	{
		a=nlData.entryPair[j];
		i=j-1;
		while(nlData.entryPair[i].key>a.key && i>=0)
		{
			nlData.entryPair[i+1]=nlData.entryPair[i];
			i--;
		}
		nlData.entryPair[i+1]=a;
	}
	return 0;
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
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ 
	
	for(int i = 0; i < nlData.numOfEntries/2; i++)
		sibling.insert(nlData.entryPair[nlData.numOfEntries-i-1].key,nlData.entryPair[nlData.numOfEntries-i-1].pid);
	nlData.numOfEntries = (int)ceil(double(nlData.numOfEntries/2));
	
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
	for(int i = 0; i < nlData.numOfEntries; i ++)
	{
		if(nlData.entryPair[i].key > searchKey)
		{
			pid = nlData.entryPair[i].pid;
			return 0;
		}
	}

	pid = nlData.nextPID;
	return 0;

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
	nlData.numOfEntries++;
	nlData.entryPair[0].pid = pid1;
	nlData.entryPair[0].key = key;
	nlData.nextPID = pid2;

	return 0;
}

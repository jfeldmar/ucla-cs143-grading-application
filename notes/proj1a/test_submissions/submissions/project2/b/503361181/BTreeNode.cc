#include "BTreeNode.h"

using namespace std;

BTLeafNode::BTLeafNode()
{
	leaf = new int[256];
	for(int i=0; i<256; i++)
	{
		leaf[i] = -1;
	}
}

BTLeafNode::~BTLeafNode()
{
	delete [] leaf;

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
	
	int r = pf.read(pid, &leaf);

	return r; 

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
	
	int r = pf.write(pid, &leaf);

	return r;

}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 

	int count = 0;
	
	for(int i=0; i<N; i++)
	{
		if(leaf[i*3] != -1)
			count++;
	}

	return count;	
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
	
	if(getKeyCount()==N)
		return RC_NODE_FULL;
	else
	{
		int* temp = new int[256];
		int i=0;
		//copy the first few elements.
		for(; leaf[i*3+2] < key && leaf[i*3] != -1; i++)
		{
				temp[i*3] = leaf[i*3];
				temp[i*3+1] = leaf[i*3+1];
				temp[i*3+2] = leaf[i*3+2];
		}
			
		//insert the new key
			temp[i*3] = rid.pid;
			temp[i*3+1] = rid.sid;
			temp[i*3+2] = key;
		//copy the rest of the elements
		for(; i<N; i++)
		{
			temp[(i+1)*3] = leaf[i*3];
			temp[(i+1)*3+1] = leaf[i*3+1];
			temp[(i+1)*3+2] = leaf[i*3+2];
		}
		//copy the last pid
		temp[i*3] = leaf[i*3];
		
		delete [] leaf;
		leaf = temp;
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

	int c = getKeyCount();
	int split = (c + 1)/2;
	
	siblingKey = leaf[split*3+2];
	
	//copy keys.
	for(int i=split, j = 0; i<N; i++, j++)
	{
		sibling.leaf[j*3] = leaf[i*3];
		sibling.leaf[j*3+1] = leaf[i*3+1];
		sibling.leaf[j*3+2] = leaf[i*3+2];
		
		leaf[i]=-1;
		leaf[i+1]=-1;
		leaf[i+2]=-1;
		
	}
	
	if(key < siblingKey)
	{
		this->insert(key, rid);
	}
	else
		sibling.insert(key, rid);

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
	int i = 0;
	for(; i<N; i++)
	{
		if(leaf[i*3+2] >= searchKey && leaf[i*3] != -1)
		{
			eid = i;
		}
	}
	if(i==N)
		return RC_NO_SUCH_RECORD;

	return 0; 

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

	if(leaf[eid*3] == -1)
		return RC_NO_SUCH_RECORD;
		
	rid.pid = leaf[eid*3];
	rid.sid = leaf[eid*3+1];
	key = leaf[eid*3+2];

	return 0; 
	
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 

	if(leaf[N*3] == -1)
		return RC_NO_SUCH_RECORD;
	else return leaf[N*3]; 

}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	leaf[N*3] = pid;

	return 0; 

}

BTNonLeafNode::BTNonLeafNode()
{
	nonleaf = new int[256];
	for(int i=0; i<256; i++)
	{
		nonleaf[i] = -1;
	}

}

BTNonLeafNode::~BTNonLeafNode()
{
	delete [] nonleaf;
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
	
	int r = pf.read(pid, &nonleaf);

	return r; 

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

	int r = pf.write(pid, &nonleaf);

	return r;

}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
	int count = 0;
	for(int i=0; i<N; i++)
	{
		if(nonleaf[i*2] != -1)
			count++;
	}
	return count; 
	
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 

	if(getKeyCount()==N)
		return RC_NODE_FULL;
	else
	{
		int* temp = new int[256];
		int i = 0;
		for(; nonleaf[i*2+1] < key && nonleaf[i*2] != -1; i++)
		{
				temp[i*2] = nonleaf[i*2];
				temp[i*2+1] = nonleaf[i*2+1];
		}
			
		//insert the new key
			temp[i*2] = pid;
			temp[i*2+1] = key;
		//copy the rest of the elements
		for(; i<N; i++)
		{
			temp[(i+1)*2] = nonleaf[i*2];
			temp[(i+1)*2+1] = nonleaf[i*2+1];
		}
		
		delete [] nonleaf;
		nonleaf = temp;

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

	int c = getKeyCount();
	int split = (c + 1)/2;
	
	midKey = nonleaf[split*2+1];
	
	//copy keys.
	for(int i=split, j = 0; i<N; i++, j++)
	{
		sibling.nonleaf[j*2] = nonleaf[i*2];
		sibling.nonleaf[j*2+1] = nonleaf[i*2+1];
		
		nonleaf[i]=-1;
		nonleaf[i+1]=-1;
		
	}
	
	if(key < midKey)
	{
		this->insert(key, pid);
	}
	else
		sibling.insert(key, pid);

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
	int i = 0;
	for(; i<N; i++)
	{
		if(nonleaf[i*2+1] >= searchKey && nonleaf[i*2] != -1)
			pid = nonleaf[i*2];
	}
	if(i == N)
		return RC_NO_SUCH_RECORD;

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

	nonleaf[0] = pid1;
	nonleaf[1] = key;
	nonleaf[2] = pid2;

	return 0;

}

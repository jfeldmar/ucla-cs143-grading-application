#include "BTreeNode.h"
#include <iostream>

using namespace std;

BTLeafNode::BTLeafNode()
{
	for(int i=0; i<256; i++)
	{
		leaf[i] = -1;
	}
}

BTLeafNode::~BTLeafNode()
{
	

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
		int temp[256];
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
		for(; i<N-1; i++)
		{
			temp[(i+1)*3] = leaf[i*3];
			temp[(i+1)*3+1] = leaf[i*3+1];
			temp[(i+1)*3+2] = leaf[i*3+2];
		}
		//copy the last pid
		temp[255] = leaf[255];
		
		for(i=0; i<256; i++){leaf[i] = temp[i];}

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
	RecordId srid;
	int c = getKeyCount();
	int split = (c + 1)/2;
	
	siblingKey = leaf[split*3+2];
	
	//copy keys.
	for(int i=split, j = 0; i<N; i++, j++)
	{
		srid.pid = leaf[i*3];
		srid.sid = leaf[i*3+1];
		sibling.insert(leaf[i*3+2], srid);
		
		leaf[i*3]=-1;
		leaf[i*3+1]=-1;
		leaf[i*3+2]=-1;
		
	}
	sibling.setNextNodePtr(leaf[255]);
	
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
			break;
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

	if(eid > N || leaf[eid*3] == -1)
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
	//nonleaf = new int[256];
	for(int i=0; i<256; i++)
	{
		nonleaf[i] = -1;
	}

}

BTNonLeafNode::~BTNonLeafNode()
{
	
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
	return nonleaf[255];	
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
	int i = getKeyCount();
	if(i==N)
		return RC_NODE_FULL;
	else
	{
		if(key > nonleaf[i*2-1])
		{
			nonleaf[(i+1)*2] = pid;
			nonleaf[(i+1)*2-1] = key;
		}
		else
		{
			for(int i=getKeyCount(); i>0; i--)
			{
				nonleaf[(i+1)*2] = nonleaf[i*2];//copy pid
				nonleaf[(i+1)*2-1] = nonleaf[i*2-1];//copy key
				if(key < nonleaf[i*2-1])
				{
					nonleaf[i*2-1] = key;
					nonleaf[i*2] = pid;
					break;
				}
			}
		}
		nonleaf[255]++;
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
	
	midKey = nonleaf[split*2-1];
	sibling.nonleaf[0] = nonleaf[split*2];
	//copy keys.
	int j = 1;
	for(int i=split; i<=N; i++, j++)
	{
		sibling.nonleaf[j*2] = nonleaf[i*2];
		sibling.nonleaf[j*2-1] = nonleaf[i*2-1];
		
		nonleaf[i*2]=-1;
		nonleaf[i*2-1]=-1;
		
	}
	sibling.nonleaf[255] = j;
	nonleaf[255] = split;
	for(;j<=N; j++) { sibling.nonleaf[j*2-1] = -1; sibling.nonleaf[j*2] = -1; }
	
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

	if(searchKey < nonleaf[1]){ pid = nonleaf[0]; }
	int i=getKeyCount();
	for(; i>0; i--)
	{
		if(searchKey >= nonleaf[i*2-1] && nonleaf[i*2] != -1)
			{pid = nonleaf[i*2]; break;}
			
	}
	if(i == 0)
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
	nonleaf[255] = 1;

	return 0;

}
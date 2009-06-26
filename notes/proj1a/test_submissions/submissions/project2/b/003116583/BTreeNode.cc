#include "BTreeNode.h"

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
	int read = rf.read(pid, &buffer);
	return(read);
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
	int write = rf.write(pid, &buffer);
	return(write); 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
	int count = buffer[0];
	return(count); 
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
	int temp[256];
	int amount = buffer[0];
	if(amount == 20)
	{
		return (-1010);
	}
	else
	{
		int x = 3;
		int y = 0;
		int find = 0;
		while(find == 0)
		{
			if(buffer[x] < key)
			{
				temp[x] = buffer[x];
				temp[x-1] = buffer[x-1];
				temp[x-2] = buffer[x-2];
				x+=3;
				y+=3;
			}
			else
			{
				temp[x-1] = rid.pid;
				temp[x-2] =rid.sid;
				temp[x] = key;
				y+=3;
				find = 1;
			}
		}
		amount *=3;
		amount += 1;
		while(x < amount)
		{
			temp[y] = buffer[x];
			temp[y-1] = buffer[x-1];
			temp[y-2] = buffer[x-2];
			x+=3;
			y+=3;
		}
		temp[amount+3] = buffer[amount];

		int change = buffer[0];
		change += 1;
		temp[0] = change;
		change *= 3;
		change += 1;
		while(x <= change)
		{
			buffer[x] = temp[x];
		}
	
		return 0; 
	}
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

	int x = 3;
	int max, max2;
	int count = 1;
	max = buffer[0];
	int found = 0;
	int sib = 0;
	while(count <= max)
	{
		if(key == buffer[x])
		{
			found = count;
			sibling[0] = (max-count+1);
			max2 = sibling[0];
			while(count<=max)
			{
				sibling[sib] = buffer[x-2];
				sibling[sib+1] = buffer[x-1];
				sibling[sib+2] = buffer[x];
				x+=3;
				sib+=3;
				count++;
			}	
			sibling[sib-2] = buffer[x-2];
		}			
		x += 3;
		count++;
	}
	buffer[0] = found-1;
	int ind = (found * 3);
	while(found <= max)
	{
		buffer[ind] = NULL;
		buffer[ind-1] = NULL;
		buffer[ind-2] = NULL;
		ind += 3;
		found++;
	}
	int last = (found * 3) - 2;
	buffer[last] = &sibling;

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
	int x = 3;
	int max;
	int count = 0;
	max = buffer[0];
	while(count <= max)
	{
		if(searchKey <= buffer[x])
		{
			eid = count;
			return(0);	
		}
		x += 3;
		count++;
	}
	return(-1012);
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
	int count= 0;
	count = eid;
	count *= 3;
	count -=2;
	rid.pid = buffer[count];
	rid.sid = buffer[count+1];
	key = buffer[count+2];

	return 0; 
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
	int last, p1d;
	last = buffer[0];
	last *= 3;
	last += 1;
	p1d = buffer[last];
	return(p1d); 
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	int last;
	last = buffer[0];
	last *= 3;
	last += 1;
	buffer[last] = pid;
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
	int read = rf.read(pid, &buffer); 
	return(read); 
}
    
/*
 * Write the content of the node in buffer 
 * to the page pid in the PageFile pf.
 * @param pid[IN] the  PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{
	int write = rf.write(pid, &buffer);  
	return(write); 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
	int count = buffer[0];
	return(count); 

}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 
	int temp[256];
	int amount = buffer[0];
	if(amount == 31)
	{
		return (-1010);
	}
	else
	{
		int x = 2;
		int y = 0;
		int find = 0;
		while(find == 0)
		{
			if(buffer[x] < key)
			{
				temp[x] = buffer[x];
				temp[x-1] = buffer[x-1];
				x+=2;
				y+=2;
			}
			else
			{
				temp[x-1] = pid;
				temp[x] = key;
				y+=2;
				find = 1;
			}
		}
		amount *=2;
		amount += 1;
		while(x < amount)
		{
			temp[y] = buffer[x];
			temp[y-1] = buffer[x-1];
			x+=2;
			y+=2;
		}
		temp[amount+2] = buffer[amount];

		int change = buffer[0];
		change += 1;
		temp[0] = change;
		change *= 2;
		change += 1;
		while(x <= change)
		{
			buffer[x] = temp[x];
		}
	
		return 0; 
	}
 
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
{ return 0; }

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{
	int x = 2;
	int count = 1;
	int max;
	max = buffer[0];
	while(count <= max)
	{
		if(searchKey <= buffer[x])
		{
			pid = buffer[x-1];
			return(0);	
		}
		x += 2;
		count++;
	}
	return(-1012);
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
	buffer[0] = 1;
	buffer[1] = pid1;
	buffer[2] = key;
	buffer[3] = pid2; 
	return 0; 
}

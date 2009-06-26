#include "BTreeNode.h"
using namespace std;

char * buffer;
char * nonbuffer;
const int nValue = 100;
int currenteid = 1;
int noncurrenteid = 1;

struct Entry{
	int eid;
	int key;
	RecordId& rid;

	Entry():rid(rid)
	{
		eid = 0;
		key = 0;
	}
	
	void add(int neweid,int newkey, RecordId& newrid)
	{
		eid = neweid;
		key = newkey;
		rid = newrid;
	}
};

Entry entry[100];
Entry nonentry[100];


/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ 
	buffer = (char*) malloc (1024);
	char *iterator;

	if(pf.read(pid,buffer) == 0)
	{
		for(int i = 0; i < 100; i++)
		{
			entry[i].eid = i;
			
			memcpy(iterator,buffer,4);
			entry[i].key = (int) iterator;

			memcpy(iterator,buffer,4);
			entry[i].rid.pid = (int) iterator;
			
			memcpy(iterator,buffer,4);
			entry[i].rid.sid = (int) iterator;

			currenteid++;

		}
		return 0;

	}
	else
		return -1;

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
	char * iterator;
	iterator = (char*) malloc (4);

		for(int i = 0; i < 100; i++)
		{			
			iterator = (char *) entry[i].key;
			memcpy(buffer,iterator,4);

			iterator = (char *) entry[i].rid.pid;
			memcpy(buffer,iterator,4);
			
			iterator = (char *) entry[i].rid.sid;
			memcpy(buffer,iterator,4);

		}
	if(pf.write(pid, buffer) == 0)
	{
		return 0;
	}
	else
		return -1;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
	int count;

	for(int i = 0; i <= currenteid; i++)
	{
		if(entry[i].key)
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
RC BTLeafNode::insert(int &key, RecordId& rid)
{ 
	int * eid;
	int& lookupeid(lookupeid);
	int& newkey(newkey);
	RecordId& newrid(newrid);
	
	if(locate(key,lookupeid)==0)
	{
		if(readEntry(lookupeid,newkey,newrid) == 0)
			;//do nothing
		else{
			printf("Problem reading key");
			return -1;
		}
	}
	else{
		printf("Problem locating key");
	}

	if(currenteid < (nValue-1))
	{
		if(key < newkey)
		{
			for(int i = 0; i <= currenteid; i++)
			{
				if(entry[i].key && entry[i].eid > lookupeid)
				{
					entry[i].eid++;
				}
				else break;
			}

			entry[currenteid].add(currenteid+1,key,rid);
			currenteid++;
			return 0;
		}
	}
	else if(key == newkey)
	{
		entry[currenteid].add((currenteid+1),key,rid);
		currenteid++;
		for(int i = 0; i < currenteid; i++)
		{
			if(entry[i].key && entry[i].eid > (lookupeid+1))
			{
				entry[i].eid++;
			}
			else break;
		}
		return 0;
	}
	else
	{
		return -1;
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
RC BTLeafNode::insertAndSplit(int key, RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{ 


	if(insert(key,rid) == 0)
	{
		for(int m = 0; m < (nValue-1); m++)
		{
			sibling.insert((entry[(currenteid/2) + m].key), entry[(currenteid/2) + m].rid);
			if(m == 0)
				siblingKey = (entry[(currenteid/2) + m].key);
		}
		setNextNodePtr(rid.pid+1);
		return 0;
	}
	else
	{
		return -1;
	}

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
	for(int i = 0; i < currenteid; i++)
	{
		if(entry[i].key >= searchKey)
		{
			eid = entry[i].eid;
			return 0;
		}
	}
	return -1;



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
	for(int i = 0; i <= currenteid; i++)
	{
		if(entry[i].eid == eid)
		{
			key = entry[i].key;
			rid = entry[i].rid;
			return 0;
		}
	}
	return -1; 

}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 	
	PageId newpage = entry[nValue].rid.pid;
	
	if(entry[nValue].rid.pid)
	{
		return newpage;
	}
	else
		return -1;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	entry[nValue].rid.pid = pid;
	
	if(entry[nValue].rid.pid)
		return 0;
	else
		return -1;

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
	nonbuffer = (char*) malloc (1024);
	char *iterator;

	if(pf.read(pid,nonbuffer) == 0)
	{
		for(int i = 0; i < 100; i++)
		{
			nonentry[i].eid = i;

			memcpy(iterator,nonbuffer,4);
			nonentry[i].key = (int) iterator;

			memcpy(iterator,nonbuffer,4);
			nonentry[i].rid.pid = (int) iterator;
			nonentry[i].rid.sid = -1;

			noncurrenteid++;

			if(i == 1)
			{
				initializeRoot(nonentry[i-1].rid.pid, nonentry[i-1].key, nonentry[i].rid.pid);
			}
		}
		return 0;

	}
	else
		return -1;

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
	char * iterator;
	iterator = (char*) malloc (4);

	for(int i = 0; i < 100; i++)
	{			
		iterator = (char *) nonentry[i].key;
		memcpy(nonbuffer,iterator,4);

		iterator = (char *) nonentry[i].rid.pid;
		memcpy(nonbuffer,iterator,4);

	}	
	if(pf.write(pid, nonbuffer) == 0)
	{	
		return 0;
	}
	else
		return -1;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	int count;

	for(int i = 0; i <= noncurrenteid; i++)
	{
		if(nonentry[i].key)
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
	int eid;
	int& lookupeid(lookupeid);
	int& newkey(newkey);
	RecordId& newrid(newrid);
	
	for(int i = 0; i < noncurrenteid; i++)
	{
		if(nonentry[i].key >= key)
		{
			eid = nonentry[i].eid;
			newkey = nonentry[i].key;
			break;
		}
	}

	if(noncurrenteid < (nValue-1))
	{
		if(key < newkey)
		{
			for(int i = 0; i <= noncurrenteid; i++)
			{
				if(nonentry[i].key && nonentry[i].eid > eid)
				{
					nonentry[i].eid++;
				}
				else break;
			}
			newrid.pid = pid;
			newrid.sid = -1;

			nonentry[noncurrenteid].add(eid,key,newrid);
			noncurrenteid++;
			return 0;
		}
	}
	else if(key == newkey)
	{
		newrid.pid = pid;
		newrid.sid = -1;

		nonentry[noncurrenteid].add((eid+1),key,newrid);
		noncurrenteid++;
		for(int i = 0; i < noncurrenteid; i++)
		{
			if(nonentry[i].key && nonentry[i].eid > (eid+1))
			{
				nonentry[i].eid++;
			}
			else break;
		}
		return 0;
	}
	else
	{
		return -1;
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
{ 
	RecordId& newrid(newrid);
	newrid.pid = pid;
	newrid.sid = 0;
	int count;

	if(insert(key,pid) == 0)
	{
		for(int m = 0; m < (nValue-1); m++)
		{
			if(sibling.insert((nonentry[(noncurrenteid/2) + m].key), nonentry[(noncurrenteid/2) + m].rid.pid)==0)
				count++;
			if(m == (currenteid/2))
				midKey = (nonentry[(noncurrenteid/2) + m].key);
			
		}
		return 0;

	}
	else
	{
		return -1;
	}

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
	for(int i = 0; i < nValue; i++)
	{
		if(nonentry[i].key == searchKey)
		{
			pid = nonentry[i].rid.pid;
			return 0; 
			break;
		}
	}

	return -1;

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
	RecordId& newrid(newrid);
	RecordId& newrid2(newrid);

	newrid.pid = pid1;
	newrid.sid = 0;

	newrid2.pid = pid2;
	newrid2.sid = 0;

	entry[0].rid = newrid;
	entry[0].key = key;
	entry[1].rid = newrid2;

	if(entry[0].rid.pid)
	{
		if(entry[0].key)
		{
			if(entry[1].rid.pid)
			{
				return 0;
			}
		}
	}
	else
		return -1;

}

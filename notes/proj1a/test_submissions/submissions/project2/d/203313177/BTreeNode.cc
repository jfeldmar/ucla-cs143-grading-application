#include "BTreeNode.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace std;

const int ERROR = -1;
/*
 * Read the content of the node into buffer 
 * from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{
	char buffer[1024];
	char temp[4];
	leafPair pair;
	
	if (pf.read(pid, buffer))
		return ERROR;
	
	for (int i = 0; i < 1020/12; i++)
	{
		memcpy(temp, &(buffer[i*12]), 4);
		pair.key = atoi(temp);
		
		if (pair.key == -2)
			break;
		
		memcpy(temp, &(buffer[(i*12)+4]), 4);
		pair.rid.pid = atoi(temp);
		
		memcpy(temp, &(buffer[(i*12)+8]), 4);
		pair.rid.sid = atoi(temp);
		
		records.push_back (pair);
	}
	
	memcpy(temp, &buffer[1020], 4);
	toNextLeaf = atoi(temp);
	return 0; 
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
	char buffer[1024];
	char temp[5];
	
	for (int i = 0; i < 1024/4; i++)
	{
		sprintf(temp, "%d", -2);
		memcpy(&(buffer[i*4]), temp, 4);
	}
	
	for (int i = 0; i < records.size(); i++)
	{
		sprintf(temp, "%d", records[i].key);
		memcpy(&(buffer[i*12]), temp, 4);
		
		sprintf(temp, "%d", records[i].rid.pid);
		memcpy(&(buffer[(i*12)+4]), temp, 4);
		
		sprintf(temp, "%d", records[i].rid.sid);
		memcpy(&(buffer[(i*12)+8]), temp, 4);
		
	}
	if (toNextLeaf >= 0)
	{
		sprintf(temp, "%d", toNextLeaf);
		memcpy(&(buffer[1020]), temp, 4);
	}
	
	return (pf.write(pid, buffer));
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ return records.size(); }

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
	if (records.size() >= (1024 - 4)/12)
		return ERROR;
	leafPair pair;
	pair.key = key;
	pair.rid = rid;
	vector<leafPair>::iterator i = records.begin();
	while (i != records.end() && i->key < key)
		i++;
	records.insert(i, pair);
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
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, BTLeafNode& sibling, int& siblingKey)
{ 
	if (sibling.getKeyCount() != 0)
		return ERROR;
	
	if (getKeyCount()%2 == 0)
	{
		for (int i = getKeyCount()/2; i < getKeyCount(); i++)
			sibling.insert(records[i].key, records[i].rid);
		records.resize(getKeyCount()/2);
		
		if (key >= sibling.records[0].key)
			sibling.insert(key, rid);
		else
			insert(key, rid);
	}
	
	else
	{
		if (key < records[getKeyCount()/2].key)
		{
			for(int i = getKeyCount()/2; i < getKeyCount(); i++)
				sibling.insert(records[i].key, records[i].rid);
			records.resize(getKeyCount()/2);
			
			insert(key, rid);
		}
		
		else
		{
			for (int i = (getKeyCount()/2) + 1; i < getKeyCount(); i++)
				sibling.insert(records[i].key, records[i].rid);
			records.resize((getKeyCount()/2) + 1);
			
			sibling.insert(key, rid);
		}
	}
	siblingKey = sibling.records[0].key;
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
	for (int i = 0; i < records.size(); i++)
	{
		if (records[i].key >= searchKey)
		{
			eid = i;
			return 0;
		}
	}
	return ERROR;
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
	if (eid < 0 || eid >= records.size())
		return ERROR;
	key = records[eid].key;
	rid = records[eid].rid;
	return 0;
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
	if(toNextLeaf == -1)
		return ERROR;
	else
		return toNextLeaf; 
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	toNextLeaf = pid;
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
	char buffer[1024];
	char temp[4];
	nonLeafPair pair;
	
	if (pf.read(pid, buffer))
		return ERROR;
	
	memcpy(temp, &(buffer[0]), 4);
	lPid = atoi(temp);
	
	for (int i = 0; i < 1020/8; i++)
	{
		memcpy(temp, &(buffer[(i*8)+4]), 4);
		pair.key = atoi(temp);
		
		memcpy(temp, &(buffer[(i*8)+8]), 4);
		pair.pid = atoi(temp);
		
		if (pair.key == -2)
			break;
		
		records.push_back (pair);
	}
	return 0;
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
	char buffer[1024];
	char temp[5];
	
	for (int i = 0; i < 1024/4; i++)
	{
		sprintf(temp, "%d", -2);
		memcpy(&(buffer[i*4]), temp, 4);
	}
	
	sprintf(temp, "%d", lPid);
	memcpy(&(buffer[0]), temp, 4);
		
	for (int i = 0; i < records.size(); i++)
	{
		sprintf(temp, "%d", records[i].key);
		memcpy(&(buffer[(i*8)+4]), temp, 4);
	
		sprintf(temp, "%d", records[i].pid);
		memcpy(&(buffer[(i*8)+8]), temp, 4);
	}
	
	return (pf.write(pid, buffer));
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ return records.size(); }


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{	
	/* #FIX ME# */
	if (records.size() >= (1020)/8)
		return ERROR;
	nonLeafPair pair;
	pair.key = key;
	pair.pid = pid;
	vector<nonLeafPair>::iterator i = records.begin();
	while (i != records.end() && i->key < key)
		i++;
	records.insert(i, pair);
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
	if (sibling.getKeyCount() != 0)
		return ERROR;
	
	if (getKeyCount()%2 == 0)
	{
		for (int i = getKeyCount()/2; i < getKeyCount(); i++)
			sibling.insert(records[i].key, records[i].pid);
		records.resize(getKeyCount()/2);
		
		if (key >= sibling.records[0].key)
			sibling.insert(key, pid);
		else
			insert(key, pid);
	}
	
	else
	{
		if (key < records[getKeyCount()/2].key)
		{
			for(int i = getKeyCount()/2; i < getKeyCount(); i++)
				sibling.insert(records[i].key, records[i].pid);
			records.resize(getKeyCount()/2);
			
			insert(key, pid);
		}
		
		else
		{
			for (int i = (getKeyCount()/2) + 1; i < getKeyCount(); i++)
				sibling.insert(records[i].key, records[i].pid);
			records.resize((getKeyCount()/2) + 1);
			
			sibling.insert(key, pid);
		}
	}
	midKey = sibling.records[0].key;
	sibling.lPid = sibling.records[0].pid;
	sibling.records.erase(sibling.records.begin());
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
	if (records.empty())
		return ERROR;
	if (searchKey < records[0].key){
		pid = lPid;		
		return 0; }
	for (int i = 1; i < records.size(); i++)
	{
		if (records[i].key > searchKey)
		{
			pid = records[i-1].pid;
			return 0;
		}
	}
	pid = records[records.size() - 1].pid;
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
	lPid = pid1;
	insert(key, pid2);
	return 0;
}

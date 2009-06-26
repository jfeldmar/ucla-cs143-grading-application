#include "BTreeNode.h"


//BTNonLeafNode::count=0;
//BTNonLeafNode::page = PageFile();

BTLeafNode::BTLeafNode(){
	Pid = -1;
	count = 0;
	//page = PageFile();
	int* correct = (int*)buffer;
	correct[0]=0;
	correct[1]=-1;
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

	/*
	RecordFile record = RecordFile();
	RecordId rid;
	rid.pid=pid;
	int key;
	string value;
	int i=0;
	while(i<record.EndRid())
	{
		record.read(rid, key, value);
		nodes[i].value=*value;
		nodes[i].key=key;
	}
	count=i;
	*/
	page = pf;
	RC rc = pf.read(pid,buffer); 
	int* temp = (int*)buffer;
	count = temp[0];
	Pid = pid;
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
	int* temp = (int*)buffer;
	temp[0]=count;
	RC rc = pf.write(pid,buffer);
	return rc;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ return count; }

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 	
	read(Pid,page);
	fprintf(stdout,"Adding key %d\n",key);
	SubNodeLeaf* leaf;
	int* nextP;
	int i=8;
	leaf = (SubNodeLeaf*)(buffer+i);
	i=0;
	nextP = (int*) buffer;
	fprintf(stdout,"Count = %d\n",count);
	fprintf(stdout,"Next Pid = %d\n",nextP[1]);
	while(i<count && leaf[i].key < key){
		i++;
		if(i==count &&  nextP[1] > 0){
			nextP = (int*) buffer;
			PageId pid = nextP[1];
			read(pid,page);
			i=0;
			count = nextP[0];
			fprintf(stdout,"Should not be here \n");
		}
	}
	int j = count;

	SubNodeLeaf* temp =(SubNodeLeaf*) (buffer + 8);
	while(j>i){
		temp[j] = temp[j-1];
		j--;
	}
	leaf[i].key = key;
	leaf[i].value = rid;
	count++;

/*
	SubNodeLeaf ins;
	ins.key=key;
	ins.value=rid;
	nodes.insert(ins);
	*/
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
	/*if(sibling.count != 0)
		return RC_INVALID_ATTRIBUTE;
	//sibling.nodes.insert(key,rid);
	return 0;
	*/
	int counter= count-1;
	count/=2;
	int* changeOrig = (int*) buffer;
	SubNodeLeaf* filler = (SubNodeLeaf*) (buffer+8);
	changeOrig[0] = count;
	while(counter>count)
	{
		sibling.insert(filler[counter].key,filler[counter].value);
		counter--;
	}
	int* changeSib = (int*)sibling.buffer;
	int* correct = (int*)buffer;
	changeSib[0] = count;
	int in = correct[1];
	changeSib[1] = in;
	correct[1]=page.endPid();
	fprintf(stdout,"nextPid = %d\n",correct[1]);
	sibling.count = count;
	sibling.Pid = page.endPid();
	sibling.write(page.endPid(), page);
	fprintf(stdout,"nextPid after write = %d\n",correct[1]);
	*((int*)buffer+1)=correct[1];
	write(Pid,page);
	if(key>=changeSib[2]){
		sibling.insert(key,rid);
		fprintf(stdout,"nextPid after insert in second group= %d\n",correct[1]);
	}
	else
		insert(key,rid);
	fprintf(stdout,"nextPid after insert in first group = %d\n",correct[1]);
	SubNodeLeaf* findFirst = (SubNodeLeaf*) (buffer+8);
	siblingKey = findFirst[0].key;
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
	int i=0;
	int total = 0;
	//while(searchKey<nodes[i].key&&i<nodes.size())

	SubNodeLeaf* locate = (SubNodeLeaf*)(buffer + 8);
	int* next;
	while(i<count && locate[i].key<searchKey)
	{
		next = (int*) buffer;
		i++;
		total++;
		if(i == count && next[1] > 0)
		{
			read(next[1], page);
			i = 0;
		}
	}
	eid=total;
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
{ 	return 0;
}

/*
 * Output the pid of the next slibling node.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	int* buff = (int*) buffer;
	return buff[1];
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	int* buff = (int*) buffer;
	buff[1] = pid;
	return 0;
}

BTNonLeafNode::BTNonLeafNode()
{
	count = 0;

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
	page=pf;
	return pf.read(pid,buffer);
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
	count++;
	return pf.write(pid,buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ return count; }


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 	
	/*
	SubNodeNonLeaf ins;
	ins.key=key;
	ins.value=pid;
	*/
	//nodes.insert(ins);
	read(pid,page);
	SubNodeNonLeaf* insertion = (SubNodeNonLeaf*)buffer+8;
	BTLeafNode temp;
	int i = 0;
	while(i<count && insertion[i].key < key)
	{	i++;}
	temp.read(pid,page);
	SubNodeLeaf* first = (SubNodeLeaf*)temp.buffer+8;
	int j = count;
	while(j>i)
		insertion[j]=insertion[j-1];
	count++;
	if(first[i].key<key)
		insertion[i].value1=pid;
	else
		insertion[i].value2=pid;
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
	//sibling.nodes.insert(key,pid);
	midKey = key;
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
	int i=0;
	//while(searchKey<=nodes[i].key&&i<nodes.size())
	//	i++;
	//if(i==nodes.size())
	//	return RC_END_OF_TREE;
	//pid = nodes[i].value;
	//int tempHeight = treeHeight;
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
{ /*
	RC rc = 0;
	rc = insert(pid1,key);
	if(rc<0)
		return rc;
	rc = insert(pid2,key);
	return rc;
	*/
	insert(key,pid1);
	insert(key,pid1);
	read(page.endPid(),page);
	int* fix = (int*) buffer;
	fix[0] = page.endPid();
	fix[1] = 2;

	return 0;
}

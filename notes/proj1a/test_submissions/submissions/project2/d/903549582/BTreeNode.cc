#include "BTreeNode.h"

using namespace std;

/*
 *BTLeafNode constructor
 *initialize member variables
 *added
*/
BTLeafNode::BTLeafNode(){
	
	memset(keys, -1, sizeof(int) * MAX_KEYS);
	memset(records, -1, sizeof(RecordId) * MAX_POINTERS);
	pageIndex = MAX_POINTERS - 1;
	//tmp = (void*) malloc( sizeof(int) * MAX_KEYS + sizeof(RecordId) * MAX_POINTERS);
	tmp = (void*) malloc(1024);
}

void
BTLeafNode::printNodeInfo(){
if(SH_DEBUG){

	keyCount = getKeyCount();
	int i;
	
	for(i=0; i<keyCount; i++){
		printf("|%d|", keys[i]);
	}
	printf("\n");
	for(i=0; i<keyCount; i++){
		printf("|%d,%d|", records[i].pid, records[i].sid);
	}
	printf("\n");	
	printf("Last:%d\n", records[pageIndex].pid);
}
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
	//memset(tmp, -1, 1024);
	
	//int result;
	
	//int k[MAX_KEYS];
	//RecordId rd[MAX_POINTERS];
	
	/*
	int* tttmp = (int*) tmp;
	for(int i=0; i<256; i++){
		printf("%d,", tttmp[i]);
		if( i % 10 == 0 ) printf("\n");
	}
	printf("\n");
	void* k = tmp;
	void* rd = tmp + sizeof(int) * MAX_KEYS;
	
	memcpy(k, tmp, sizeof(int)*MAX_KEYS);
	memcpy(rd, tmp + sizeof(int) * MAX_KEYS, sizeof(RecordId) * MAX_POINTERS);
	
	RecordId* rrd = (RecordId*) rd;
	printf("from read1\n");
	for(int i=0; i<MAX_POINTERS; i++)
		printf("|%d,%d|", rrd[i].pid, rrd[i].sid);
	printf("\n");
	*/

	int result = pf.read(pid, tmp);

	//printf("pid:%d result:%d\n", pid, result);
	int* ttmp = (int*) tmp;
	int* tmp_keys = (int*) ttmp;
	int* ttmp_records = ttmp + MAX_KEYS;
	RecordId* tmp_records = (RecordId*) ttmp_records;
	//RecordId* tmp_records = (RecordId*) tmp + sizeof(int) * MAX_KEYS;

/*
	printf("from read\n");
	for(int i=0; i<MAX_POINTERS; i++)
		printf("|%d,%d|", tmp_records[i].pid, tmp_records[i].sid);
	printf("\n");
*/	
	memcpy(keys, tmp_keys, sizeof(int) * MAX_KEYS);
	memcpy(records, tmp_records, sizeof(RecordId) * MAX_POINTERS);
	
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
	int* ttmp = (int*) tmp;
	int* tmp_keys = (int*) ttmp;
	//RecordId* tmp_records = (RecordId*) tmp + sizeof(int) * MAX_KEYS;
	int* ttmp_records = ttmp + MAX_KEYS;
	RecordId* tmp_records = (RecordId*)ttmp_records;
	memcpy(tmp_keys, keys, sizeof(int) * MAX_KEYS);
	memcpy(tmp_records, records, sizeof(RecordId) * MAX_POINTERS);
	//printf("when write pid:%d\n", pid);

	/*
		printNodeInfo();

	int* tttmp = (int*) tmp;
	for(int i=0; i<256; i++){
		printf("%d,", tttmp[i]);
		if( i % 10 == 0 ) printf("\n");
	}
	printf("\n");
	*/
	
	pf.write(pid, (void*)tmp);

	if(SH_DEBUG){
		printf("from write\n");
		for(int i=0; i<MAX_POINTERS; i++)
			printf("|%d,%d|", tmp_records[i].pid, tmp_records[i].sid);
		printf("\n");
	}
		
	//printf("after write pid:%d\n", pid);
	pf.read(pid, tmp);
	//printf("%d, %d\n", tmp_records[0].pid, tmp_records[0].sid );
	
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	int count =0;
	for(int i=0; i<MAX_KEYS; i++){
		if(keys[i] <0) break;
		count++;	
	}
	
	//if(SH_DEBUG)
		//printf("#of keys: %d\n", count);

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
	//node is full; cannot insert the key
	keyCount = getKeyCount();

	if(keyCount == MAX_KEYS) return RC_NODE_FULL;

	int tmpKey[MAX_KEYS];
	RecordId tmpRecords[MAX_POINTERS];

	memset(tmpKey, -1, sizeof(int) * MAX_KEYS);
	memset(tmpRecords, -1, sizeof(RecordId) * MAX_POINTERS);
	
	int i=0;
	
	for(i=0; i<MAX_KEYS; i++){
		if( key < keys[i] || keys[i] == -1 ){ //insert here
			tmpKey[i] 	 = key;
			tmpRecords[i] = rid;

			if(SH_DEBUG)
				printf("key: %d, pid:%d sid:%d inserted at %dth\n", key, rid.pid, rid.sid, i);
			break;
		}
		tmpKey[i] = keys[i];
		tmpRecords[i] = records[i];
	}
	for(i; i<MAX_KEYS-1; i++){
		tmpKey[i+1] 	  = keys[i];
		tmpRecords[i+1] = records[i];	
	}
	
	//copy the last pointer.
	tmpRecords[pageIndex] = records[pageIndex];
	
	memcpy(keys, tmpKey, sizeof(int) * MAX_KEYS);
	memcpy(records, tmpRecords, sizeof(RecordId) * MAX_POINTERS);
	
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
	keyCount = getKeyCount();
	int tmpKey[MAX_KEYS+1];
	RecordId tmpRecord[MAX_POINTERS+1];

	memset(tmpKey, -1, sizeof(int) * MAX_KEYS+1);
	memset(tmpRecord, -1, sizeof(RecordId) * MAX_POINTERS+1);
	
	int i=0;
	bool last = true;
	
	for(i=0; i<MAX_KEYS; i++){
		if( key < keys[i] || keys[i] == -1 ){ //insert here
			tmpKey[i] 	 = key;
			tmpRecord[i] = rid;
			last = false;
			if(SH_DEBUG)
				printf("key: %d, pid:%d sid:%d inserted at %dth\n", key, rid.pid, rid.sid, i);
			break;
		}
		tmpKey[i] = keys[i];
		tmpRecord[i] = records[i];
	}
	for(i; i<MAX_KEYS; i++){
		tmpKey[i+1] 	  = keys[i];
		tmpRecord[i+1] = records[i];	
	}
	if(last){
		tmpKey[i] = key;
		tmpRecord[i] = rid;
	}
	
	//copy the last pointer.
	tmpRecord[pageIndex+1] = records[pageIndex];

	if(SH_DEBUG){
		for(i=0; i<=MAX_KEYS; i++){
			printf("|%d|", tmpKey[i]);
		}
		printf("\n");
		for(i=0; i<=MAX_POINTERS; i++){
			printf("|%d,%d|", tmpRecord[i].pid, tmpRecord[i].sid);
		}
		printf("\n");
	}
	
	keyCount++;

	siblingKey = tmpKey[keyCount / 2];

	if(SH_DEBUG)
		printf("siblingKey=%d\n", siblingKey);

	memset(keys, -1, sizeof(int) * MAX_KEYS);
	memset(records, -1, sizeof(RecordId) * MAX_POINTERS);
	
	memset(sibling.keys, -1, sizeof(int) * MAX_KEYS); 
	memset(sibling.records, -1, sizeof(RecordId) * MAX_POINTERS);
	
	if(SH_DEBUG)
		printf("KeyCount=%d\n", keyCount);
		
	for(i=0; i < keyCount/2; i++){
		keys[i] = tmpKey[i];
		records[i] = tmpRecord[i];
	}	

	int j=0;
	for(i=keyCount/2; i<MAX_KEYS+1; i++){
		sibling.keys[j] = tmpKey[i];
		sibling.records[j] = tmpRecord[i];
		j++;			
	}
	sibling.records[j] = tmpRecord[i];
	sibling.records[pageIndex] = tmpRecord[pageIndex+1];
	

	//for debugging
	if(SH_DEBUG){
		printf("original\n");
		for(i=0; i<MAX_KEYS; i++){
			printf("|%d|", keys[i]);
		}
		printf("\n");
		for(i=0; i<MAX_POINTERS; i++){
			printf("|%d,%d|", records[i].pid, records[i].sid);
		}
		printf("\n");
	
		printf("sibling\n");
		for(i=0; i<MAX_KEYS; i++){
			printf("|%d|", sibling.keys[i]);
		}
		printf("\n");
		for(i=0; i<MAX_POINTERS; i++){
			printf("|%d,%d|", sibling.records[i].pid, sibling.records[i].sid);
		}
		printf("\n");
	}
	
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
	keyCount = getKeyCount();
	int i;
	for(i=0; i<keyCount; i++){
		if( searchKey <= keys[i] ){
			eid = i;
			return 0;		
		}
	}
	eid = i-1;
	//	return RC_NO_SUCH_RECORD; 
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
	key = keys[eid];
	rid = records[eid];
	return 0; 
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	return records[pageIndex].pid;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	records[pageIndex].pid = pid;
	return 0; 
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//BTNonLeafNode

/*
 *BTNonLeafNode constructor
 *initialize member variables
 *added
*/
BTNonLeafNode::BTNonLeafNode(){

	memset(keys, -1, sizeof(int) * MAX_KEYS);
	memset(pageIds, -1, sizeof(PageId) * MAX_POINTERS);	
	//tmp = new int[MAX_POINTERS + MAX_KEYS];
	//tmp = (void*) malloc( sizeof(int) * MAX_KEYS + sizeof(PageId) * MAX_POINTERS);
	tmp = (void*) malloc(1024);
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
	//int* tmp = new int[MAX_POINTERS + MAX_KEYS];
	int result = pf.read(pid, (void*) tmp);

	/*
	int* ttmp = (int*) tmp;
	int* tmp_keys = (int*) ttmp;
	PageId* tmp_pids = (PageId*) (ttmp + sizeof(int) * MAX_KEYS);
	*/
	int* ttmp = (int*) tmp;
	int* tmp_keys = (int*) ttmp;
	int* ttmp_pids = ttmp + MAX_KEYS;
	PageId* tmp_pids = (PageId*)ttmp_pids;
	
	memcpy(keys, tmp_keys, sizeof(int) * MAX_KEYS);
	memcpy(pageIds, tmp_pids, sizeof(PageId) * MAX_POINTERS);

	printNodeInfo();
	return result;
}

void
BTNonLeafNode::printNodeInfo(){
if(SH_DEBUG){
	keyCount = getKeyCount();
	int i;
	
	for(i=0; i<keyCount; i++){
		printf("|%d|", keys[i]);
	}
	printf("\n");
	for(i=0; i<keyCount+1; i++){
		printf("|%d|", pageIds[i]);
	}
	printf("\n");
}
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
	/*
	int* ttmp = (int*) tmp;
	int* tmp_keys = (int*) ttmp;
	PageId* tmp_pids = (PageId*) (ttmp + sizeof(int) * MAX_KEYS);
	*/

	int* ttmp = (int*) tmp;
	int* tmp_keys = (int*) ttmp;
	int* ttmp_pids = ttmp + MAX_KEYS;
	PageId* tmp_pids = (PageId*)ttmp_pids;

	memcpy(tmp_keys, keys, sizeof(int) * MAX_KEYS);
	memcpy(tmp_pids, pageIds, sizeof(PageId) * MAX_POINTERS);

	int result = pf.write(pid, (void*)tmp);
	
	//delete[] tmp;
	return result;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	int count =0;	
	for(int i=0; i<MAX_KEYS; i++){
		if(keys[i] <0) break;
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
	//node is full; cannot insert the key
	if(keyCount == MAX_KEYS) return RC_NODE_FULL;

	keyCount = getKeyCount();
	
	int tmpKey[MAX_KEYS];
	PageId tmpPageId[MAX_POINTERS];
		
	memset(tmpKey, -1, sizeof(int) * MAX_KEYS);
	memset(tmpPageId, -1, sizeof(PageId) * MAX_POINTERS);
		
	int i;
		
	for(i=0; i<MAX_KEYS; i++){
		if( key < keys[i] || keys[i] == -1 ){ //insert here
			tmpKey[i] = key;
			tmpPageId[i] = pageIds[i];
			tmpPageId[i+1] = pid;
			
			if(SH_DEBUG)
				printf("key: %d, pid:%d inserted at %dth\n", key, pid, i);
			break;
		}
		tmpKey[i] = keys[i];
		tmpPageId[i] = pageIds[i];
	}
	
	for(i; i<MAX_KEYS-1; i++){
		tmpKey[i+1] = keys[i];
		tmpPageId[i+2] = pageIds[i+1];
	}
	
	memcpy(keys, tmpKey, sizeof(int) * MAX_KEYS);
	memcpy(pageIds, tmpPageId, sizeof(PageId) * MAX_POINTERS);
	
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
	keyCount = getKeyCount();
	int tmpKey[MAX_KEYS+1];
	PageId tmpPageId[MAX_POINTERS+1];

	memset(tmpKey, -1, sizeof(int) * (MAX_KEYS+1));
	memset(tmpPageId, -1, sizeof(PageId) * (MAX_POINTERS+1));

	int i;
	bool last = true;		
	for(i=0; i<MAX_KEYS; i++){
		if( key < keys[i] || keys[i] == -1 ){ //insert here
			tmpKey[i] = key;
			tmpPageId[i] = pageIds[i];
			tmpPageId[i+1] = pid;
			last = false;
			if(SH_DEBUG)
				printf("key: %d, pid:%d inserted at %dth\n", key, pid, i);
			break;
		}
		tmpKey[i] = keys[i];
		tmpPageId[i] = pageIds[i];
	}
	
	for(i; i<MAX_KEYS; i++){
		tmpKey[i+1] = keys[i];
		tmpPageId[i+2] = pageIds[i+1];
	}
	if(last){
		tmpKey[i] = key;
		tmpPageId[i] = pageIds[i];
		tmpPageId[i+1] = pid;	
	}
	

	//debug
	if(SH_DEBUG){
		for(i=0; i<=MAX_KEYS; i++){
			printf("|%d|", tmpKey[i]);
		}
		printf("\n");
		for(i=0; i<=MAX_POINTERS; i++){
			printf("|%d|", tmpPageId[i]);
		}
			printf("\n");
	}

	keyCount++;

	midKey = tmpKey[keyCount / 2];

	if(SH_DEBUG)
		printf("midKey=%d\n", midKey);

	memset(keys, -1, sizeof(int) * MAX_KEYS);
	memset(pageIds, -1, sizeof(PageId) * MAX_POINTERS);
	
	memset(sibling.keys, -1, sizeof(int) * MAX_KEYS); 
	memset(sibling.pageIds, -1, sizeof(PageId) * MAX_POINTERS);
	
	if(SH_DEBUG)
		printf("KeyCount=%d\n", keyCount);
		
	for(i=0; i < keyCount/2; i++){
		keys[i] = tmpKey[i];
		pageIds[i] = tmpPageId[i];
	}	
	pageIds[i] = tmpPageId[i];

	int j=0;
	for(i=keyCount/2 + 1; i<MAX_KEYS+1; i++){
		sibling.keys[j] = tmpKey[i];
		sibling.pageIds[j] = tmpPageId[i];
		j++;			
	}
	sibling.pageIds[j] = tmpPageId[i];
	
	if(SH_DEBUG){
		printf("Original\n");	
		for(i=0; i<MAX_KEYS; i++){
			printf("|%d|", keys[i]);
		}
		printf("\n");
		for(i=0; i<MAX_POINTERS; i++){
			printf("|%d|", pageIds[i]);
		}
		printf("\n");
	
		printf("Sibling\n");	
		for(i=0; i<MAX_KEYS; i++){
			printf("|%d|", sibling.keys[i]);
		}
		printf("\n");
		for(i=0; i<MAX_POINTERS; i++){
			printf("|%d|", sibling.pageIds[i]);
		}
		printf("\n");
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
	keyCount = getKeyCount();
	int i;
	
	for(i=0; i<keyCount; i++){
		if( searchKey < keys[i] ){
			pid = pageIds[i];
			return 0;
		}	
	}
	pid = pageIds[i];
	return 0;
	//return RC_NO_SUCH_RECORD; 
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
	keys[0] = key;
	pageIds[0] = pid1;
	pageIds[1] = pid2;

	return 0; 
}
#include "BTreeNode.h"
#include <stdint.h>
#include <iostream>
using namespace std;
//////////////////////////////////// TO DO /////////////////////////
//MAKE A CONSTRUCTOR FOR EACH CLASS SUCH THAT IT INITIALIZES FIRST BYTE AS THE THE NODE TYPE.
//FIGURE OUT A WAY TO SET THE NODE ID AND PARENT ID . DO WE NEED A FUNCTION FOR THAT?
//
BTLeafNode::BTLeafNode()
{
	int i = 0;
	for(i = 0; i < PageFile::PAGE_SIZE; i++)
	{
		buffer[i] = '0';
	}
	buffer[0] = '1'; //leaf type
	i = 0;
	//initialize count to 0
	memcpy(buffer + 4, &i, sizeof(int));
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
	RC   rc;
	if ((rc = pf.read(pid, buffer)) < 0) return rc;
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
	RC rc;
	if ((rc = pf.write(pid, buffer)) < 0) return rc;
	return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
	// the second four bytes of a page contains # records in the page
  int count;
  memcpy(&count, buffer + 4, sizeof(int));
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
	int key_pairs;
	int j;
	key_pairs = getKeyCount();
	int loop_var = 0;
	if (key_pairs == MAX_KEY_COUNT)
	{ return RC_NODE_FULL;}
	
	char temp[PageFile::PAGE_SIZE];
	char* ptr ; //= buffer + OFFSET;
	//int offset;
	int value;
	int new_number;
	int i = OFFSET; //loop variable
	
	if(key_pairs == 0)
	{
		new_number = key_pairs + 1;
		memcpy(buffer + 4, &new_number, sizeof(int));
		memcpy(buffer + OFFSET, &rid, 8);
		memcpy(buffer + OFFSET + 8, &key, 4);
		return 0; //write succesful. return.
	}
	
	ptr = buffer + OFFSET;
	memcpy(&value, ptr + 8, sizeof(int));//GET FIRST PAIR VALUE
	for(i = 0; i < key_pairs && key >= value ; i ++)
	{
		if(key == value){return RC_INVALID_ATTRIBUTE;}
		ptr = ptr + 12;
		memcpy(&value, ptr + 8, sizeof(int));
	}
	if((key_pairs) == i)
	{
	  //cout<<"entering equal with value = "<<key<<endl;
		//ptr is pointing to the end of the last pair
		new_number = key_pairs + 1;
		memcpy(buffer + 4, &new_number , sizeof(int));//change the key_pair value
		memcpy(ptr, &rid, 8);
		memcpy(ptr + 8, &key, sizeof(int)); 
		return 0;
	}
	else
	{
	  	//ptr pointing to the begining of the value pair where key is greater then inserting value
		//i has pairs that were less 
		ptr = buffer + OFFSET + i*12;
		new_number = key_pairs + 1;
		memcpy(buffer + 4, &new_number, sizeof(int));//change the key_pair value
			
		for(j = 0; j < OFFSET + i*12; j++)
		{
			temp[j] = buffer[j];
		}
		
		memcpy(temp + j, &rid, 8);
		memcpy(temp + j + 8, &key, 4);
		loop_var = OFFSET + i*12;;
		
		j = j + 12;
	       
		for(; loop_var < OFFSET +i*12+ (key_pairs - i)*12; j++, loop_var++)
		{
			temp[j] = buffer[loop_var];
		}
		for( j = PageFile::PAGE_SIZE - 4; j < PageFile::PAGE_SIZE; j++)
		  {
		    temp[j] = buffer[j];
		  }
		
		for(j = 0; j < PageFile::PAGE_SIZE; j++)
		{
			buffer[j] = temp[j];
			//cout<<endl<<"index value = "<< j <<" temp value = "<<int(temp[j]);
		}
		//cout<<endl;
		return 0;
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
	//char temp_main[PageFile::PAGE_SIZE + 12 ];
	//char temp_newnode[PageFile::PAGE_SIZE];
	char temp[PageFile::PAGE_SIZE ];
	int key_pairs;
	int j;
	int i;
	int loop_var = 0;
	int loop_var2 = 0;
	int value;
	char* ptr;
	int new_number;
	//int remaining;
	
	key_pairs = getKeyCount();
	int count = (key_pairs +1)/ 2; //divide the values in half.
	
	ptr = buffer + OFFSET;
	memcpy(&value, ptr + 8, sizeof(int));//GET FIRST PAIR VALUE
	for(i = 0; i < key_pairs && key >= value ; i ++)
	{
		if(key == value){return RC_INVALID_ATTRIBUTE;}
		ptr = ptr + 12;
		memcpy(&value, ptr + 8, sizeof(int));
	}
	//case when record to be inserted at last!
	if(key_pairs == i)
	{
		//ptr is pointing to the end of the last pair
		new_number = key_pairs + 1;
		count = (key_pairs +1)/ 2;
		memcpy(buffer + 4, &new_number , sizeof(int));//change the key_pair value
		memcpy(ptr, &rid, 8);
		memcpy(ptr + 8, &key, sizeof(int)); 
		
		/////write to the current OFFSET node and sibling.
		for(j = 0; j < OFFSET; j++)
		{
			sibling.buffer[j] = buffer[j]; //copy the first OFFSET bytes or the meta data
		}
		
		//copy the values from halfway point into the sibling buffer
		ptr = sibling.buffer + OFFSET + count*12;
		
		loop_var = j; //loop_var stores the current position in the sibling buffer
		j = OFFSET + count*12; //set the cursor to the begining of the record
		for(; j < OFFSET + (key_pairs + 1)*12; j++, loop_var++) //
		{
			sibling.buffer[loop_var] = buffer[j];
		}
		
		for( j = PageFile::PAGE_SIZE - 4; j < PageFile::PAGE_SIZE; j++)
		{
		    sibling.buffer[j] = buffer[j];
		}
		  
		//set the value count in buffer and sibling buffer to their correct values.
		memcpy(buffer + 4, &count , sizeof(int));//change the key_pair value
		new_number = (key_pairs + 1) - count;
		memcpy(sibling.buffer + 4, &new_number, sizeof(int));
		memcpy(&siblingKey, sibling.buffer + 16 + 8, sizeof(int));		//send out sibling key
		//set the pageid in the sibling to that of original. already done.
		
		/////////// to do. ///////////
		//make sure u set the pageid of child to point to sibling.
		//make sure u set the current_node of the sibling.
		ptr = 0;
		return 0;
	}
	//case when record to be inserted somewhere in the begining or in middle!
	else
	{
		//ptr pointing to the begining of the value pair where key is greater then inserting value
		//i has pairs that were less 
		ptr = buffer + OFFSET + i*12;
		new_number = key_pairs + 1;
		memcpy(buffer + 4, &new_number, sizeof(int));//change the key_pair value
			
		for(j = 0; j < OFFSET + i*12; j++)
		{
			temp[j] = buffer[j];
		}
		
		memcpy(temp + j, &rid, 8);
		memcpy(temp + j + 8, &key, 4);
		loop_var2 = j;
		j = j + 12;
		for(; loop_var2 < OFFSET + key_pairs*12; j++, loop_var2++)
		{
			temp[j] = buffer[loop_var2];
		}
		for( j = PageFile::PAGE_SIZE - 4; j < PageFile::PAGE_SIZE; j++)
		  {
		    temp[j] = buffer[j];
		  }	
		for(j = 0; j < PageFile::PAGE_SIZE; j++)
		{
			buffer[j] = temp[j];
		}
		//writting to that extra space in buffer done. now split it!.
		
		new_number = key_pairs + 1;
		count = (key_pairs +1)/ 2;
		/////write to the current node and sibling.
		for(j = 0; j < OFFSET; j++)
		{
			sibling.buffer[j] = buffer[j]; //copy the first OFFSET bytes or the meta data
		}
		//set the value count in buffer and sibling buffer to their correct values.
		loop_var = j; //loop_var stores the current position in the sibling buffer
		j = OFFSET + count*12; //set the cursor to the begining of the record
		for(; j < OFFSET + (key_pairs + 1)*12; j++, loop_var++) //remember. the buffer array is actually bigger now!
		{
			sibling.buffer[loop_var] = buffer[j];
		}
		
		for( j = PageFile::PAGE_SIZE - 4; j < PageFile::PAGE_SIZE; j++)
		{
		    sibling.buffer[j] = buffer[j];
		}
		  
		//set the value count in buffer and sibling buffer to their correct values.
		memcpy(buffer + 4, &count , sizeof(int));//change the key_pair value
		new_number = (key_pairs + 1) - count;
		memcpy(sibling.buffer + 4, &new_number, sizeof(int));
		memcpy(&siblingKey, sibling.buffer + 16 + 8, sizeof(int));
		//set the pageid in the sibling to that of original. already done.
		
		/////////// to do. ///////////
		//make sure u set the pageid of child to point to sibling.
		//make sure u set the current_node of the sibling.
		ptr = 0;
		return 0;
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
 //pairs start from 0, 1 ...
RC BTLeafNode::locate(int searchKey, int& eid)
{
	eid = -1;
	int count = getKeyCount();
	char* ptr = buffer + OFFSET;//point to start of record id, key pair
	int i = 0;
	int key_stored;
	
	if (count <= 0){return RC_NO_SUCH_RECORD;}
	
	for(i = 0; i < count; i++)
	{
		memcpy(&key_stored, ptr + 8, sizeof(int));
		if(key_stored >= searchKey)
		{
			eid = i;
			return 0;
		}
		ptr = ptr + 12; //point to start of next record id, key pair
	}
	
	//value not found. set to something default! eid = -1
	eid = getKeyCount();					///////change here!!!
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
	//eid between 0 to count() -1;
	//getKeyCount() returns count;
	//max index is counts - 1;
	int count = getKeyCount();
	if (count <= eid){return RC_NO_SUCH_RECORD;}
	
	char *ptr = buffer + OFFSET + 12*eid; //point to begining of the rid and key pair
	memcpy(&key, ptr + 8, sizeof(int));
	memcpy(&rid, ptr, 8);

	return 0; 
}

/*
 * Output the pid of the next slibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	PageId value;
	memcpy(&value, buffer + PageFile::PAGE_SIZE - 4, 4);
	return value; 
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 	//PageId value;
	memcpy(buffer + PageFile::PAGE_SIZE - 4, &pid,  4); 
	return 0;
	
	//please make sure u call write function from BTreeIndex. Also, what is RC supposed to return?
	//what should the offset be? -4 or -5?
}

char* BTLeafNode::get_buffer()
{
	return buffer;
}


//sets the parents PID to the given PID
RC BTLeafNode::setparentPtr(PageId pid)
{
	memcpy(buffer + 4 + 4, &pid, sizeof(int));
	return 0;
	
}

//sets the childs own PID to the given PID
RC BTLeafNode::setownPtr(PageId pid)
{
	//first four + value_count + parent id
	memcpy(buffer + 4 + 4 + 4, &pid, sizeof(int));
	return 0;
}

//returns the parents pid
PageId BTLeafNode::getparentPtr()
{
	PageId pid;
	memcpy(&pid, buffer + 4 + 4, sizeof(int));
	return pid;
}
	
//returns the parents pid
PageId BTLeafNode::getownPtr()
{
	PageId pid;
	memcpy(&pid, buffer + 4 + 4 + 4, sizeof(int));
	return pid;
}

/////////////////////
////non leafe starts here!!
///////////////////////////

/// making a constructor here!
BTNonLeafNode::BTNonLeafNode()
{
	int i = 0;
	for(i = 0; i < PageFile::PAGE_SIZE; i++)
	{
		buffer[i] = '0';
	}
	buffer[0] = '0'; // non leaf type
	i = 0;
	//initialize count to 0
	memcpy(buffer + 4, &i, sizeof(int));
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
	RC   rc;
	if ((rc = pf.read(pid, buffer)) < 0) return rc;
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
	RC rc;
	if ((rc = pf.write(pid, buffer)) < 0) return rc;
	return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	// the second four bytes of a page contains # records in the page
  int count;
  memcpy(&count, buffer + 4, sizeof(int));
  return count;
 }


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
 //Point to node, the pid is for >= key. this is opposite to the leaf.
RC BTNonLeafNode::insert(int key, PageId pid)
{
	int key_pairs;
	int j;
	int loop_var = 0;
	key_pairs = getKeyCount();
	
	if ( key_pairs == MAX_KEY_COUNT_NONLEAF )
	{ return RC_NODE_FULL;}
	
	char temp[PageFile::PAGE_SIZE];
	char* ptr ; //= buffer + OFFSET;
	//int offset;
	int value;
	int new_number;
	// ---- we are assuming that the non leaf given is never empty --- 
	int i = OFFSET + sizeof(pid); //loop variable ----- IMPORTANT!!! THE FIRST POINTER NEVER CHANGES! ---------
	ptr = buffer + OFFSET + sizeof(pid); // ----- IMPORTANT!!! THE FIRST POINTER NEVER CHANGES! ---------
	
	memcpy(&value, ptr, sizeof(key));//GET FIRST PAIR VALUE
	for(i = 0; i < key_pairs && key >= value ; i ++)
	{
		if(key == value){return RC_INVALID_ATTRIBUTE;}
		ptr = ptr + sizeof(key) + sizeof(pid); //shift by 4 + 4
		memcpy(&value, ptr, sizeof(int));
	}
	if(key_pairs == i)
	{
		//ptr is pointing to the end of the last pair
		new_number = key_pairs + 1;
		memcpy(buffer + 4, &new_number , sizeof(int));//change the key_pair value
		memcpy(ptr, &key, sizeof(key)); 
		memcpy(ptr + sizeof(key), &pid, sizeof(pid));
		return 0;
	}
	else
	{
		//ptr pointing to the begining of the value pair where key is greater then inserting value
		//i has pairs that were less 
		ptr = buffer + OFFSET +  sizeof(pid) + i*(sizeof(key) + sizeof(pid)); //ptr is OFFSET + first pid + the size of entries!
		new_number = key_pairs + 1;
		memcpy(buffer + 4, &new_number, sizeof(int));//change the key_pair value
			
		for(j = 0; j < OFFSET + sizeof(pid) + i*(sizeof(key) + sizeof(pid)); j++)
		{
			temp[j] = buffer[j];
		}
		
		memcpy(temp + j, &key, sizeof(key));
		memcpy(temp + j + sizeof(key), &pid, sizeof(pid));
		loop_var = j;
		j = j + sizeof(key) + sizeof(pid); //j = j + 8
		for(; loop_var < OFFSET + sizeof(int) + key_pairs*(sizeof(key) + sizeof(pid)); j++, loop_var++)
		{
			temp[j] = buffer[loop_var];
		}
		
		for(j = 0; j < PageFile::PAGE_SIZE; j++)
		{
			buffer[j] = temp[j];
		}
		return 0;
	}

	return RC_INVALID_ATTRIBUTE;
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
	char temp[PageFile::PAGE_SIZE ];
	int key_pairs;
	unsigned int j;
	int i;
	unsigned int loop_var = 0;
	unsigned int loop_var2 = 0;
	int value;
	char* ptr;
	int new_number;
	//int remaining;
	
	key_pairs = getKeyCount();
	int count = (key_pairs +1)/ 2; //divide the values in half.
	
	ptr = buffer + OFFSET + sizeof(pid); //point to the begining of the key, pid pair. so now u point to begining of the key.
	
	memcpy(&value, ptr, sizeof(int));//GET FIRST PAIR VALUE
	for(i = 0; i < key_pairs && key >= value ; i ++)
	{
		if(key == value){return RC_INVALID_ATTRIBUTE;}
		ptr = ptr +  sizeof(key) + sizeof(pid);
		memcpy(&value, ptr, sizeof(int));
	}
	//case when record to be inserted at last!
	if(key_pairs == i)
	{
		//ptr is pointing to the end of the last pair
		int temp_count = MAX_KEY_COUNT_NONLEAF / 2; // the idea is start copying in sibling 1 point after the current one.
		new_number = key_pairs + 1;
		count = (key_pairs + 1) / 2;
		memcpy(buffer + 4, &new_number , sizeof(int));//change the key_pair value
		memcpy(ptr, &key, sizeof(key)); 
		memcpy(ptr +  sizeof(key), &pid, sizeof(pid));
		
		
		/////write to the current OFFSET node and sibling.
		for(j = 0; j < OFFSET; j++)
		{
			sibling.buffer[j] = buffer[j]; //copy the first OFFSET bytes or the meta data
		}
		
		//copy the values from halfway point into the sibling buffer
		ptr = buffer + OFFSET + sizeof(int) + (temp_count + 1)*(sizeof(key) + sizeof(pid));
		
		loop_var = j; //loop_var stores the current position in the sibling buffer
		j = OFFSET + sizeof(int) + (temp_count + 1)*(sizeof(key) + sizeof(pid)) - sizeof(pid); //set the cursor to the begining of the pid of middle record.
		for(; j < OFFSET + sizeof(int) + (key_pairs + 1)*(sizeof(key) + sizeof(pid)); j++, loop_var++) //
		{
			sibling.buffer[loop_var] = buffer[j];
		}
		

		//set the value count in buffer and sibling buffer to their correct values.
		memcpy(buffer + 4, &temp_count , sizeof(int));//change the key_pair value
		new_number = (key_pairs + 1) - temp_count - 1;
		memcpy(sibling.buffer + 4, &new_number, sizeof(int));
		memcpy(&midKey, ptr - 2*sizeof(int), sizeof(int)); //copy the middle key
		
		//set the pageid in the sibling to that of original. already done.
		
		/////////// to do. ///////////
		//make sure u set the pageid of child to point to sibling.
		//make sure u set the current_node of the sibling.
		ptr = 0;
		return 0;
	}
	//case when record to be inserted somewhere in the begining or in middle!
	else
	{
		//ptr pointing to the begining of the value pair where key is greater then inserting value
		//i has pairs that were less 
		//ptr = sibling.buffer + OFFSET + sizeof(int) + (temp_count + 1)*(sizeof(key) + sizeof(pid));
		
		int temp_count = MAX_KEY_COUNT_NONLEAF / 2; // the idea is start copying in sibling 1 point after the current one.
		ptr = buffer + OFFSET + sizeof(int) + i*(sizeof(key) + sizeof(pid));
		new_number = key_pairs + 1;
		memcpy(buffer + 4, &new_number, sizeof(int));//change the key_pair value
			
		for(j = 0; j < OFFSET + sizeof(int) + i*(sizeof(key) + sizeof(pid)); j++)
		{
			temp[j] = buffer[j];
		}
		
		memcpy(temp + j, &key, sizeof(key));
		memcpy(temp + j + sizeof(int), &pid, sizeof(pid));
		
		loop_var2 = j;
		j = j + (sizeof(key) + sizeof(pid));
		for(; loop_var2 < OFFSET + sizeof(int) + key_pairs*(sizeof(key) + sizeof(pid)); j++, loop_var2++)
		{
			temp[j] = buffer[loop_var2];
			//cout<<PageFile::PAGE_SIZE<<endl;
			//cout<<loop_var<<endl;
		}
		
		for(j = 0; j < PageFile::PAGE_SIZE; j++)
		{
			buffer[j] = temp[j];
		}
		//writting to that extra space in buffer done. now split it!.
		
		for(j = 0; j < OFFSET; j++) //don't want to include the very first pointer
		{
			sibling.buffer[j] = buffer[j]; //copy the first OFFSET bytes or the meta data
		}
		
		//copy the values from halfway point into the sibling buffer
		ptr = buffer + OFFSET + sizeof(int) + (temp_count + 1)*(sizeof(key) + sizeof(pid));
		
		loop_var = j; //loop_var stores the current position in the sibling buffer
		j = OFFSET + sizeof(int) + (temp_count + 1)*(sizeof(key) + sizeof(pid)) - sizeof(pid); //set the cursor to the begining of the pid of middle record.
		for(; j < OFFSET + sizeof(int) + (key_pairs + 1)*(sizeof(key) + sizeof(pid)); j++, loop_var++) //
		{
			sibling.buffer[loop_var] = buffer[j];
		}

		//set the value count in buffer and sibling buffer to their correct values.
		memcpy(buffer + 4, &temp_count , sizeof(int));//change the key_pair value
		new_number = (key_pairs + 1) - temp_count - 1;
		memcpy(sibling.buffer + 4, &new_number, sizeof(int));
		memcpy(&midKey, ptr - 2*sizeof(int), sizeof(int)); //copy the middle key
			
		/////////// to do. ///////////
		//make sure u set the pageid of child to point to sibling.
		//make sure u set the current_node of the sibling.
		ptr = 0;
		return 0;
	}

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
	int count = getKeyCount();
	char* ptr = buffer + OFFSET;//point to start of pid
	int i = 0;
	int key_stored;
	
	if (count <= 0){return RC_NO_SUCH_RECORD;}
	
	for(i = 0; i < count; i++)
	{
		memcpy(&key_stored, ptr + sizeof(int), sizeof(int)); //pointing to pid, increase by 4 or pid size to get to key
		if( searchKey < key_stored)
		{
			//eid = i;
			memcpy(&pid, ptr, sizeof(int));
			return 0;
		}
		ptr = ptr + 2*sizeof(int); //point to start of next key pair, record id
	}
	//memcpy(&pid, ptr + sizeof(searchKey), sizeof(int)); //return the last value
	memcpy(&pid, ptr, sizeof(int));
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
	int key_pairs;
	key_pairs = 1;
	memcpy(buffer + 4, &key_pairs, sizeof(int));
	memcpy(buffer + OFFSET, &pid1, sizeof(pid1));
	memcpy(buffer + OFFSET + sizeof(pid1), &key, sizeof(key));
	///////////////////// correct////////////////////////////
	memcpy(buffer + OFFSET + sizeof(pid1) + sizeof(key), &pid2, sizeof(pid2));
	//////////// not correct one//////////////////////
	//memcpy(buffer + PageFile::PAGE_SIZE - sizeof(pid2), &pid2, sizeof(pid2));
	
	return 0; //write succesful. return.
}

//sets the parents PID to the given PID
RC BTNonLeafNode::setparentPtr(PageId pid)
{
	memcpy(buffer + 4 + 4, &pid, sizeof(int));
	return 0;
	
}

//sets the childs own PID to the given PID
RC BTNonLeafNode::setownPtr(PageId pid)
{
	//first four + value_count + parent id
	memcpy(buffer + 4 + 4 + 4, &pid, sizeof(int));
	return 0;
}

//returns the parents pid
PageId BTNonLeafNode::getparentPtr()
{
	PageId pid;
	memcpy(&pid, buffer + 4 + 4, sizeof(int));
	return pid;
}
	
//returns the parents pid
PageId BTNonLeafNode::getownPtr()
{
	PageId pid;
	memcpy(&pid, buffer + 4 + 4 + 4, sizeof(int));
	return pid;
}

char* BTNonLeafNode::get_buffer()
{
	return buffer;
}















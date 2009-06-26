#include "BTreeNode.h"
#include <iostream>

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
	RC rc;
	rc = pf.read(pid, buffer);
	
	if(rc < 0)
		return rc;
	
	return 0;
}

RC BTLeafNode::set_parent_pid(PageId pid)
{
	memcpy(buffer+8, &pid, sizeof(int));
	return 0;
}

RC BTLeafNode::set_own_pid(PageId pid)
{
	memcpy(buffer+12, &pid, sizeof(int));
	return 0;
}

PageId BTLeafNode::get_parent_pid()
{
	PageId temp;
	memcpy(&temp, buffer+8, sizeof(int));
	return temp;
}

PageId BTLeafNode::get_own_pid()
{
	PageId temp;
	memcpy(&temp, buffer+12, sizeof(int));
	return temp;
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
	rc = pf.write(pid, buffer);
	
	if(rc < 0)
		return rc;

 	return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	int key_count = 0;
	// from buffer ptr, get the first 4 bytes and return
	memcpy(&key_count, buffer+4, sizeof(int));
	return key_count;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
	if(getKeyCount() >= MAX_KEYS)
	{
		return RC_NODE_FULL;
	}
	else
	{
		// if there are no keys in the node
		if(getKeyCount() == 0)
		{
			memcpy((buffer+META_SIZE), &rid, 8);
			memcpy((buffer+META_SIZE)+8, &key, sizeof(int));
		}
		else
		{
			int i = 0, to_copy = 0;
			for(i = 0; i < getKeyCount(); i++)
			{
				int temp_key = 0;
				memcpy(&temp_key, buffer+META_SIZE+(12*i)+8, sizeof(int));
				if(temp_key > key)
				{
					to_copy = i;
					break;
				}
				if(temp_key == key)
				{
					return RC_INVALID_ATTRIBUTE;
				}
			}

			if(i == getKeyCount())
			{
				memcpy(buffer+META_SIZE+(12*i), &rid, 8);
				memcpy(buffer+META_SIZE+(12*i)+8, &key, sizeof(int));
			}
			else
			{
				char temp_buf[PageFile::PAGE_SIZE];
				int t_buf_index = 0;

				// copy the contents to another array
				for(i = to_copy; i < getKeyCount(); i++)
				{
					RecordId tmp_rid;
					int tmp_key = 0;
					memcpy(&tmp_rid, buffer+META_SIZE+(12*i), 8);
					memcpy(&tmp_key, buffer+META_SIZE+(12*i)+8, sizeof(int));

					memcpy(temp_buf+(t_buf_index*12), &tmp_rid, 8);
					memcpy(temp_buf+(t_buf_index*12)+8, &tmp_key, sizeof(int));
					t_buf_index++;
				}

				// copy the new entry into the proper location
				memcpy(buffer+(12*to_copy)+META_SIZE, &rid, 8);
				memcpy(buffer+(12*to_copy)+META_SIZE+8, &key, sizeof(int));

				// copy the other entries back in
				t_buf_index = 0;
				for(i = to_copy+1; i <= getKeyCount(); i++)
				{
					RecordId tmp_rid;
					int tmp_key = 0;
					memcpy(&tmp_rid, temp_buf+(12*t_buf_index), 8);
					memcpy(&tmp_key, temp_buf+(12*t_buf_index)+8, sizeof(int));

					memcpy(buffer+META_SIZE+(i*12), &tmp_rid, 8);
					memcpy(buffer+META_SIZE+(i*12)+8, &tmp_key, sizeof(int));
					t_buf_index++;
				}
			}
		}
	}
	
	// update the count
	int old_count = 0;
	memcpy(&old_count, buffer+4, sizeof(int));
	old_count++;
	memcpy(buffer+4, &old_count, sizeof(int));
	return 0;
}

/*
 * for a given PageId pid, find the parent of that node
 */
//RC BTLeafNode::parent(PageId& pid)
//{
	
//}

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
	// if there are no keys in the node
	if(getKeyCount() == 0)
	{
		memcpy((buffer+META_SIZE), &rid, 8);
		memcpy((buffer+META_SIZE)+8, &key, sizeof(int));
	}
	else
	{
		int i = 0, to_copy = 0;
		for(i = 0; i < getKeyCount(); i++)
		{
			int temp_key = 0;
			memcpy(&temp_key, buffer+META_SIZE+(12*i)+8, sizeof(int));
			if(temp_key > key)
			{
				to_copy = i;
				break;
			}
			if(temp_key == key)
			{
				return RC_INVALID_ATTRIBUTE;
			}
		}

		if(i == getKeyCount())
		{
			memcpy(buffer+META_SIZE+(12*i), &rid, 8);
			memcpy(buffer+META_SIZE+(12*i)+8, &key, sizeof(int));
		}
		else
		{
			char temp_buf[PageFile::PAGE_SIZE];
			int t_buf_index = 0;

			// copy the contents to another array
			for(i = to_copy; i < getKeyCount(); i++)
			{
				RecordId tmp_rid;
				int tmp_key = 0;
				memcpy(&tmp_rid, buffer+META_SIZE+(12*i), 8);
				memcpy(&tmp_key, buffer+META_SIZE+(12*i)+8, sizeof(int));

				memcpy(temp_buf+(t_buf_index*12), &tmp_rid, 8);
				memcpy(temp_buf+(t_buf_index*12)+8, &tmp_key, sizeof(int));
				t_buf_index++;
			}

			// copy the new entry into the proper location
			memcpy(buffer+(12*to_copy)+META_SIZE, &rid, 8);
			memcpy(buffer+(12*to_copy)+META_SIZE+8, &key, sizeof(int));

			// copy the other entries back in
			t_buf_index = 0;
			for(i = to_copy+1; i <= getKeyCount(); i++)
			{
				RecordId tmp_rid;
				int tmp_key = 0;
				memcpy(&tmp_rid, temp_buf+(12*t_buf_index), 8);
				memcpy(&tmp_key, temp_buf+(12*t_buf_index)+8, sizeof(int));

				memcpy(buffer+META_SIZE+(i*12), &tmp_rid, 8);
				memcpy(buffer+META_SIZE+(i*12)+8, &tmp_key, sizeof(int));
				t_buf_index++;
			}
		}
	}
	
	// update the count
	int old_count = 0;
	memcpy(&old_count, buffer+4, sizeof(int));
	old_count++;
	memcpy(buffer+4, &old_count, sizeof(int));

	// get the sizes of the splits. second sibling carries +1 if odd sizes
	int size_1;
	int size_2;
	
	if(getKeyCount() % 2 == 1) // odd size
	{
		size_1 = getKeyCount()/2;
		size_2 = size_1+1;
	}

	else
	{
		size_1 = size_2 = getKeyCount()/2;
	}

	int j = 0, j_2 = 0;
	RecordId temp_rid;
	int temp_key = 0;

	for(j = size_1; j < getKeyCount(); j++)
	{
		// copy the rid
		memcpy(&temp_rid, buffer+META_SIZE+(12*j), 8);
		// copy the key
		memcpy(&temp_key, buffer+META_SIZE+(12*j)+8, 4);

		// copy the rid and key over to the sibling
		memcpy((sibling.buffer)+META_SIZE+(j_2*12), &temp_rid, 8);
		memcpy((sibling.buffer)+META_SIZE+(j_2*12)+8, &temp_key, 4);
		j_2++;
	}

	// change the size of the first sibling
	memcpy(buffer+4, &size_1, sizeof(int));
	
	// set the size of sibling
	memcpy(sibling.buffer+4, &size_2, sizeof(int));

	// get the contents of the first key in sibling and return
	memcpy(&siblingKey, (sibling.buffer)+META_SIZE+8, 4);

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
	char* buf_ptr = (buffer+META_SIZE);
	int temp = -1, i = 0;

	for(i = 0; i < getKeyCount(); i++)
	{
		memcpy(&temp, buf_ptr+(i*12)+8, sizeof(int));
		if(temp == searchKey)
		{
			eid = i;
			return 0;
		}
		else if(temp >= searchKey)
		{
			eid = i;
			return 0;
		}
	}

	if(i == getKeyCount())
	{
		eid = -1;
		return RC_NO_SUCH_RECORD;
	}

	return 0;
}

/* delete this later */
char* BTLeafNode::get_buffer()
{
	return buffer;
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
	char* buf_ptr = (buffer+META_SIZE);
	// 0 <= eid <= getKeyCount() - 1
	if(eid >= getKeyCount())
		return RC_NO_SUCH_RECORD;

	memcpy(&key, buf_ptr+(12*eid)+8, 4);
	memcpy(&rid, buf_ptr+(12*eid), 8);
	return 0;
}

/*
 * Output the pid of the next sibling node.
 * @return 0 if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{
	// get the pointer @ lastKeyCount() - 1 and return
	PageId m_pid;
	memcpy(&m_pid, buffer+(PageFile::PAGE_SIZE)-4, 4);

	return m_pid;
}

/*
 * Set the pid of the next sibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	memcpy(buffer+(PageFile::PAGE_SIZE)-4, &pid, 4);
	return 0;
}

/* delete this later */
char* BTNonLeafNode::get_buffer()
{
	return buffer;
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
	RC rc;
	rc = pf.read(pid, buffer);
	
	if(rc < 0)
		return rc;
	
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
	rc = pf.write(pid, buffer);
	
	if(rc < 0)
		return rc;

	return 0;
}

RC BTNonLeafNode::set_parent_pid(PageId pid)
{
	memcpy(buffer+8, &pid, sizeof(int));
	return 0;
}

RC BTNonLeafNode::set_own_pid(PageId pid)
{
	memcpy(buffer+12, &pid, sizeof(int));
	return 0;
}

PageId BTNonLeafNode::get_parent_pid()
{
	PageId temp;
	memcpy(&temp, buffer+8, sizeof(int));
	return temp;
}

PageId BTNonLeafNode::get_own_pid()
{
	PageId temp;
	memcpy(&temp, buffer+12, sizeof(int));
	return temp;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	int key_count;
	// from buffer ptr, get the first 4 bytes and return
	memcpy(&key_count, buffer+4, sizeof(int));
	return key_count;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
	char* buf_ptr = buffer+4+META_SIZE;
	if(getKeyCount() >= MAX_KEYS)
	{
		return RC_NODE_FULL;
	}
	
	int i = 0, to_copy = 0;
	for(i = 0; i < getKeyCount(); i++)
	{
		int temp_key = 0;
		memcpy(&temp_key, buf_ptr+(8*i), sizeof(int));
		if(temp_key > key)
		{
			to_copy = i;
			break;
		}
		if(temp_key == key)
		{
			return RC_INVALID_ATTRIBUTE;
		}
	}

	if(i == getKeyCount())
	{
		memcpy(buf_ptr+(8*i)+4, &pid, sizeof(int));
		memcpy(buf_ptr+(8*i), &key, sizeof(int));
	}
	else
	{
		char temp_buf[PageFile::PAGE_SIZE];
		int t_buf_index = 0;

		// copy the contents to another array
		for(i = to_copy; i < getKeyCount(); i++)
		{
			PageId tmp_pid;
			int tmp_key = 0;
			memcpy(&tmp_pid, buf_ptr+(8*i)+4, sizeof(int));
			memcpy(&tmp_key, buf_ptr+(8*i), sizeof(int));

			memcpy(temp_buf+(t_buf_index*8)+4, &tmp_pid, sizeof(int));
			memcpy(temp_buf+(t_buf_index*8), &tmp_key, sizeof(int));
			t_buf_index++;
		}

		// copy the new entry into the proper location
		memcpy(buf_ptr+(8*to_copy)+4, &pid, sizeof(int));
		memcpy(buf_ptr+(8*to_copy), &key, sizeof(int));

		// copy the other entries back in
		t_buf_index = 0;
		for(i = to_copy+1; i <= getKeyCount(); i++)
		{
			PageId tmp_pid;
			int tmp_key = 0;
			memcpy(&tmp_pid, temp_buf+(8*t_buf_index)+4, sizeof(int));
			memcpy(&tmp_key, temp_buf+(8*t_buf_index), sizeof(int));

			memcpy(buf_ptr+(i*8)+4, &tmp_pid, sizeof(int));
			memcpy(buf_ptr+(i*8), &tmp_key, sizeof(int));
			t_buf_index++;
		}
	}

	// update the count
	int old_count = 0;
	memcpy(&old_count, buffer+4, sizeof(int));
	old_count++;
	memcpy(buffer+4, &old_count, sizeof(int));

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
	char* buf_ptr = buffer+4;
	int i = 0, to_copy = 0;
	for(i = 0; i < getKeyCount(); i++)
	{
		int temp_key = 0;
		memcpy(&temp_key, buf_ptr+META_SIZE+(8*i), sizeof(int));
		if(temp_key > key)
		{
			to_copy = i;
			break;
		}

		if(temp_key == key)
		{
			return RC_INVALID_ATTRIBUTE;
		}
	}

	if(i == getKeyCount())
	{
		memcpy(buf_ptr+META_SIZE+(8*i)+4, &pid, sizeof(int));
		memcpy(buf_ptr+META_SIZE+(8*i), &key, sizeof(int));
	}
	else
	{
		char temp_buf[PageFile::PAGE_SIZE];
		int t_buf_index = 0;

		// copy the contents to another array
		for(i = to_copy; i < getKeyCount(); i++)
		{
			PageId tmp_pid;
			int tmp_key = 0;
			memcpy(&tmp_pid, buf_ptr+META_SIZE+(8*i)+4, sizeof(int));
			memcpy(&tmp_key, buf_ptr+META_SIZE+(8*i), sizeof(int));

			memcpy(temp_buf+(t_buf_index*8)+4, &tmp_pid, sizeof(int));
			memcpy(temp_buf+(t_buf_index*8), &tmp_key, sizeof(int));
			t_buf_index++;
		}

		// copy the new entry into the proper location
		memcpy(buf_ptr+(8*to_copy)+META_SIZE+4, &pid, sizeof(int));
		memcpy(buf_ptr+(8*to_copy)+META_SIZE, &key, sizeof(int));

		// copy the other entries back in
		t_buf_index = 0;
		for(i = to_copy+1; i <= getKeyCount(); i++)
		{
			PageId tmp_pid;
			int tmp_key = 0;
			memcpy(&tmp_pid, temp_buf+(8*t_buf_index)+4, sizeof(int));
			memcpy(&tmp_key, temp_buf+(8*t_buf_index), sizeof(int));

			memcpy(buf_ptr+META_SIZE+(i*8)+4, &tmp_pid, sizeof(int));
			memcpy(buf_ptr+META_SIZE+(i*8), &tmp_key, sizeof(int));
			t_buf_index++;
		}
	}
	
	// update the count
	int old_count = 0;
	memcpy(&old_count, buffer+4, sizeof(int));
	old_count++;
	memcpy(buffer+4, &old_count, sizeof(int));

	// get the sizes of the splits. second sibling carries +1 if odd sizes
	int size_1, size_2;
	to_copy = 0;
	
	if(getKeyCount() % 2 == 1) // odd size
	{
		size_1 = size_2 = to_copy = getKeyCount()/2;
	}

	else
	{
		size_1 = to_copy = (getKeyCount()/2) - 1;
		size_2 = getKeyCount()/2;
	}

	int j = 0, j_2 = 0;
	PageId temp_pid;
	int temp_key = 0;
	char* sib_ptr = sibling.buffer+META_SIZE+4;

	for(j = size_2; j < getKeyCount(); j++)
	{
		// copy the pid and key to temp placeholders
		memcpy(&temp_pid, buf_ptr+META_SIZE+(8*j)+4, sizeof(int));
		memcpy(&temp_key, buf_ptr+META_SIZE+(8*j), sizeof(int));

		// copy the pid and key over to the sibling
		memcpy(sib_ptr+(j_2*8)+4, &temp_pid, sizeof(int));
		memcpy(sib_ptr+(j_2*8), &temp_key, sizeof(int));
		j_2++;
	}

	// change the size of the first sibling
	memcpy(buffer+4, &size_1, sizeof(int));
	
	// set the size of sibling
	memcpy(sibling.buffer+4, &size_2, sizeof(int));

	// set the midKey which is located at position to_copy
	memcpy(&midKey, buf_ptr+META_SIZE+(to_copy*8), sizeof(int));

	// get the pid of the midkey and put that as the first pid of the sibling
	PageId mid_pid;
	memcpy(&mid_pid, buf_ptr+META_SIZE+(to_copy*8)+4, sizeof(int));
	memcpy(sibling.buffer+META_SIZE, &mid_pid, sizeof(int));

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
	int i = 0, tmp_key = 0;

	for(i = 0; i < getKeyCount(); i++)
	{
		// get the key at index i
		memcpy(&tmp_key, buffer+META_SIZE+(i*8)+4, sizeof(int));
		if(searchKey < tmp_key)
		{
			memcpy(&pid, buffer+META_SIZE+(i*8), sizeof(int));
			return 0;
		}
	}

	// reaches the end
	if(i == getKeyCount())
	{
		memcpy(&pid, buffer+META_SIZE+(getKeyCount()*8), sizeof(int));
		return 0;
	}
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
	// copy the pid2 into the way end of the buffer to point to the 
	// sibling to the right of it.
	memcpy(buffer+META_SIZE, &pid1, sizeof(int));
	memcpy(buffer+META_SIZE+4, &key, sizeof(int));
	memcpy(buffer+META_SIZE+8, &pid2, sizeof(int));
	int init_size = 1;
	
	// set the size of the node to 1
	memcpy(buffer+4, &init_size, sizeof(int));
	return 0;
}

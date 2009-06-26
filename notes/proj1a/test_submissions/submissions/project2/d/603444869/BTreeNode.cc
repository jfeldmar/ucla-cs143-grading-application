#include "BTreeNode.h"

using namespace std;


//constructor
BTLeafNode::BTLeafNode() {
    numKeys = 0;
    next_node = -1;
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
    char tmp[5000];
    pf.read(pid,tmp);
    int i = 0;
    int arr[100];
    int count=0;
    while(tmp[i] != '\0') {
        while(tmp[i] == ' ')
            i++;
        int j = i;
        while(tmp[j] != ' ' && tmp[j] != '\0') {
            j++;
        }
        int size = j-i;
        int multiplier;
        if(tmp[i] == '-') {
            multiplier = -1;
            i++;
            size--;
        } else
        multiplier = 1;
        for(int m = 1; m < size; m++)
            multiplier *= 10;
        int get = 0;
        while(tmp[i] != ' ') {
            if(tmp[i] == '\0')
                break;
            int digit = tmp[i]-'0';
            get += digit*multiplier;
            multiplier /= 10;
            i++;
        }
        arr[count] = get;
        count++;
    }
    numKeys = arr[0];
    next_node = arr[1];
    int j = 0;
    count-=2;
    for(int i = 0; i < count-1; i+=3){
       key[j] = arr[i+2]; 
       recordid[j].pid = arr[i+3]; 
       recordid[j].sid = arr[i+4]; 
       j++;
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
RC BTLeafNode::write(PageId pid, PageFile& pf)
{
    //stringstream out;
    //out << numKeys;
    char* s = new char[5000]; 
    int tmp;
    int count = 0;
    bool iszero = true;
    bool isneg = false;
    for (int i = numKeys-1; i >=0; i--) {
        tmp = recordid[i].sid;
    iszero = true;
    while(tmp > 0) {
        iszero = false;
            s[count] = (tmp % 10) + '0';
            count++;
            tmp /= 10;
    }
    if(iszero)
    {
        s[count] = '0';
        count++;
    }
        s[count] = ' ';
        count++;
        tmp = recordid[i].pid;
    iszero = true;
    isneg = false;
    if (tmp < 0) {
        isneg= true;
        tmp*=-1;
    }
    while(tmp > 0) {
        iszero = false;
            s[count] = (tmp % 10) + '0';
            count++;
            tmp /= 10;
    }
    if(isneg) {
        s[count] ='-';
        count++;
    }
    if(iszero)
    {
        s[count] = '0';
        count++;
    }
        s[count] = ' ';
        count++;
        tmp = key[i];
    iszero = true;
    isneg = false;
    if (tmp < 0) {
        isneg= true;
        tmp*=-1;
    }
    while(tmp > 0) {
        iszero = false;
            s[count] = (tmp % 10) + '0';
            count++;
            tmp /= 10;
    }
    if(isneg) {
        s[count] ='-';
        count++;
    }
    if(iszero)
    {
        s[count] = '0';
        count++;
    }
        s[count] = ' ';
        count++;
    }
        tmp = next_node;
    iszero = true;
    isneg = false;
    if (tmp < 0) {
        isneg= true;
        tmp*=-1;
    }
    while(tmp > 0) {
        iszero = false;
            s[count] = (tmp % 10) + '0';
            count++;
            tmp /= 10;
    }
    if(isneg) {
        s[count] ='-';
        count++;
    }
    
    if(iszero)
    {
        s[count] = '0';
        count++;
    }
        s[count] = ' ';
        count++;
    tmp = numKeys;
    iszero = true;
    while(tmp > 0) {
        iszero = false;
            s[count] = (tmp % 10) + '0';
            count++;
            tmp /= 10;
    }
    if(iszero)
    {
        s[count] = '0';
        count++;
    }
    s[count] = ' ';
    count++;
    char* rev =new char[500];
    for(int i = 0; i < count; i++) {
        rev[i] = s[count-i-1];
    }
    //cout << rev << endl;
    pf.write(pid,rev);
/*
    out << numKeys << " ";
    for (int i = 0; i < numKeys; i++) {
        out <<  key[i] << " ";
        out <<  recordid[i].pid << " ";
        out <<  recordid[i].sid << " ";
        cout <<  key[i] << " ";
        cout <<  recordid[i].pid << " ";
        cout << recordid[i].sid << " ";
    }
    string tmp = out.str();
    char* s = new char[tmp.length()];
    for(int i = 0; i < tmp.length(); i++) {
        s[i] = tmp[i];
    }
    cout << s << endl;
    pf.write(pid,s);
    out.str("");
    out.clear();
*/
    return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ return numKeys; }

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int a_key, const RecordId& rid)
{ 
    if(numKeys >= MAX_RECORDS) {
        return 1; //node is full
    } else {
    //cout << "inserting nonleaf: " << a_key << "\n";
        //printf("inserting: %d\n", a_key);
        int tmp_key[MAX_RECORDS];
        RecordId tmp_rec[MAX_RECORDS];
        int i;
        int replace_key;
        RecordId replace_rec;
        //sort the keys
        for (i = 0; i < numKeys; i++) {
            if(key[i] > a_key) {
                replace_key=key[i];
                replace_rec=recordid[i];
                key[i] = a_key;
                recordid[i] = rid;
                numKeys++;
                break;
            }
        }
        if (i == numKeys) {
            key[numKeys] = a_key;
            recordid[numKeys] = rid;
            numKeys++;
        } else {
            for(int j = i+1; j < numKeys; j++) {
                int tmp = key[j]; 
                RecordId tmp_r = recordid[j];
                key[j] = replace_key;
                recordid[j] = replace_rec;
                replace_key=tmp;
                replace_rec=tmp_r;
            }
        }
    }
    /*
    for (int i = 0; i < numKeys; i++)
    {
        //printf("keys are: %d\n", key[i]);
    }
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
RC BTLeafNode::insertAndSplit(int a_key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{ 
    if (sibling.getKeyCount() > 0)
        return 1; //sibling should be empty!

    //split in half
    int split_size;
    insert(a_key,rid);
    if (numKeys%2) 
        split_size = numKeys/2+1;
    else
        split_size = numKeys/2;

    //cout << split_size << endl;

    for (int i = 0; i < numKeys; i++) {
        if(i >= split_size) {
            sibling.insert(key[i],recordid[i]); 
     //       cout << "sibiling inserted " << key[i] << " " << recordid[i].pid << " " << recordid[i].sid << endl;
        }
    }
    numKeys = split_size;
    /*
    for(int i = 0 ; i < numKeys; i++){
        cout << "original keys are " << key[i] << " " << recordid[i].pid << " " << recordid[i].sid << endl;
    }
    */
    siblingKey = key[split_size];
    sibling.setNextNodePtr(getNextNodePtr());
    setNextNodePtr(recordid[split_size].pid);
    return 0; 
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equal to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
    for (int i = 0; i < numKeys; i++) {
        if(key[i] >= searchKey)
        {
            eid = i; //return the index to the key array (aka eid)
            return 0;
        }
    }
    eid = numKeys-1; //return the index to the key array (aka eid)

    return 1; //there isn't a key greater than searchKey 
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& a_key, RecordId& rid)
{ 
    a_key = key[eid];
    rid = recordid[eid];
    return 0; 
}

/*
 * Output the pid of the next slibling node.
 * @return pid if successful. Return an error code if there is an error.
 */
PageId BTLeafNode::getNextNodePtr()
{ 
    return next_node; 
}

//constructor: same as initializeRoot
BTNonLeafNode::BTNonLeafNode()
{
    first_pid = 0; //starting pid
    numKeys=0; 
}

//constructor: same as initializeRoot
BTNonLeafNode::BTNonLeafNode(PageId pid1, int key, PageId pid2)
{
    first_pid = pid1; //starting pid
    numKeys=0; 
    insert(key, pid2);
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
    next_node = pid; 
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
    //cout << "reading pid: " << pid<< endl;
    char tmp[5000];
    pf.read(pid,tmp);
    //cout << "read: " << tmp << endl;
    int i = 0;
    int arr[500];
    int count=0;
    while(tmp[i] != '\0') {
        while(tmp[i] == ' ')
            i++;
        int j = i;
        while(tmp[j] != ' ' && tmp[j] != '\0') {
            j++;
        }
        int size = j-i;
        int multiplier;
        if(tmp[i] == '-') {
            multiplier = -1;
            i++;
            size--;
        } else
        multiplier = 1;
        for(int m = 1; m < size; m++)
            multiplier *= 10;
        int get = 0;
        while(tmp[i] != ' ') {
            if(tmp[i] == '\0')
                break;
            int digit = tmp[i]-'0';
            get += digit*multiplier;
            multiplier /= 10;
            i++;
        }
        arr[count] = get;
        count++;
    }
    numKeys = arr[0];
    first_pid = arr[1];
    int j = 0;
    for(int i = 1; i < count-1; i+=2){
       key[j] = arr[i+1]; 
       pageid[j] = arr[i+2]; 
       j++;
    }
    //cout << "got stack smashing?\n";
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
    //cout << "writing in: " << pid << endl;
    char* s = new char[500]; 
    int count = 0;
    bool iszero = true;
    bool isneg = false;
    int tmp;
    for (int i = numKeys-1; i >= 0; i--) {
        tmp = pageid[i];
    iszero = true;
    isneg = false;
    if (tmp < 0) {
        isneg= true;
        tmp*=-1;
    }
    while(tmp > 0) {
        iszero = false;
            s[count] = (tmp % 10) + '0';
            count++;
            tmp /= 10;
    }
    if(isneg) {
        s[count] ='-';
        count++;
    }
    if(iszero)
    {
        s[count] = '0';
        count++;
    }
        s[count] = ' ';
        count++;
        tmp = key[i];
    iszero = true;
    while(tmp > 0) {
        iszero = false;
            s[count] = (tmp % 10) + '0';
            count++;
            tmp /= 10;
    }
    if(isneg) {
        s[count] ='-';
        count++;
    }
    if(iszero)
    {
        s[count] = '0';
        count++;
    }
        s[count] = ' ';
        count++;
    }
    tmp = first_pid;
    isneg = false;
    if (tmp < 0) {
        isneg= true;
        tmp*=-1;
    }
    while(tmp > 0) {
        iszero = false;
            s[count] = (tmp % 10) + '0';
            count++;
            tmp /= 10;
    }
    if(isneg) {
        s[count] ='-';
        count++;
    }
    if(iszero)
    {
        s[count] = '0';
        count++;
    }
    s[count] = ' ';
    count++;
    tmp = numKeys;
    isneg = false;
    if (tmp < 0) {
        isneg= true;
        tmp*=-1;
    }
    while(tmp > 0) {
        iszero = false;
            s[count] = (tmp % 10) + '0';
            count++;
            tmp /= 10;
    }
    if(isneg) {
        s[count] ='-';
        count++;
    }
    if(iszero)
    {
        s[count] = '0';
        count++;
    }
    s[count] = ' ';
    count++;
    char* rev =new char[500];
    for(int i = 0; i < count; i++) {
        rev[i] = s[count-i-1];
    }
    pf.write(pid,rev);
    //cout << "wrote: " << s << endl;
    return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
    return numKeys; 
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int a_key, PageId rid)
{ 
    if(numKeys >= MAX_RECORDS) {
        return 1; //node is full
    } else {
        //printf("nonleaf inserting: %d\n", a_key);
        int tmp_key[MAX_RECORDS];
        PageId tmp_rec[MAX_RECORDS];
        int i;
        int replace_key;
        PageId replace_rec;
        //sort the keys
        for (i = 0; i < numKeys; i++) {
            if(key[i] > a_key) {
                replace_key=key[i];
                replace_rec=pageid[i];
                key[i] = a_key;
                pageid[i] = rid;
                numKeys++;
                break;
            }
        }
        if (i == numKeys) {
            key[numKeys] = a_key;
            pageid[numKeys] = rid;
            numKeys++;
        } else {
            for(int j = i+1; j < numKeys; j++) {
                int tmp = key[j]; 
                PageId tmp_r = pageid[j];
                key[j] = replace_key;
                pageid[j] = replace_rec;
                replace_key=tmp;
                replace_rec=tmp_r;
            }
        }
    }
    
    /*
    for (int i = 0; i < numKeys; i++)
    {
        //printf("nonleaf keys are: %d\n", key[i]);
    }
    */
    
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
RC BTNonLeafNode::insertAndSplit(int a_key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ 
    if (sibling.getKeyCount() > 0)
        return 1; //sibling should be empty!

    insert(a_key,pid);
    //split in half, must account for first_pid
    int split_size;
    if ((numKeys+1)%2) 
        split_size = (numKeys)/2+1;
    else
        split_size = (numKeys)/2;

    for (int i = 0; i < numKeys; i++) {
        if(i == split_size) {
            sibling.first_pid=pageid[i]; //insert the key/pageid starting from the split
        }
        if(i > split_size) {
            sibling.insert(key[i],pageid[i]); //insert the key/pageid starting from the split
        }
    }
        //cout << "insert <debug>:" << endl;
        //cout << "  first_pid: " << first_pid << endl;
    for(int i = 0; i < numKeys; i++) {
        //cout << "  key: " << key[i] << endl;
    }
        //cout << "  first_pid 2: " << sibling.first_pid << endl;
    //for(int i = 0; i < sibling.numKeys; i++) {
        //cout << "  key: " << sibling.key[i] << endl;
    //}
    midKey = key[split_size]; //set midkey to the middle key of split
    numKeys = split_size;
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
    int i;
    for(i = 0;i < numKeys; i++) {
        if(searchKey < key[i] ) {
            if (i == 0)
                pid = first_pid;
            else
                pid = pageid[i-1];
            break;
        }
    }
    if (i == numKeys)
        pid = pageid[i-1];
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
    numKeys=0;
    first_pid = pid1;
    insert(key, pid2);
    return 0;
}

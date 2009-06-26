Matthew Pham
203467103
mattvpham@gmail.com

Project 2C

LEAF NODE STRUCTURE
|| RID, KEY | RID, KEY | ... | PADDING | TYPE | NUMKEYS | NEXTNODE ||

NONLEAF NODE STRUCTURE
|| KEY, PID | KEY, PID | ... | PADDING | TYPE | NUMKEYS | PREVNODE ||

I may change the leaf node struct to be <key, rid> pairs instead of
<rid, key> pairs.

There are a couple new functions introduced for debugging purposes:
printNodes();

New helper functions:
insertInLeaf() // self explanatory : see book
insertInParent()
locateWithRecord() // same as locate except it puts the visited nodes
// on the m_visitedStack

HOW TO USE:
create an index
open("indexname", 'w');
index.insert(key, rid);
index.close();

locate an entry
open("indexname", 'r');
IndexCursor cursor;
index.locate(key, cursor);
int myKey;
RecordId myRid;
index.readForward(cursor, myKey, myRid);
// compare keys
table.read(myRid.page, myRid.slot);

ADDITIONAL INFORMATION:
Sendie, Tarun, and Seung-Ho provided help understanding the spec
and some test data, but they did not provide any programming assistance.

In order to change the maximum number of nodes, there are three
locations that must be changed: BTreeIndex.h, BTreeNode.h:leaf,
and BTreeNode.h:nonleaf



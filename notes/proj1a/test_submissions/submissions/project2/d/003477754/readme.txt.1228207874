 ----------------
| Notes: Part 2D |
 ----------------

If BTreeIndex exists, the first thing we do is to scan the conditions stored in a vector as argument and check for ranges of keys to consider as 
well as equality conditions.  If multiple equality conditions occur, and not all equate to the same value then the the query results in empty 
set.  Similarly, if an equality condition occur for a key value that is outside of set range then  

Assumption: Key values are nonnegative
			
 ------------------------
| Newly added functions: |
 ------------------------

Additional proxy functions
1) print_tuple		-> given an attribute, key, and value print the tuple
2) value_cond		-> given a condition statement, its bound, equality condition, decide if a string value satisfies that condition

 -------------------
| Additional Notes: |
 -------------------
- For finding tuples with index, we always set ranges first by looking at all conditions in the vector.  We always assume
that only >= or <= is used.  For example, if a condition is set as "key > 10" we set lower bound range as "key >= 11".  Similarly
with "key < 100" we set upper bound range as "key <= 99".  That way when we are actually looking for tuples using index, we can 
directly use locate.

- When setting upper/lower bounds for values, we have a boolean condition that remembers the kind of comparison that we have.  For example
if condition is set as value < 'abcd' AND value >= 'a', then u_vBound = 'abcd' and l_vBound = 'a' with isUpperBoundEq set to true (<) and 
isLowerBoundEq set to true (>=) 

- < takes precedence over <= because in upperBound, we want a smaller bound
- > takes precedence over >= because in lowerBound, we want a larger bound

 ---------------
| Notes: Part 2C|
 ---------------

Assumption: Location of root and height of tree is stored in page 0 
	    : When treeHeight is 1, then the root node is a leafNode
		otherwise if treeHeight is more than 1, then root is always a nonLeafNode

##################################
# Page 0 of BTreeIndex PageFile: #
##################################

Byte 0-3 Pid of root
Byte 4-7 Height of tree

All the other pages in this pagefile are basically nodes in the tree (either BTLeafNode or BTNonLeafNode)

 ------------------------
| Newly added functions: |
 ------------------------

1) getRootPid		-> returns the pid of root node
2) getTreeHeight		-> returns the height of the tree
3) getParentPid		-> given a node, returns pid of parent
4) insert_in_parent	-> when a node is split, call this function 
5) locate_helper		-> given a key value, return pid of the node that contains a value greater than or 
				   equal to key

Additional proxy functions
1) print_node		-> prints information about the nodes such as pointers 
2) test_readForward	-> goes from the first entry of leftmost leaf node to the last entry of right most node

 -------------------
| Additional Notes: |
 -------------------

- pid of root and height of tree is updated when the BTreeIndex is closed.
- insert takes care of 2 special cases: when the index is new and when height is 1
- insert_in_parent is a recursive function, it helps insert function by taking care
of inserting new item to parent and subsequent upward cascade.


 ---------------------------
| General Structure: Part 2B|
 ---------------------------

Both leaf and non leaf nodes are represented as a 1024 bytes Char array.  For now, 
the first 20 bytes are reserved for additional information that we may need for later
parts.  

##############
# Leaf Node: #
##############

Byte 0 - Node Type
Byte 1 - Padding
Byte 2 - Padding
Byte 3 - Padding

byte  4-7  - Entry count

byte  8-11 - pointer to parent (pageID)
byte 12-15 - pointer to self   (pageID)

Byte 20-23 - Page ID
Byte 24-27 - Slot ID
Byte 28-31 - Key 

Byte 32-35 - Page ID 
Byte 36-39 - Slot ID
Byte 40-43 - Key

Byte 1020-1023 - pointer to next node (pageID)

#################
# NonLeaf Node: #
#################

Byte 0 - Node Type
Byte 1 - Padding
Byte 2 - Padding
Byte 3 - Padding

byte  4-7  - Entry count

byte  8-11 - pointer to parent (pageID)
byte 12-15 - pointer to self   (pageID)

Byte 20-23 - Page ID of child < Key1

Byte 24-27 - Key1  
Byte 28-31 - Page ID of child >= Key1 

Byte 32-35 - Key2
Byte 36-39 - Page ID of child >= Key2

 -------------------
| Additional Notes: |
 -------------------

Note: constant value FIRST_ENTRY_OFFSET in both BTLeafNode and BTNonLeafNode class is used to move pointer 
to the first byte after proxy information storage, which for now is set to 20 bytes.

Note when iterating entries in a node for BTNonLeafNode:
insert		 -> pointer points to key in a (key, pointer) pair
locateChildPointer -> pointer points to pid

Note when splitting nodes:
	for insertAndSplit in BTLeafNode	 sibling pointer have one more key value in odd case
	for insertAndSplit in BTNonLeafNode  sibling node have one less key value in odd case

 ------------------------
| Newly added functions: |
 ------------------------

1) Constructor for BTLeafNode
2) Constructor for BTNonLeafNode
3) getBuffer() for both BTLeafNode and BTNonLeafNode
4) printContent() for both BTLeafNode and BTNonLeafNode
5) getter and setter functions for pointers to parent pid and self pid for both BTLeafNode and BTNonLeafNode
6) on BTreeNode.cc
   #include <iostream>
   #include <string>


 ----------------
| Notes : Part 2A|
 ----------------

First create/open table name as specified in the argument
for load.  Then use fstream to open the file name which is 
first extracted and stored as an array of characters.  Once
the file is opened, then extract each line by using getline 
command, and then use parseLoadLine function of SqlEngine.  
After we extract (key,value) pair from each line, append
to the end of table by using append function.  Finally 
close both the file and the record.
 -------------------
| General Structure |
 -------------------

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

Byte 1021-1024 - pointer to next node (pageID)

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
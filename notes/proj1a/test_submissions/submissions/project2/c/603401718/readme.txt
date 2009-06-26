David Hwang
603401718

Adjustments to BTreeNode.cc and BTreeNode.h were made to interface better
with BTreeIndex.  BTreeNode was modified such that all memory for the node
would be allocated from the start.  Fixed bug where siblingKey wasn't 
being returned correctly.  Other cosmetic changes may have occured.

BTreeIndex.cc has some bugs, but should work for small cases.  Still need
some work on it.

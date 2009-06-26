Richard Van	
003498073

Kimberly Hsiao
303450164

Part 2A was straightforward after you understand the Bruinbase architecture.  I wrote some pseudo-code to load,
read, and close files then implemented the code.  

Part 2B was pretty hard mainly because we weren't sure how to test the functionality of our BTreeNode classes. 
Also, it was hard to differentiate between what a BTLeafNode took care of and a BTNonLeafNode did.  
We implemented our BTreeNodes thinking about BTreeIndex and how that will use the nodes to create the index.

Part 2C was hard because of debugging.  One little error would mess up the whole tree, but we were able to implement
each function and isolate it and then test it with our test cases.  The hardest thing was to write test cases
that would thoroughly test our BTIndex and BTNodes.  We learned a lot about how the pagefile worked to read/write
disk and it was quite enjoyable.  

NOTE that we used 1-grace day for this part of the project
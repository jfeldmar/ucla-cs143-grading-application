Peter Peterson
203462520 - pedro@tastytronic.net
Project 2C README

PLEASE NOTE:

While I made some minor changes to BTreeNode.h/cc, I made a VERY SMALL change that affected many lines; specifically, I added a macro so that I could quickly turn on and off debugging print statements without affecting a large number of lines. To do so, I had to add "DEBUG" to many fprintf statements. I hope that these "changes" won't affect the number of changed lines of code between my parts B, C, and the future D. Thanks.

CODE PROBLEM:

There is a slight problem with the PageFile library and the way the lab is cut up. Since the lab does not specifically explain how the node buffer is to be defined, the student must figure this out for themselves. This in and of itself is fine. However, the student may chose (for debugging purposes) to use a smaller buffer to hold fewer keys (for testing). Unfortunately, the PageFile read/write functions memcpy PAGE_SIZE amount of memory, even if the pointer handed to them is only (say) 128 bytes. This results in overwriting variables on the stack.

If the buffer were included in the lab, students would at least know what the underlying subsystem expects.

For my Part C submission, I have designed a largely recursive system.

My design includes all necessary information inside the nodes on disk through the use of special values, flags, etc.

In particular:

* I use a supernode, which is pid 0 in the file; this points to the current root node.
* The root node, which first initialized, points to two pages (as per the spec), but carries a special key flag, value '-1010'. This will be replaced by the first real key inserted into the root node.
* When BTreeIndex::insert is called, it calls insertInto, which takes a key, rid, and target node to insert into, using the rootpid as the target node. This will recursively drill down through the tree until it hits a leaf node.
* while drilling through the tree, the node.mypid member is set; this is not stored on disk.
* leaf nodes are marked with the flag -2 in the sid of the last rid.
* after the key is inserted in the leaf node, the return values from insertInto specify whether a new child key needs to be inserted into the previous level.





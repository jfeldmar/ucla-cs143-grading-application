Justin Meza
503355401

I implemented most of the crucial functions in Part B so in Part C I tied them together to complete the B-tree indexing backend.

Implementing locate was the easiest as most of the work was deligated to the locate functions in the BT*Node classes.

insert took a little longer to code but went smoothly because of the insertAndSplit functions.

readForward will be useful for returning ranges of keys and simply involved a locate and a read to the node.

Justin Meza
503355401

I modified the BTreeNode.* files and implemented node insertion using some helper functions to aid my design. I created a new Pair structure within the classes to manipulate (key, *) data. I then used helper functions getPair and setPair to read/write pair data from memory buffers stored within each class.

For BTNonLeafNode::insertAndSplit I was unsure whether to leave the middle node in the second tree for this exercise or to remove it. I ended up deciding to remove the node because it will make implemention entire tree insertion easier as I can just skip this step later on.

Peter Peterson
203462520
pedro@tastytronic.net
Project 2B Readme

Not much here worth noting, except that my node format is as follows:

The nodes are made of an array of struct nodeEntry which is a RecordId followed by an int (key).

The array looks like this:

[RecordId Key RecordId Key ... RecordId Key]

The final nodeEntry is special; the "dangling" Key holds the "key count" for the node in both leaf nodes and non-leaf nodes. 

In leaf nodes, the final RecordId.sid is set to -2 when the node is a leaf node, since there should never be a slotID -2. The "next leaf pointer" is stored in the final RecordId.pid.

In Non-leaf nodes, there is a need to store a pointer to the child node that represents the part of the tree containing "keys greater than the last key in the current node" -- this is the "last pointer" of a non-leaf node. In my implementation, this "last pointer" is always stored in the final RecordId.pid, even if the related key is earlier in the node. In this case, there is "free space" between the last key and the last pointer, like so:

[RecordId Key1 RecordId Key2 ... ... RecordId.pid Key]

In this case, RecordId.pid points to the node with keys greater than Key2. If a key is inserted in between Key2 and RecordId.pid, the pointers are adjusted properly.

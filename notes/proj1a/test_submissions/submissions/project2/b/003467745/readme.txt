CS 143
Project 2B

README

We implemented the nodes with 127 keys and 128 pointers in each one. The non leaf nodes are organized in the typical pointer, key, pointer, key pattern and has a 4 byte gap at the end  of the node. The leaf nodes have a 4 byte gap 4 bytes from the end since we chose to set the pointer to the next sibling block at the end for ease of access. Other than that the functions are implemented as described in the file.
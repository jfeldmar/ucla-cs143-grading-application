CS143 Project 2A
 
Alfred Heu, 903549582, elf777@cs.ucla.edu
SeungHoon Lee, 403450559, shlee@cs.ucla.edu

As a team, we did pair programming and tested/verified the logics together.

Functionalities
We implement B+ tree nodes: leaf node, and non-leaf node.
Based on the size of a page(1KB), we set the max number of keys as 85.
Both two node tyeps maintain two data structures.
1)keys[]: contains the key in a node
2)pageIds[](records[]): the pointers to child node or actual data location stored

Each functions performs the functionalities described in the header file

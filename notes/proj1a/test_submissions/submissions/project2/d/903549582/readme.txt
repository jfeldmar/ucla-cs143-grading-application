CS143 Project 2D
 
Alfred Heu, 903549582, elf777@cs.ucla.edu
SeungHoon Lee, 403450559, shlee@cs.ucla.edu

As a team, we did pair programming and tested/verified the logics together.

Functionalities
We completely implement Bruinbase database system with B+ tree index.
We store tree info. in the first page of the page file. (like pageId=0)
Each functions performs the functionalities described in the header file.

12/01/2008
We assume that every key in our tree is non-negative.
However, today we realize that we also have to implement the negative key in our B++ tree
and we have some fundamental reasons why it is so hard to change our 'BTreeIndex' and 'BTreeNode' to work for the negative key in a day.
Thus, we'd like to submit the version which is working well in the non-negative key.

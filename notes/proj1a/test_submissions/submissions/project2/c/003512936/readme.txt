-- Names: 
	Angel Darquea - 003512936
	Erick Romero - 803507035
-- Date: 11/23/2008
-- Fall 2008
-- Project 2B and 2C - README

--2B
- We splitted the work by node classes.
- We pathched a newly created node with -1 as its pids so that later we could
determine the number of entries on the node.
- Our nonleaf implementatiojn will waste 4 bytes per node.

We found a bug in 2B in which we did not check if a node was empty prior to 
inserting so insert function was returning when doing so.

--2C
- Cant implement (yet) overflow cases for nonleafs. Debugging.

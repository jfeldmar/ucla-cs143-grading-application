CS 143
PROJECT 2C

README FILE

For the BTreeIndex class we implemented the function insert to determine what to do on a case by case basis. There are five seperate cases that are explained at the top of the function. The cases are designed so that they can overflow into the next one if necessary. For instance, the case that handles leaf overflow inserts and splits into the leaf node but then passes its return values to the non-leaf overflow case if its parent node is full. The non-leaf overflow case can then in turn pass its return values to the root overflow case if it traverses all the way to find a full root node.

Some parts used for debugging were put inside of if statement blocks controled by a variable called DEBUG defined at the top of the BTreeNode.h file. If it is set to 1 then debugging is on and if its set to 0 then debugging is off. It is currently in the off position.

We added 2 additional helper functions to each class in the BTreeNode file which allow us to print out the contents of each type of node to a file.

We also modified the BTreeNode file so that the size of the nodes is changeable. This helped greatly when testing since we could use low values such as 2 for the maximum number of keys.

We implemented the SELECT function of the SqlEngine by checking for any conditions useful by the B+ Tree and then either setting a range and using the B+ Tree to return the values or if no conditions applied, returning tuples the usual way.
================================================================================
PROJECT 2 PART A README
================================================================================

For the first part of our project, we implemented the LOAD command by 
simply opening a file stream, using getline() to retrieve each CSV of the file,
and then parsed it using the parseLoadLine() commadn. We did run into a few
negligible problems setting up the ifstream and found that we had to make sure
that we broke early from the loop if parseLoadLine failed. To test our
implementation, we first ran the following SELECT statements:

   SELECT COUNT(*) FROM Movie; -- Sanity checking
   SELECT COUNT(*) FROM Movie2; 

Then afterwards, we diffed the binary *.tbl files that were produced from the LOAD
statement and found that they were equal.

================================================================================
PROJECT 2 PART B README
================================================================================

For the second part of our project, we decided to stick with the classes
provided to us in BTreeNode.h to avoid major issues with inheritance; while many
of the functions relating to leaf nodes and non-leaf nodes were *similar*, they
were not similar enough for a common parent class to be useful in this case. Our
implementation has several features that should help improve performance in the
later parts of this project:

=> The maximum number of entries per node is automatically calculated at runtime
   and stored in const members of BTLeafNode and BTNonLeafNode. This allow us to
   maximize the space used by each node in a page depending on our page size.
=> Instead of iterating directly through the array of entries in a node, we use
   a binary search that takes on average log(n) time to complete. In testing, we
   found that this search took about 3-4 iterations in the worst case, compared
   to 50-100 when we use a for loop.
=> We also use a general insertion function to speed up insertion time; the
   function makes use of the binary search function we defined earlier and
   allows us to cut iteration time by about half. Both this function and the
   binary search function use an interface similar to that of the library
   function qsort(), which allowed us to easily transition from one mode of
   insertion to the other.

We also added a few features to help us exhaustively debug node creation and
insertion:

=> We added a drop() function to each class to clear out the entries in a node.
=> We added print() functions to each class to print the current key count of
   the node, the first pointer of the node (in the case of nonleaf) or the pointer
   to the next node (in the case of leaf), and the entries in the node.
=> Finally, we created two new files for testing, test.cc and test.h, with 
   debugging functions that main() redirects to if BTREENODE_DEBUG is #defined.

The most difficult part of this project was debugging and testing the functions;
after constructing helper functions to aid us, the process became a lot easier
and the development time a lot faster as a result.

David Hwang
603401718

Not carefully accounting for memory, many changes were
made to the code to correctly account for memory
allocation.  Each node now allocates 1024 bytes, the size
of a page.  Also as std::cout/cerr were not working correctly
during debugging, fprintf statements had to be used.
However, algorithmically, the tree index works the same as
it did before.

Changes to BTreeNode:
Replaced use of STL sort with cstdlib qsort
Changed std::cout/cerr to fprintf statements
Instead of storing the extraneous data at the
  beginning of the buffer; now stored at the
  end of the buffer.
In locate, when no valid entry found, eid is set to -1
Added a function which frees the buffer's memory

Changes to BTreeIndex:
Changed std:cout/cerr to fprintf statements
Remembers to free the memory of the nodes

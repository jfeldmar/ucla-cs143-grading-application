Richard Van	
003498073

Kimberly Hsiao
303450164

Part 2A was straightforward after you understand the Bruinbase architecture.  I wrote some pseudo-code to load,
read, and close files then implemented the code.  

Part 2B was pretty hard mainly because we weren't sure how to test the functionality of our BTreeNode classes. 
Also, it was hard to differentiate between what a BTLeafNode took care of and a BTNonLeafNode did.  
We implemented our BTreeNodes thinking about BTreeIndex and how that will use the nodes to create the index.

Part 2C was hard because of debugging.  One little error would mess up the whole tree, but we were able to implement
each function and isolate it and then test it with our test cases.  The hardest thing was to write test cases
that would thoroughly test our BTIndex and BTNodes.  We learned a lot about how the pagefile worked to read/write
disk and it was quite enjoyable.  
NOTE that we used 1-grace day for this part of the project


Part 2D was more difficult than we had expected. It was a good thing we started early because we had to think about
our implementation strategy for a while before we figured out an efficient way to parse the conditions. In addition, 
we struggled a lot with debugging and found that we had to change a lot of things in Parts 2B and 2C. We found 
several arithmetic bugs that caused our tree to be created improperly or caused our results to cut off before 
we intended them too. However, in the end we were able to debug all of our code and create a working program! :)

We may have changed many lines of code from the previous submissions, but only because we found several bugs and 
had lots of lines of testing that we commented out for our final submission. Even though we made various changes, 
it should not be a change of over 50% because we completed this project incrementally and tested each part in 
isolation.  

We learned a lot about disk and memory access during this project. We know that in the real world we will never
have to implement a B+Tree index, but going through this procedure showed us the power of using an index. 


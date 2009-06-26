Student's Name: Ho Ching Lam
ID: 703464017
email: ionlam2000@ucla.edu


Student's Name: Chi Kin Cheang
ID: 203615429
email: sunnybeef@gmail.com



Project2

Part A: 
Implementation of "load" functioon in SqlEngine.cc

Part B:
Lam did the Leaf node
Cheang did the NonLeaf node

Part C:
Lam did root and NonLeaf split
Cheang did Leaf split
we together did some locate function, insertion and
made a tree printing functions for debugging

Part D:
Modify the Select and Load function in SqlEngine to support indexing.
We finished part D and did the debugging together.
We used 84 for the n.

All the demo test cases have been tested without any problem.
And we did some other test case such as:
SELECT * FROM xlarge where key > -100000 and key < 100000 and key <> 100 and key <> 272 and key < 10 and key > 0
SELECT * FROM xlarge where key <> 10 and key <> 272 and key > 100 and key < 10 and key = 100
...

everythings works fine.
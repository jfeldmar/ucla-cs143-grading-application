seung-hyo choi
003-484-434
seunghyo@ucla.edu


readme.txt for 2a, 2b
--------

this was a really time-consuming project. wow. for this project, i largely worked 
with tarun solanki and sendie hudaya.

the only thing worth mentioning is the structures of the non/leafnodes:

	leaf node:
		metadata(16) | recordid(8) | key(4) | recordid(8) | key(4) | ... |

	non-leaf node:
		metadata(16) | pageid(4) | key(4) | pageid(4) | key(4) | ... |

i added one function for debugging: get_buffer that returns a pointer to an internal 
array of characters. there are four functions for each class: get/set parent pid and 
get/set its own pid.

finally, there are nodetypes saved at *(buffer+0). non-leaf = 0 while leaf = 1. 


readme.txt for 2c
--------

for project 2c, i worked once again on the project with sendie hudaya and 
tarun solanki. for the most part, collaboration was performed within the 
scope of testing the index. 

the index takes care of all test cases provided in test.sh and also utilizes 
the index for <> cases when conditions on keys allow for efficient look-up in 
the index.

one caveat for this is the fact that the program is limited in working with 
multiple <> cases. the array of the size is limited to 10 conditions which is 
a reasonable limitation. the odds that a query has 10 <> conditions on either 
key or value is small enough to not worry about it. the fact that a query 
with <> conditions along with other conditions on key has to search through 
this array decreases efficiency by a small amount.



readme.txt for 2d
--------

support for negative keys has been added to the implementation. all types of queries 
should work and the index is somewhat optimized based on input, such as detecting 
empty sets, whether to use the index, etc. the key count for each node is 80, 
meaning that for even a table such as xlarge, the height is only around 3. 

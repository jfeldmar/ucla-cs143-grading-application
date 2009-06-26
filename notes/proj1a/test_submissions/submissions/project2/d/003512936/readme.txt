CS 143 PROJECT 2D, Fall 2008

Names:
	Angel Darquea
	SID: 003512936
	angeldarquea@ucla.edu
	
	Erick Romero
	SID: 803507035
	erickrom@ucla.edu

Notes:
- Our implementation worked for all the test cases provided. We do, however, see some 
excesive page reads when rading a value from the xlarge table.
- We had to go back to our previous submissions to address bugs that surfaced once we
loaded and queried the test tables. One particular bug that required a bit of thinking
was found in the way we where handling the PageIDs. Another one that was a bit tricky to 
address was one in which our root was getting filled with duplicates.
- Most of the additional queries we tested our implementation with seemed to work, however
queries when using inequality operators in the xlarge table(and not ranges using conjunctions) read excesive
pages as well. This seems to be due to our range handling in the select function. With more time
we would have decomposed our implementation of the BTIndex class to accuartely handle big input tables.


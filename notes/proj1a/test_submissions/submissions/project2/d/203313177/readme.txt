Gregory Gay
203313177
gag43214@ucla.edu

Known Errors:
It can only return the next tuple GT or LT for a range query.
For LE and GE it only returns the key value.
Doesn't handle splits/overflows properly.
Loading with index for xlarge.del doesn't work.
It returns 1 more than the number of pages read.
I know that the indexing and tree work, but that my select function
	doesn't work correctly.
...
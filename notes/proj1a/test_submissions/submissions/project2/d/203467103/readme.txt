Matthew Pham
203467103
mattvpham@gmail.com

Project 2D

Seems to be working. I tested on xsmall and xlarge.

The '.idx' and '.tbl' files have to be deleted before each LOAD,
or there will be duplication issues.

Using the index increases the number of reads by a lot but seems
to lower the amount of time spent on the hard disk.

When there is a '<>' clause, the SqlEngine will simply scan the
entire table.


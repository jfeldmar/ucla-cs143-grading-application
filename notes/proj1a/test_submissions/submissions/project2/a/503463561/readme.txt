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

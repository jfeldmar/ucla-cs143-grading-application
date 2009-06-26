Justin Meza
503355401

I simply opened the input file and read lines from it until they were exhausted. I used `parseLoadLine' to parse the input line and the `RecordFile::*' functions to add records to the table.

Finally, I ran the statement and did a diff of the sample table the the one my program generated -- there were no differences.

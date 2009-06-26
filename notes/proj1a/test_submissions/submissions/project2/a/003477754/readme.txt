First create/open table name as specified in the argument
for load.  Then use fstream to open the file name which is 
first extracted and stored as an array of characters.  Once
the file is opened, then extract each line by using getline 
command, and then use parseLoadLine function of SqlEngine.  
After we extract (key,value) pair from each line, append
to the end of table by using append function.  Finally 
close both the file and the record.
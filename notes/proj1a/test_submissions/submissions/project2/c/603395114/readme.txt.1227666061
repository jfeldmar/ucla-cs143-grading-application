Stephen Oakley
603395114
smokinoak@ucla.edu

PROJECT 2

------ A ------
The load function has been modified to support the SQL LOAD 
query.  In order to implement the functionality, I have chosen
to us an ifstream for the input file.  

The file is read a single line at a time using the getline
function provided by std::string.  Once the line is read,
it is parsed by SqlEngine::parseLoadLine and then submitted
to the RecordFile via RecordFile::append.  

All errors are noted as they occur thus allowing partial
loads of tables from a file.  If the load file cannot be
opened the new table will not be created.

------ B ------
The BTreeNode.h file implement a binary search alond with 
comparison operators for both the leaf record and nonleaf 
record types.  By doing this insertion and lookups are 
simple binary search and a shift for insertion.  The read
and writes are done using structs that are of size
PageFile::PAGE_SIZE.  The insertAndSplit functions attempt
to maintain balanced nodes so that tree remains shallow.
There are auxilary output functions which are for debugging
purposes only.

------ C ------
Notes will come later.
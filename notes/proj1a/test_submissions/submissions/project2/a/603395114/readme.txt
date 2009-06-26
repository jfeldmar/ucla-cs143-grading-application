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
CS 143 PROJECT 2B, Fall 2008

Name: Erick Romero
UID: 803-507-035
Email: erickrom@ucla.edu

Notes:  When loading a table, my implementation will output an error message
saying: "ERROR: could not read a line" if the file has an empty line.  It will
still load all the values from the file into the table, but my implementation
checks that the parseLoadLine function does not give an error, but the way it
was given to us it will return an error when a blank line is read from the 
file.

Also, I wasn't sure what to use for the RecordId value returned from the append
function, I just created a RecordId variable and passed it, but don't use it at
all.

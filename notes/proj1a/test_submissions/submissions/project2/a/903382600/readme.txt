Name: Kenny Wong
SID: 903382600
EMAIL: nakoxathena@hotmail.com

README FOR PROJECT 2A

The algorithm for the laod function is simple.

1. Initialize variables
2. Attempt to open input file
3. Opens the table file (or creates a new one if it does not already exist)
4. Reads a line from the input file
5. Parse the line
6. Upload it into the table
7. Increment counter
8. Repeat steps 4-7 until eof of input file is reached
9. Close all data streams

The reason why the first get line exists outside of the while(!eof) and the next at the end of the loop
is because eof is not set until an attempt to read outside of the file is made.  Simply put, even if the last
character of the file is read, eof will not be set to true until an attempt is made to read past the last character.
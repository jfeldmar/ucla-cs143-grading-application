Name: Kenny Wong
SID: 903382600
EMAIL: nakoxathena@hotmail.com

README FOR PROJECT 2D

The final implemenation took longer than expected as I had significant debugging to do in my BTreeNode.cc and BTreeindex.cc files.
It turns out I had some frame smashing problems caused by incorrect pointer arithmetic i my insert functions.

The load command was easily implemented but the select command took a little more work.  It wasn't hard so much as it was tedious.
I couldn't think of any elegant work around to the original text so I used an inelegant method of coding adding lots of if symmetric if/for blocks
to check for various equality and range conditions.

I might not have cleaned up all the debug functions in the previous code which might lead to the 50% change being exceeded, hopefully that's not the case
Name: Jeremy Shaw
SID#: 403-361-577
e-mail: katachi38@hotmail.com

Notes:
project should satisfies all requirements for lab2d.
See code comments for any ambiguity.

NOTE: passing value as condition or no condition at all will use normal
      operation to search

each node is represented by a page
a page will contain a maximum of 81 entries

below is a representation of the structure of a leaf node/page

+--------------------------------------------------------------------+
|number |      |      |      |      |      |      |                  |
| of    | key1 | pid1 | sid1 | key2 | pid2 | sid2 |    .........     |
|entries|      |      |      |      |      |      |                  |
+--------------------------------------------------------------------+

non-leaf node/page will use the exact same structure except we won't be using sid's
and the first pointer that denotes < than the first key is always stored at the very start
thus, an example of a non-leaf node will look like the following with only one key entry

+--------------------------------------------------------------------+
|       |      |      |      |      |      |      |                  |
|   1   |   0  | pid1 |   0  | key1 | pid2 |   0  |    .........     |
|       |      |      |      |      |      |      |                  |
+--------------------------------------------------------------------+


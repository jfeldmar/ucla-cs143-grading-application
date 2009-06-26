Peter A. H. Peterson
pedro@tastytronic.net
203462520 
Project 2D and Post Mortem

ADDITIONS TO OLD CODE

I did need to make some changes to BTreeIndex.cc and BTreeNode.cc (and possibly .h) due to bugs in my design. Specifically, I was not properly inserting records and this was causing corruption in the tree in specific circumstances. I was forced to fix this or part D would not have turned out properly.

I may also have commented out or deleted many print statements and/or comments, both of which I use heavily in the design process.

COMPLIANCE

As far as I can tell, my program beats or nearly ties every test in test.sql. I am not aware of any significant bugs.

DESIGN

My approach for Project 2D was straightforward.

First, for INSERT, we use the BTreeIndex::insert function to insert records for all new keys.

For SELECT, we split the SELECT flow control depending on whether there is an index for the table. If not, the original code is run.

If there is an index, we open the index and set up some runtime variables we will use later, including the location of the first leaf of the tree, a cursor for reading leaves, and searchMin and searchMax variables, which are keys used to define the range of the tree that we are going to search. 

Since the conditions are all ANDed together, additional constraints can only restrict the number of possible tuples. This, in addition to the fact that the values are sorted in the tree means that we can take the operators =, <, >, =>, and <= and develop a range of the table keys to search. Likewise, we can easily short circuit some SELECTions if they include conflicting conditions.

For example, the statement:

SELECT * FROM xlarge WHERE key = 57 AND key = 60 

... is obviously going to return no results. Less obviously, we can also reject statements like this:

SELECT * FROM xlarge WHERE key < 57 AND key > 60

By keeping the searchMin and searchMax temporary variables at hand while parsing conditions, we can create a tight range to search and reject obviously meaningless queries.

Furthermore, a SELECTion like this:

SELECT * FROM xlarge WHERE key > 1000 AND key > 5000

... is really just the single (more restrictive) condition key > 5000.

If a value= or <> SELECT is used on a table with an index, this does not affect the range in any way. As a result, value= and <> SELECT statements have the potential of being much more expensive than simple key-related SELECTions.

In order to use the range reduction techniques while still searching for value= and <> conditions, we use a twofold approach. First, we read the conditions and search the index in order to establish the most restrictive potential values for searchMin and searchMax. Then, using the index again, we read through the tree and doublecheck the conditions (since this does not require extra page reads). The doublechecking will verify all comparisons such as =, >, and <, but more importantly, it will also check the value= and <> conditions. Since value= and <> will only return tuples if they are ALSO in a provided key range, we essentially numerically reduce the candidate keys and then only return those tuples that additionally satisfy the value= and <> conditions if they exist.

PERFORMANCE

This results in very good performance in general. My numbers are all better, or very close to the output.txt results. In particular, my system must have an extra page or two that is read as setup or metadata (I use a supernode, for example). This results in my page counts for small index searches being very consistently in the neighborhood of +2 page reads compared to the output.txt. I know of a few possible places where this could be optimized, but it is not worth the hassle.

However, it is notable that in the large COUNT(*), my results are in around 10x faster than the output.txt results. For COUNT(*) searches, my approach was to do the same range limiting as described above, but then merely "reading forward" through the leaves of the tree and *not* actually reading the node values. Since the tree is already sorted, I can, for purely numerical searches not using value= or <>, simply return the number of records in the leaves. Caching will keep me from re-reading the leaf every time. For COUNT(*) that are modified by value= or <> conditions, the system falls back to the simpler method where the keys are read and are analyzed for content. My results for the SELECT(*) on the medium table with 100 records, I read 6 pages instead of output.txt's 33. For large (1000 records), I read 21 pages vs. 238, and for xlarge, (12,000+) I read 218 pages vs. 1656. 



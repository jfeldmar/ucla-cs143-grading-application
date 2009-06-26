SID: 503401785
Name: Mary Qi
E-mail: maryqi@ucla.edu

Currently, my implementation of the movie database is only the most basic interface
containing 
 - 3 pages to insert people (actors and directors), comments, and movies
    pagei1.php
    pagei2.php
    pagei3.php
 - 2 pages to browse people (actors and diretors), and movies
    pageb1.php
    pageb2.php
 - 1 page for searching
    pages1.php
 - 1 page for updating movies by adding actors
    pageu1.php
There are no additional features that were added, but for ease of use, after each
insertion or update is completed, there is a link that can redirect you to the page
for the actor/director or movie that was updated.  The search range also incorporate
the directors in the database, and from each page you can get to another page
through the links at the bottom. When an invalid year in entered for table columns
that are dates, the default 0000-00-00 is entered instead. However, for ints, it
checks to make sure that it there are no alphabetic letters and an error is
returned if there is. Actor&Movie relations can be added when within a movie page
by the links that says 'Add' on the side.
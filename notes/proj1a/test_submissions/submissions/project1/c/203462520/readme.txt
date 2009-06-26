Peter A. H. Peterson
203462520
pedro@tastytronic.netPeter A. H. Peterson
203462520
pedro@tastytronic.net
README for Project 1C
CS 143 Fall 2008

COMPLIANCE:

I have met all project criteria, so far as I know.

The project spec defines 6 pages:

I1: Adding actor and/or director information.
I2: Adding comments to movies.
I3: Adding movie information.

B1: Displaying actor information, including links to movies they have been in.
B2: Displaying movie information: links to actors, average rank, all user comments, and an add comment button.

S1: Search page: search for actors, actresses, movies through a keyword search interface.

My project has 8 different pages, which combine various specification pages and add new features.

Search: PS1 -- search.php (satisfies S1)

search.php is integrated into the sidebar of all pages and allows the user to search for actors, actresses, directors, and/or movies, by searching for substrings of last, first, and title and by selecting search targets with checkboxes. For example, one can enter "Rob Reiner" for first and last name, check "actor" and "director" and then add "Star Wars" in the movie title field and hit submit. The search enging will return matching actors and directors, and any movies matching "Star Wars".

Browsing pages:

PB1 -- actor.php (satisfies B1, S1)

actor.php displays actor information, including links to movies and personal information. Like all other pages, it incorporates search.php in a sidebar.

PB2 -- movie.php (satisfies S1, B2, I2)

movie.php displays movie information, director information, average rating, and all actors and actresses in the movie. Immediately following the movie information is an "add comments" and ratings box (so there is not a separate "add comments" page), followed by all comments, listed most recent first (as is done on many blogs, etc.). Averages are calculated while comments are posted, so the first comment will update the average rating (as opposed to it not being available until subsequent views).

PB3 -- director.php (new feature, S1)

director.php displays director information, just like actor.php.

Adding Information pages:

PI1 -- editperson.php (I1)

editperson.php adds actors, actresses, or directors. It is convenient that directors have no gender, which means that we can use a single radio button group to determine the difference between actors, actresses, and directors.

PI2 -- editmovie.php (I3)

editmovie.php adds new movies to the database, and uses a drop-down menu to select from the existing genres.

PI3 -- editdirector.php (not explicitly required)

editdirector.php allows users to link existing directors with movies. This feature is not specifically required in the spec.

PI4 -- editrole.php (not explicitly required)

editrole.php allows users to associate an actor, a film, and the name of a role. This information is added to the MovieActor table, and will immediately appear in displays of the actor's profile and the related movie profile. This feature is not specifically required in the spec.

Other features:

Movietron 5000 is also somewhat good at telling users what was missing from their input, or why queries or insertions might have failed.

Furthermore, all user input is typechecked and/or escaped for safety.

Finally, I used a generalized output function, called TablePrint in the MovieTron.php library file to handle table output of tuple information. TablePrint can (optionally) create "(detail)" links to other tables and scripts. In this way, it is simple to have TablePrint create (detail) links to all movies an actor has been in.

Additionally, the MovieTron.php has a generalized error function, ErrorMsg(), which takes as an argument an error string to print before graceful termination. In any case where typechecking or SQL operations fail, the script calls ErrorMsg("Reason why script failed.") and prints out helpful information.

Finally, the search.php file and searchbar-vert.html file were include()ed into scripts, allowing for a high degree of code reuse and tight integration with other site components. As a result, most of the above pages consist simply of the necessary type checking and flow control elements.
README for Project 1C
CS 143 Fall 2008

COMPLIANCE:

I have met all project criteria, so far as I know.

The project spec defines 6 pages:

I1: Adding actor and/or director information.
I2: Adding comments to movies.
I3: Adding movie information.

B1: Displaying actor information, including links to movies they have been in.
B2: Displaying movie information: links to actors, average rank, all user comments, and an add comment button.

S1: Search page: search for actors, actresses, movies through a keyword search interface.

My project has 8 different pages, which combine various specification pages and add new features.

Search: PS1 -- search.php (satisfies S1)

search.php is integrated into the sidebar of all pages and allows the user to search for actors, actresses, directors, and/or movies, by searching for substrings of last, first, and title and by selecting search targets with checkboxes. For example, one can enter "Rob Reiner" for first and last name, check "actor" and "director" and then add "Star Wars" in the movie title field and hit submit. The search enging will return matching actors and directors, and any movies matching "Star Wars".

Browsing pages:

PB1 -- actor.php (satisfies B1, S1)

actor.php displays actor information, including links to movies and personal information. Like all other pages, it incorporates search.php in a sidebar.

PB2 -- movie.php (satisfies S1, B2, I2)

movie.php displays movie information, director information, average rating, and all actors and actresses in the movie. Immediately following the movie information is an "add comments" and ratings box (so there is not a separate "add comments" page), followed by all comments, listed most recent first (as is done on many blogs, etc.). Averages are calculated while comments are posted, so the first comment will update the average rating (as opposed to it not being available until subsequent views).

PB3 -- director.php (new feature, S1)

director.php displays director information, just like actor.php.

Adding Information pages:

PI1 -- editperson.php (I1)

editperson.php adds actors, actresses, or directors. It is convenient that directors have no gender, which means that we can use a single radio button group to determine the difference between actors, actresses, and directors.

PI2 -- editmovie.php (I3)

editmovie.php adds new movies to the database, and uses a drop-down menu to select from the existing genres.

PI3 -- editdirector.php (not explicitly required)

editdirector.php allows users to link existing directors with movies. This feature is not specifically required in the spec.

PI4 -- editrole.php (not explicitly required)

editrole.php allows users to associate an actor, a film, and the name of a role. This information is added to the MovieActor table, and will immediately appear in displays of the actor's profile and the related movie profile. This feature is not specifically required in the spec.

Other features:

Movietron 5000 is also somewhat good at telling users what was missing from their input, or why queries or insertions might have failed.

Furthermore, all user input is typechecked and/or escaped for safety.

Finally, I used a generalized output function, called TablePrint in the MovieTron.php library file to handle table output of tuple information. TablePrint can (optionally) create "(detail)" links to other tables and scripts. In this way, it is simple to have TablePrint create (detail) links to all movies an actor has been in.

Additionally, the MovieTron.php has a generalized error function, ErrorMsg(), which takes as an argument an error string to print before graceful termination. In any case where typechecking or SQL operations fail, the script calls ErrorMsg("Reason why script failed.") and prints out helpful information.

A helpful index.html has been added that forwards the client to search.php.

Finally, the search.php file and searchbar-vert.html file were include()ed into scripts, allowing for a high degree of code reuse and tight integration with other site components. As a result, most of the above pages consist simply of the necessary type checking and flow control elements.

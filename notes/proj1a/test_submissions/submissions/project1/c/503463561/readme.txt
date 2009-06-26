================================================================================
Project 1C README file
================================================================================

================================================================================
Project Criteria Met
================================================================================

Basic requirements:

I1: Add information about an actor or director. * Implemented in i1.php
I2: Add comments to movies. * Implemented in i2.php
I3: Add movie information. * Implemented in i3.php

B1: Show information about an actor or director. * Implemented in b1.php
B2: Show information about a movie. * Implemented in b2.php

S1: Search for an actor/actress/movie using keywords. * Implemented in s1.php

Additional requirements:

* Bad data entry errors are managed gracefully, and the database returns error
  messages specifying exactly why the user input was rejected.
* The same set of tables as was created in Project 1B were used in this project;
  we did not use any auxiliary tables.
* Navigation is simple: all pages in the database share a common navigation bar;
  and no iframes or frames were used in implementing this site.

================================================================================
Additional features implemented
================================================================================

* All pages on the site use the same headers, footers, and general layout,
  defined in wrapper.php.
* Instead of having a single drop-down bar containing the list of all
  actors/directors or movies on the site, which causes a lot of overhead when
  loading individual pages, we replaced it with an AJAX selector which
  dynamically loads information from the database only as needed.
* Text fields on the input pages now remember what they contain if a submission
  error occurs; thus, the end user no longer has to constantly retype data if
  he/she makes a mistake. We didn't have enough time to get this to work with
  the AJAX selector, however.
* Error messages appear next to the fields where an error has occurred, rather
  than at the top of the page, to help usability.
* Our website is valid XHTML 1.0 Transitional.

================================================================================
Collaborative Details
================================================================================

Brian Jew worked on pages i1.php, i2.php, s1.php, e1.php, and s1.php. Brian
Nguyen worked on pages b1.php, b2.php, i3.php, e2.php, index.php, wrapper.php,
ajax.php, and selector.php. Both of us worked on style.css, and general debugging. 

What worked really well on this project was our usage of a SVN repository to
manage multiple revisions and concurrent editing. SVN allowed us to easily keep
updated on the latest iteration of the project without having to resort to lots
of e-mails and confusion. The ability to revert back to previous versions, as
well, saved lots of grief in a few instances.

What we probably need to work on for the next project is making sure we have
well-defined tasks. It was easy to do so for the first part of this project
since each individual page could be worked on independently of the others, but
the second part of the project-- linking the pages together, coming up with a
common UI, etc.-- was much harder to break up into pieces that could be worked
on concurrently. Finding ways to do so for our next project will make life much
easier.

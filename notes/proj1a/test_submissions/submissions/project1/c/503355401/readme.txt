Justin Meza
503355401

For this project, I chose to create a fictional movie database site, MovieBuff, in the spirit of sites like IMDB.com. I created a design for the site first, using HTML and CSS and then added PHP code.

Searches can be initiated from any page on the site by simply typing the search query into the text box labeled "search" at the top right corner of any page and pressing enter when ready.

Three different tables are searched: Actor, Movie, and Director. In the Actor and Director tables, the first and last attributes are searched; in the Movie table, the title attribute is searched.

The search page summarizes the number of exact and related matches. A related match matches any of the search terms from the query. It is useful because if one does not know exactly what they are searching for, they can guess a few terms and let the search engine do the rest.

Actors are listed in the form: FIRST LAST. Movies are listed in the form: TITLE (YEAR). Directors are listed in the form: FIRST LAST (Director) so that they are not confused with actors.

The three different types of search categories provide hyperlinks to pages to display information about them. In addition to the information in a particular row of the database, actor pages list the movies they have acted in, movies list the director and actors in the movie, and directors list the movies they have directed and acted in. All of the previously mentioned information is provided as hyperlinks to individual pages.

Actors, directors, and movies may be added to the database. From the main page, one can click on one of the links on the right labeled "Add Actor", "Add Director", or "Add Movie" to go to the corresponding page. Once an item has been added to the database, the user is redirected to a "Thank You" page, thanking them for their contribution, and offering a link back to the main page.

On the movie description page, the average user rating of a movie is shown as its rating out of four stars under its title. A user can hover over one of the four stars and click to select what they would like to rate a movie. However, their rating will not be submitted until they enter a comment in a form at the bottom of the page. Users can also view comments near the bottom of the page.

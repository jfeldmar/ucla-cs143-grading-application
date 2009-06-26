Edward Chang
ID #603366734
etchang@ucla.edu

Project 1C



Three input pages:
* Page I1: A page that lets users to add actor and/or director information.
	- page checks to make sure all the fields are filled out except dob, First name and Last name can only be letters, Date of Birth has to be in proper YYYY-MM-DD format, or it has the proper error msg
	- also, the correct date is only inserted if the date is one that happened already, if not, the date is inserted as 0000-00-00

* Page I2: A page that lets users to add comments to movies.
	- page checks that every field is filled out before you are allowed to conitnue or it has a custom error msg

* Page I3: A page that lets users to add movie information.
	- every field needs to be filled out or custom error message is returned

* Page I4: A page that lets users add the actor/movie relation and assign the role
	- every field must be filled out or custom error message is returned

Two browsing pages:
* Page B1: A page that shows actor information.
          o DONE - Show links to the movies that the actor was in.
* Page B2: A page that shows movie information.
          o DONE - Show links to the actors/actresses that were in this movie.
          o DONE - Show average “rank” of the movie based on user comments.
          o DONE - Show all user comments.
          o DONE - Contain “Add Comment’ button which links to Page I2 where users can add comments
		NOTE: my version of add comments of i2, only submits the insert when the user has inputted all the information, which is why I simply link to the i2 page

One search page:
* Page S1: A page that lets users search for an actor/actress/movie through a keyword search interface.
	- in the results for both Actor and Movies the link to the subsequent info page is there
	- in the case of no results it is simply left blank and handled gracefully

EXTRAS:
	- made the page pretty, as well as an index.php where you can run just off the parent folder (ex. http://cs143.com/imdb/ as opposed to http://cs143.com/imdb/i1_name.php)
	- background color formatted, text formatted
	- on each page the title is a different color, a nice added touch
	- on the Actor and Movie info pages, if nothing is selected it says "No Actor or Actress Selected!" or "No Movie Selected!" respectively
	- on all the pulldown menus involving directors, actors, movies, etc., there is a friendly default selected value of "Pick a Actor/Director/Movie!"

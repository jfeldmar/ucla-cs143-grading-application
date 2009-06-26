Jimmy Nguyen
903-384-091
jnmgd@ucla.edu

The results of the queries are as follows:
_______________________________________________
Names of actors in the movie 'Die Another Day':

first		last

Halle		Berry
Judi		Dench
Rosamund	Pike
Pierce		Brosnan
John		Cleese
Paul		Darrow
Thomas		Ho
Michael		Madsen
Toby		Stephens
Rick		Yune
_______________________________________________
Count of actors who acted in multiple movies:

COUNT(*)

4824
_______________________________________________
Most popular genre of movies after the year 2000:

genre

Drama
_______________________________________________
Constraints:

The following are given:

Every movie has a unique identification number.
Every movie must have a title.
Every actor must have a date of birth.

The following are ones I've created:

Every actor must have a first and last name.
Every actor must have a sex.
Every actor has a unique identification number.
Every director has a first and last name.
Every directory must have a date of birth.
Every movie must have a genre.
Every movie must have a director.
Every movie must have at least one actor.
Every actor in a movie must have a role.
Movies must be a number between when they first existed until now.
A movie id referenced in MovieGenre must exist in Movie.
A movie id referenced in MovieDirector must exist in Movie.
A director id referenced in MovieDirector must exist in Director.
A movie id referenced in MovieActor must exist in Movie.
An actor id referenced in MovieActor must exist in Actor.
A movie id referenced in Review must exist in Movie.
Identification numbers cannot be negative.
Names cannot contain symbols and punctuation.
Dates can only contain numbers.
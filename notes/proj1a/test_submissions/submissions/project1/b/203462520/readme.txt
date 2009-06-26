CS143 Project 1B
Peter Peterson -- 203462520 -- pedro@tastytronic.net

PLEASE NOTE: I've identified more constraints here than I needed to. I added a subset to create.sql, and only "violated" 12 of those according to the directions.

Constraints that should exist in the database:

NOT NULL constraints are essentually CHECK constraints.

Databases:
Actor:

1. Every actor must have a date of birth.
2. Every actor must have a unique id. (primary key)
3. Every actor must have a sex.
4. dod must be > dob
5. id must be <= MaxPersonID

Director:

1. Every director must have a date of birth.
2. Every director must have a unique id. (primary key)
3. dod must be > dob
4. Every director must direct at least one Movie (listed in MovieDirector).

Movie:

1. Every movie has a unique id. (primary key)
2. Every movie must have a title.
3. The year for each movie must be >= the dob year for Director. (and all actors)
4. id must be <= MaxMovieID

MovieActor:

1. Every mid must correspond to an id in Movie
2. Every aid must correspond to an id in Actor
3. mid, aid must be NOT NULL (a movie w/o an actor or vice versa makes no sense)

MovieDirector:

1. Every mid must correspond to an id in Movie
2. Every did must correspond to an id in Director
3. mid, did must be NOT NULL (a movie must have a director and vice versa)


Note: it is possible that a director is unknown -- then he would simply not be listed in MovieDirector.

MovieGenre:

1. Each mid must correspond to an id in Movie
2. mid and genre must be NOT NULL

MaxMovieID:

1. MaxMovieID must be the highest value of Movie.id.
2. id must be > 0 (at the very least)

MaxPersonID:

1. MaxPersonID must be the highest value of Actor.id.
2. id must be > 0 (at the very least)

Review:

1. All reviews must have a timestamp, mid, and comment (no name is allowed for anonymity)
2. mid in review tuple must correspond to id in Movie



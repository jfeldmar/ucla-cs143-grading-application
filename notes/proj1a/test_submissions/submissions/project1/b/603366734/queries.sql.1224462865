-- this first query takes the names of the actors that were in Die ANother Day
-- it first cross references the tables of Actor, MovieActor and Movie, then it first
-- checks for an actor id with the movie id corresponding to die another day, then
-- it displays the first and last names

SELECT first, last FROM Actor, Movie, MovieActor WHERE Actor.id=MovieActor.aid AND MovieActor.mid=Movie.id AND Movie.title='Die Another Day';





-- this next query gives the count of all the actors who acted in multiple movies
-- we first take the list of all the actors who have acted in multiple movies
-- then we use that as a subquery with select count(*) making sure to rename the subquery clause (MACTOR)

select count(*) from (select first, last from MovieActor, Actor where MovieActor.aid = Actor.id having count(mid)>1) MACTOR;

-- this last query gives the title of all the movies that Pierece Brosnan has been in
-- it first checks for the first name Pierce and last name BRosnan in the Actor table,
-- then it goes to MovieActor and checks the aid with the Actor.id, then it matches the
-- MovieActor.mid with the Movie.id, finally it displays the title of the movie

SELECT title FROM Movie, Actor, MovieActor WHERE Actor.first='Pierce' AND Actor.last='Brosnan' AND Actor.id=MovieActor.aid AND Movie.id=MovieActor.mid;
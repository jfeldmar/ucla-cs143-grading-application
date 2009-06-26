
--1.Find the names of all the people who acted in 'Die Another Day'.
--SQL: Get the id of the movie, join MovieActor and Actor, and then select
--the names of the people whose id is associated with the movie.

SELECT first, last
FROM Actor, MovieActor
WHERE Actor.id=MovieActor.aid 
AND MovieActor.mid=(SELECT id FROM Movie WHERE title='Die Another Day');

--2. Find the count of all actors who acted in multiple movies.
--SQL: Count the movies with multiple actors, get the actor ids of these
--actors, and count how many.

SELECT COUNT(NUM.aid) FROM (SELECT aid FROM MovieActor GROUP BY aid HAVING COUNT(mid)>1) NUM;

--3. Find the number of actors who are also directors.
--It's a little slow.

SELECT COUNT(*) AS "Number of Actors/Directors" FROM Actor, Director
WHERE Actor.id=Director.id;

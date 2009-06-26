-- Get the names of all actors in "Die Another Day"
SELECT first, last FROM Actor, Movie, MovieActor
WHERE Actor.id = MovieActor.aid AND MovieActor.mid = Movie.id
AND Movie.title = 'Die Another Day';

-- Count how many actors have acted in more than one movie
SELECT COUNT(*) FROM
(SELECT * FROM MovieActor
GROUP BY aid
HAVING COUNT(*) > 1)
AS A;

-- Give the names and roles of actors who have portrayed doctors, and in what movies
SELECT first, last, role, title FROM Actor, MovieActor, Movie
WHERE Actor.id = MovieActor.aid AND (MovieActor.role LIKE 'Dr.%'
or MovieActor.role LIKE '%Doctor%') and Movie.id = MovieActor.mid;
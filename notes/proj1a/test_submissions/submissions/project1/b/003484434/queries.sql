-- select all actors from the movie "Die Another Day"
SELECT first, last
FROM MovieActor, Movie, Actor
WHERE Movie.title='Die Another Day'
	AND MovieActor.mid = Movie.id
	AND MovieActor.aid = Actor.id;

-- get the number of actors who've acted in 
-- multiple movies
SELECT count(*)
FROM	(SELECT aid
	FROM MovieActor
	GROUP BY aid
	HAVING count(aid) >= 2) RES;

-- get the names of people who have been both 
-- an Actor and a Director
SELECT first, last
FROM Actor
WHERE Actor.id in
	(SELECT id
	FROM Director);

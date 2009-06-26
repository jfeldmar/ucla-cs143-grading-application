-- Give me the names of all the actors in the movie 'Die Another Day'.
SELECT last, first FROM Actor
WHERE id IN (	SELECT aid FROM MovieActor 
				WHERE mid IN (	SELECT id FROM Movie
								WHERE title = 'Die Another Day'	));
--  Give me the count of all the actors who acted in multiple movies. 
SELECT COUNT(id) FROM Actor
WHERE id IN (	SELECT aid FROM MovieActor
				GROUP BY aid
				HAVING COUNT(mid) >= 2	);
-- Give the names of all people (Actors & Directors) who have not been involved in a movie.
SELECT last, first FROM Actor
WHERE id NOT IN (SELECT aid FROM MovieActor)
UNION
SELECT last, first FROM Director
WHERE id NOT IN (SELECT did FROM MovieDirector);
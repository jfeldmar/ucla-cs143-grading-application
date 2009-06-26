--The names of all the actors in the movie 'Die Another Day'.
SELECT A.last, A.First 
FROM MovieActor as M, Actor as A
WHERE M.mid in (SELECT id FROM Movie
		WHERE title = 'Die Another Day')
	AND M.aid = A.id;


--The count of all the actors who acted in multiple movies.
--We assume that no actor plays more than two roles in a movie.
SELECT COUNT(*)
FROM (SELECT aid
	FROM MovieActor
	GROUP BY aid
	HAVING COUNT(aid) >= 2
	) MA;

--The names(Last, First) of all the directors who have directed more than two movies belonging to '20th Century Fox'
SELECT Director.last, Director.first
FROM MovieDirector MD, (SELECT id FROM Movie WHERE company = '20th Century Fox') AS Fox, Director
WHERE MD.mid = Fox.id AND MD.did = Director.id
GROUP BY did
HAVING COUNT(MD.did) >= 2

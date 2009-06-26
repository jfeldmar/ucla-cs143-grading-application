-- Names of all actors in the move 'Die Another Day'
SELECT first, last
FROM Actor A, MovieActor MA, Movie M
WHERE (A.id = MA.aid AND M.id = MA.mid AND M.title = 'Die Another Day');

-- Count of all actors who acted in multiple movies.
SELECT COUNT(R.aid)
FROM (SELECT aid 
			FROM MovieActor 
			GROUP BY aid 
			HAVING COUNT(mid) > '1') R;

-- Count of all directos who directed multiple movies.
SELECT COUNT(R.did) 
FROM (SELECT did 
			FROM MovieDirector
			GROUP BY did 
			HAVING COUNT(mid) > '1') R;

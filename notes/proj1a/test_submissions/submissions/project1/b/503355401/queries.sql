-- Returns the names of all the actors in the movie ``Die Another Day''.
SELECT first, last
  FROM Actor A, MovieActor MA, Movie M
 WHERE A.id = MA.aid
   AND M.id = MA.mid
   AND M.title = 'Die Another Day';

-- Returns the count of all the actors who acted in multiple movies.
SELECT COUNT(count)
FROM Actor A, (SELECT aid, COUNT(aid) count
	FROM MovieActor MA
	GROUP BY aid) C
WHERE count > 1
	AND A.id = aid;

-- Returns all of the actors present in movies from the latest year in the
-- database, presumably the current year.
SELECT first, last
  FROM Actor A, MovieActor MA, Movie M
 WHERE MA.aid = A.id
   AND MA.mid = M.id
   AND year = (SELECT MAX(year)
                 FROM Movie);

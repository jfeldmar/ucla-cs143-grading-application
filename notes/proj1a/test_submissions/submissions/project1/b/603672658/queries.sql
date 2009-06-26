--
-- Give the names of all the actors in the movie 'Die Another Day'
--
SELECT last, first
FROM Actor A, Movie M, MovieActor MA
WHERE A.id = MA.aid AND M.id = MA.mid AND title = 'Die Another Day';

--
-- Give the count of all the actors who acted in multiple movies
--
SELECT COUNT(aid)
FROM (SELECT aid
      FROM MovieActor
      GROUP BY aid
      HAVING COUNT(DISTINCT mid) > 1) MA;

--
-- Give the names of directors who directed at least 2 movies after year 2000
--
SELECT last, first
FROM Director D
WHERE 2 <= (SELECT COUNT(*)
            FROM Movie M, MovieDirector MD
            WHERE M.id = MD.mid AND D.id = MD.did AND year > 2000);
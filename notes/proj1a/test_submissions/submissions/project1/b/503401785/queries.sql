SELECT first, last
FROM Actor A, Movie M, MovieActor MA
WHERE MA.aid = A.id AND MA.mid = M.id AND M.title = 'Die Another Day';

--Returns the first and last name of the actors who acted in the
--movie Die Another Day.


SELECT COUNT(aid)
FROM (SELECT aid FROM MovieActor GROUP BY aid HAVING COUNT(*) > 1) A;

--Returns the count of the actors who acted in more than one movie by
--getting a table of all actors who meet the requirements and then
--counting the number of tuples returned.


SELECT M.title
FROM Director D, Movie M, MovieDirector MD
WHERE D.first = 'Steven' AND D.last = 'Spielberg' AND M.id = MD.mid AND D.id = MD.did;

--Returns all movies that were directed by Steven Spielberg.
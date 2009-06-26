/*Queries*/

--The names of all the actors in the movie 'Die Another Day'.
SELECT A.last, A.first
FROM Actor A, Movie M, MovieActor MA
WHERE A.id = MA.aid AND 
      M.id = MA.mid AND 
      M.title = 'Die Another Day';
      
--The count of all the actors who acted in multiple movies.
SELECT COUNT(DISTINCT MA1.mid)
FROM MovieActor MA1, MovieActor MA2
WHERE MA1.aid = MA2.aid AND 
      MA1.mid <> MA2.mid;
      
--Find the name of the people who is actor as well as director in the same movie and that movie title .
SELECT A.last, A.first, M.title
FROM Actor A, Director D, MovieActor MA, MovieDirector MD, Movie M
WHERE A.id = MA.aid AND
      D.id = MD.did AND
      MA.mid = MD.mid AND
      A.id = D.id AND
      MA.mid = M.id;
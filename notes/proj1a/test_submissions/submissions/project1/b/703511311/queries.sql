--1. Give me the names of all the actors in the movie 'Die Another Day'
SELECT first, last 
FROM Movie M, Actor A, MovieActor MA
WHERE M.id = MA.mid AND A.id = MA.aid AND M.title = 'Die Another Day';

--2. Give me the count of all the actors who acted in multiple movies
SELECT COUNT(*) Total
FROM (
     SELECT aid
     FROM MovieActor
     GROUP BY aid
     HAVING COUNT(*) > 1) S;

--3. Give me the names of all the actors who are also a director and directed a movie he starred in, and order them in ascending order of their id
SELECT DISTINCT id, last, first
FROM Actor A, MovieDirector MD, MovieActor MA
WHERE A.id = MA.aid AND MD.mid = MA.mid AND MD.did = MA.aid
ORDER BY id ASC;
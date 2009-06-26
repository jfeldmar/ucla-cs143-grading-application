--1. Find the names of all the actors in the movie 'Die Another Day'. 
SELECT A.first, A.last
FROM Movie M, MovieActor MA, Actor A
WHERE M.title='Die Another Day' AND M.id=MA.mid AND A.id=MA.aid;

--2. Find the count of all the actors who acted in multiple movies. 
select count(*)
from 
(SELECT aid
FROM MovieActor
GROUP BY aid 
HAVING COUNT(mid) > 1) MA;

--3. Find the titles of all movies directed by directors who are dead
SELECT title
FROM Director D, Movie M, MovieDirector MD
WHERE dod IS NOT NULL AND D.id=MD.did AND MD.mid=M.id;
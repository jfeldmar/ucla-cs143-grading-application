--Tanya Gillis CS 143 Project 1B
--queries.sql

--Lists names of all the actors in the movie 'Die Another Day'.
SELECT first, last 
FROM Actor 
WHERE id IN 

(SELECT aid 
FROM MovieActor MA
WHERE MA.mid = (

SELECT id
FROM Movie M
WHERE M.title = 'Die Another Day')); 

--Counts all the actors who acted in multiple movies.
SELECT COUNT(*)
FROM (
SELECT DISTINCT AID
FROM MovieActor
GROUP BY AID
HAVING COUNT(*) > 1) A;

--Lists the names of all comedy movies.
SELECT title
FROM Movie
WHERE id IN 

(SELECT mid
FROM MovieGenre
WHERE genre='Comedy');

-- queries.sql
-- script with required queries for project1b

USE CS143;

-- Query 1:  First and Last name of all the actors in the movie 'Die Another Day'
SELECT A.first, A.last
FROM Movie M, Actor A, MovieActor MA
WHERE M.id = MA.mid AND A.id = MA.aid AND M.title = 'Die Another Day';

-- Query 2:  First, Last name, and count of all the actors who acted in multiple movies
SELECT COUNT(*) 
FROM (	SELECT aid
	FROM MovieActor
	GROUP BY aid
	HAVING COUNT(*) > 1) R;

-- Query to find all the Movies in which Halle Berry has been in
SELECT M.title
FROM Movie M, MovieActor MovAct
WHERE M.id = MovAct.mid AND MovAct.aid = (SELECT id FROM Actor WHERE first='Halle' AND last='Berry');

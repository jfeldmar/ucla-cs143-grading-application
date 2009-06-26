
------------------------------------------------------------------------------------------------
-- Query: Get the names of all the actors in the movie 'Die Another Day'
------------------------------------------------------------------------------------------------

SELECT first, last 
FROM Actor A, MovieActor MA, Movie M 
WHERE A.id = MA.aid AND M.id = MA.mid AND M.title="Die Another Day"

-- RESULTS:
-- Halle Berry
-- Judi Dench
-- Rosamund Pike
-- Pierce Brosnan
-- John Cleese
-- Paul Darrow
-- Thomas Ho
-- Michael Madsen
-- Toby Stephens
-- Rick Yune

------------------------------------------------------------------------------------------------
-- Query: Get the count of all the actors who acted in multiple movies. 
------------------------------------------------------------------------------------------------

SELECT COUNT(*) FROM
(SELECT * FROM MovieActor
GROUP BY aid
HAVING COUNT(mid) > 1) AS q

-- RESULT: 4824

------------------------------------------------------------------------------------------------
-- EXTRA QUERIES
------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------
-- Query: Get a list of last names that are shared by more than one actor
-- along with their counts
------------------------------------------------------------------------------------------------

SELECT last, COUNT(*)
FROM Actor
GROUP BY last
HAVING COUNT(*) > 1

------------------------------------------------------------------------------------------------
-- Query: Get the count of the last name that is shared by the most actors
------------------------------------------------------------------------------------------------
SELECT MAX(c)
FROM 	(SELECT last, COUNT(last) AS c
		FROM Actor GROUP BY last) q

-- RESULT: 43

------------------------------------------------------------------------------------------------
-- Query: Get each movie genre and the number of movies in that genre
------------------------------------------------------------------------------------------------
SELECT genre, COUNT(*)
FROM MovieGenre 
GROUP BY genre

-- RESULT:
-- Action	640
-- Adult	11
-- Adventure	218
-- Animation	36
-- Comedy	1048
-- Crime	276
-- Documentary	66
-- Drama	1500
-- Family	139
-- Fantasy	128
-- Horror	289
-- Musical	40
-- Mystery	122
-- Romance	447
-- Sci-Fi	250
-- Short	4
-- Thriller	769
-- War	50
-- Western	37
------------------------------------------------------------------------------------------------
-- Query: Get the last name that is shared by the most actors and its count
------------------------------------------------------------------------------------------------
SELECT last, c
FROM
(SELECT last, count(last) AS c
FROM Actor GROUP BY last) q
WHERE c = 	(SELECT MAX(c) FROM 
			(SELECT last, COUNT(last) AS c
			FROM Actor GROUP BY last) q)
			
-- RESULT: Smith, 43
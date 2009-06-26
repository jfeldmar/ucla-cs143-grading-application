-- Name: Angel Darquea
-- ID: 003512936
-- Date: 10/15/2008
-- Fall 2008
-- Project 1B - queries.sql

-- --  Give me the names of all the actors in 'Die Another Day':
--  3. Finally, get the names of the actors whose ids match the aids returned by the subquery (2)
SELECT first, last
FROM Actor, ( 
			-- 2. Now, get a list of actor ids that worked in the movie
			SELECT aid
			FROM MovieActor
			WHERE mid in (
						-- 1.Get the id of the movie 'Die Another Day'
						SELECT id
						FROM Movie
						WHERE title = 'Die Another Day'
						)
			) A -- Name resulting relation as 'A'
WHERE Actor.id = A.aid;
-- --------------------------------------------------------------------------------------------------------------------
-- -- Give me the count of all the actors who acted in multiple movies:
-- 2. Count the number of tuples returned by  the subquery (1)
SELECT COUNT(*)
FROM(
	-- 1. Get a listing of all aids that appear more than once in the MovieActor table.
	SELECT aid
	FROM MovieActor
	GROUP BY aid
	HAVING COUNT(aid) > 1
	) B;
-- --------------------------------------------------------------------------------------------------------------------
-- -- Give me the name of all female actors in Action movies since the year 2000:
-- 3. Get all female actors whose ids are in (2)
SELECT first, last
FROM Actor
WHERE sex = 'Female' AND id IN (
				-- 2. Get all actors that worked in the movies in (1)
				SELECT aid
				FROM MovieActor
				WHERE mid IN (
								-- 1. Get all ids of all Action movies
								SELECT mid
								FROM MovieGenre
								WHERE genre = 'Action'
							 )	
			);
 



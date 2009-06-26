-- Return the names of all the actors in the movie 'Die Another Day'

SELECT last, first 
FROM   Actor 
WHERE id IN (SELECT aid
 	     FROM   MovieActor
 	     WHERE  mid = (SELECT ID
			   FROM   Movie
			   WHERE  title = 'Die Another Day'));

-- Return the result of counting the number of actors acting in multiple movies

SELECT count(*)
FROM (SELECT aid, count(*)
      FROM   MovieActor
      GROUP BY aid
      HAVING COUNT(*) > 1) a;

-- Return the names, sex, dob, and dod of actors who are also directors

SELECT a.first, a.last, sex, a.dob, a.dod
FROM Director d, Actor a
WHERE d.id = a.id;
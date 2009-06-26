-- names of all the actors in the movie 'Die Another Day'

SELECT A.last, A.first 
FROM   Actor A,	(SELECT aid FROM MovieActor WHERE mid=(SELECT id FROM Movie WHERE title='Die Another Day')) E
WHERE  A.id = E.aid;

-- names of all actors who acted in multiple movies

SELECT A.last, A.first
FROM   Actor A, (SELECT X.aid FROM (SELECT aid, count(*) as number FROM MovieActor GROUP BY aid ORDER BY number) X WHERE X.number > 1) Y
WHERE  A.id = Y.aid;

-- names and birthdays of actors who has first name 'Charlie'

SELECT last, dob
FROM   Actor
WHERE  first = 'Charlie';

SELECT first, last FROM Actor WHERE id IN (SELECT aid FROM MovieActor WHERE mid IN (SELECT id FROM Movie WHERE Movie.title = 'Die Another Day'));
--actors in Die Another Day, works VERY VERY slow in my own query,php, but works completely fine in the 1B demo
SELECT COUNT(DISTINCT aid) FROM MovieActor;
--actors who acted in multiple movies
SELECT first, last FROM Actor WHERE id<1000 AND sex='Male' AND id IN (SELECT aid FROM MovieActor WHERE mid IN (SELECT id FROM Movie WHERE Movie.rating='R'));
--query for male actors that have been in R rated movies whose id's are less than 1000
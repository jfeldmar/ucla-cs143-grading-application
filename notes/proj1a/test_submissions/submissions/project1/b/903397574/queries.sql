--Query 1: The names of all actors who acted in Die Another Day
SELECT last, first FROM Actor WHERE id IN (SELECT aid FROM MovieActor WHERE mid = (SELECT id FROM Movie WHERE title = 'Die Another Day'));

--Query 2: The count of all actors who have acted in multiple movies
SELECT COUNT(id) FROM Actor WHERE id IN (SELECT aid FROM MovieActor GROUP BY aid	HAVING COUNT(mid) > 1);

--Query 3: Finding Directors who have directed multiple movies

SELECT COUNT(id) FROM Director WHERE id IN (SELECT did FROM MovieDirector GROUP BY did	HAVING COUNT(mid) > 1);
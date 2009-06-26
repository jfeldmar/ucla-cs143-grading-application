--find the name of all the actors in the movie Die Another Day
SELECT DISTINCT last, first
FROM Actor A, (	SELECT aid
		FROM MovieActor
		WHERE mid = ( 	SELECT id
				FROM Movie
				WHERE title = 'Die Another Day' ) ) B
WHERE A.id = B.aid;

--find the count of all the actors who acted in multiple movies
select count(*)
from (	select distinct aid
	from MovieActor	
	group by aid
	having count(aid) > 1) T;

--find the number of actors
select count(*)
from Actor;
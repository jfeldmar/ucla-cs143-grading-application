--Give me the names of all the actors in the movie 'Die Another Day'.
select first, last
from Actor A, (select aid
from MovieActor MA, Movie M
where M.title = "Die Another Day" AND M.id = MA.mid) DAD
where A.id = DAD.aid;

--Give me the count of all the actors who acted in multiple movies.
select count(*) MultipleMovieActors from (
	select aid
	from MovieActor
	group by aid
	having count(*) > 2
	) A;

--Give me the names of all the directors that have directed films with 'Tom Cruise' in them	
SELECT DISTINCT First, Last
FROM Director D, MovieDirector MD
WHERE D.id = MD.did AND MD.mid IN (
	SELECT mid
	FROM MovieActor MA
	WHERE MA.aid = (
		SELECT id
		FROM Actor 
		WHERE first = 'Tom' AND last = 'Cruise'
		)
	);

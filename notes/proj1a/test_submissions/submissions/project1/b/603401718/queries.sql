--QUERY ONE
--We do a join of Actor and Movie Actor where Actor.id = MovieActor.aid
--Then the other condition is that the Movie's id should be equal to the
--id in the Movie table which stands for 'Die Another Day'

SELECT last, first
FROM Actor A, MovieActor M
WHERE A.id = M.aid AND M.mid = (
	SELECT id 
	FROM Movie
	WHERE title='Die Another Day'
);

--QUERY TWO
--Count the # of Distinct entries of actor ids
--where there exists two different actor entries
--which have the same aid yet different mids
--so this must equal actors w/ more than 1 movie
SELECT COUNT(DISTINCT A.aid)
FROM MovieActor A, MovieActor B
WHERE A.aid = B.aid && A.mid > B.mid;

--QUERY THREE
--This query list all the movies Tom Hanks were in
--First there is a joining of Movie and Movie Actor
--and from there we choose the actor ids which follow the
--specified condition...having Tom Hank's aid.
SELECT title
FROM Movie M, MovieActor A
WHERE M.id = A.mid AND A.aid = (
	SELECT id
	FROM Actor
	WHERE last = 'Hanks' AND first = 'Tom'
);
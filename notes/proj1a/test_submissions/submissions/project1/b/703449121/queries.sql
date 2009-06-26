-----------
--Find the actors who appeard in Die Another Day
-----------
SELECT last, first
FROM MovieActor MA, Movie M, Actor A
WHERE mid=M.id AND M.title='Die Another Day' AND aid=A.id; 

-----------
--Find the count of actors who appeared in multiple movies
-----------
SELECT COUNT(DISTINCT MA1.aid)
FROM MovieActor MA1, MovieActor MA2
WHERE MA1.mid < MA2.mid AND MA1.aid=MA2.aid;
 
-----------
--Find the average age of actors and directors named Jeff or Jeffrey
-----------

SELECT first, AVG(age) as AverageAge
FROM (  SELECT last, first, year(dod)-year(dob) AS age
	FROM Actor WHERE first like 'Jeff%' AND dod IS NOT NULL
	UNION
	SELECT last, first, year(dod)-year(dob) AS age
	FROM Director WHERE first like 'Jeff%' AND dod IS NOT NULL
     ) as ActorsAndDirectors
GROUP BY first;

SELECT A.last, A.first
FROM Actor A, Movie M, MovieActor MA
WHERE A.id = MA.aid AND M.id = MA.mid AND M.title ='Die Another Day';

SELECT COUNT(*) 
FROM ( SELECT aid 
       FROM MovieActor
       GROUP BY aid
       HAVING COUNT(*) > 1 ) MMA;

SELECT A.last, A.first 
FROM Actor A
WHERE (year(dod)-year(dob) < 18 ); 
--This query tells us the actors who died before age of 18.

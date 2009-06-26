-- Use Appropriate Database
USE CS143;

-- Give me the names of all the actors in the movie 'Die Another Day'
SELECT A.last, A.first
FROM Actor A, MovieActor MA, Movie M
WHERE A.id = MA.aid AND MA.mid = M.id AND M.title = 'Die Another Day'
GROUP BY A.id;

-- Give me the count of all the actors who acted in multiple movies
SELECT count(*)
FROM (SELECT count(aid) AS count_aid
      FROM MovieActor
      GROUP BY aid) AS aidCountList
WHERE aidCountList.count_aid > 1;

-- Give me the last and first names of all the actors who are also directors
SELECT A.last, A.first
FROM Actor A
WHERE EXISTS (SELECT *
              FROM Director D
              WHERE D.id = A.id)
GROUP BY A.id;

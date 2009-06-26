-- Names of all the actors in the movie 'Die Another Day'
SELECT A.first, A.last
FROM Actor A, (SELECT aid
               FROM MovieActor
               WHERE mid = (SELECT id
                            FROM Movie
                            WHERE title = 'Die Another Day')) B
WHERE A.id = B.aid;

-- Count of all the actors who acted in multiple movies
SELECT count(DISTINCT T1.aid)
FROM MovieActor T1, MovieActor T2
WHERE T1.aid = T2.aid AND T1.mid <> T2.mid;

-- Find out the years which had more than 400 movies released
SELECT year
FROM Movie
GROUP BY year
HAVING COUNT(year) > 400;

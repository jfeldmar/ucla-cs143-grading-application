-- Names of actors in the movie 'Die Another Day'
SELECT first, last FROM Movie AS t1, MovieActor AS t2, Actor as t3 WHERE t1.title = 'Die Another Day' AND t1.id = t2.mid AND t3.id = t2.aid;

-- Count of actors who acted in multiple movies
SELECT COUNT(*)
FROM (SELECT COUNT(*) FROM MovieActor GROUP BY aid HAVING COUNT(*) > 1) AS c1;

-- Most popular genre of movies after the year 2000
SELECT genre FROM (SELECT genre, COUNT(*) AS Frequency FROM MovieGenre AS mg, (SELECT id FROM Movie WHERE year > 2000) as m WHERE mg.mid = m.id GROUP BY genre) as m2 WHERE Frequency = (SELECT Max(Frequency) FROM (SELECT genre, COUNT(*) AS Frequency FROM MovieGenre AS mg, (SELECT id FROM Movie WHERE year > 2000) as m WHERE mg.mid = m.id GROUP BY genre) as m3);
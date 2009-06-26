-- Give me the names of all the actors in the movie 'Die Another Day'. 
SELECT first, last
FROM Actor
WHERE id IN ( SELECT aid
              FROM MovieActor
              WHERE mid = ( SELECT id
                             FROM Movie
                             WHERE title = 'Die Another Day' ) );

-- Give me the count of all the actors who acted in multiple movies. 
SELECT Count(*)
FROM ( SELECT aid
        FROM MovieActor
        GROUP BY aid
        HAVING Count(aid) > 1 ) MA;

-- My Query: Give me the count of all the directors who directed more than one movie.
SELECT Count(*)
FROM ( SELECT did
        FROM MovieDirector
        GROUp BY did
        HAVING Count(did) > 1 ) MD;


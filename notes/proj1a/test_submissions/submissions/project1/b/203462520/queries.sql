-- CS143 - Project 1B
-- Peter A. H. Peterson -- pedro@tastytronic.net
-- 203462520

-- 1. Names of all actors in 'Die Another Day'

SELECT first, last -- select first and last names
FROM Actor A,  -- from a JOIN of Actor as A
 (SELECT *  -- and all tuples from MovieActor
  FROM MovieActor 
  WHERE mid =  -- where the MovieId
   (SELECT id  -- is the id
    FROM Movie  -- of the movie whose title is
    WHERE title = 'Die Another Day' -- Die Another Day
   ) 
  ) B 
WHERE A.id = B.aid; -- where the Actor ids match the actors from the movie

-- 2. Count of all actors who act in multiple movies.

SELECT COUNT(*) -- count the number of tuples resulting from this query:
FROM 
 (SELECT DISTINCT aid, COUNT(*) as count -- select distinct actor ids and count tuples
  FROM MovieActor 
  GROUP BY aid -- ordered by actor id, so count the number of movies each actor is in
 ) ActorCount 
WHERE count > 1; -- but only keep actors who are in more than one movie

-- 3. Another Query That I Came Up With: "What are the names of all the actors who have acted with Kevin Bacon in a movie?"

SELECT DISTINCT A1.first,A1.last -- select the firstname, lastname, and movie title
FROM Actor A1, -- from a JOIN of Actor,
MovieActor MA1, -- MovieActor as MA1
 (SELECT mid  -- and this table which is all the movie ids
  FROM MovieActor 
  WHERE aid = ( -- where the actor id
  SELECT id     -- is the id
  FROM Actor    
  WHERE first='Kevin' AND last='Bacon' -- from the Actor table belonging to Kevin Bacon
  )
 ) KBM -- renamed as KBM for KevinBaconMovies
WHERE MA1.mid = KBM.mid AND A1.id = MA1.aid; -- where other actors are in the same movies as Kevin Bacon



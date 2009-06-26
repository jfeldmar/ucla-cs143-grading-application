--Gives the names of all the actors in the movie 'Die Another Day'. 
SELECT Actor.first, Actor.last
FROM MovieActor, Movie, Actor
WHERE Movie.id = MovieActor.mid AND Movie.title = 'Die Another Day' AND MovieActor.aid = Actor.id;

--Give me the count of all the actors who acted in multiple movies. 
SELECT Count(*)
FROM Actor, (
SELECT aid, Count(*) > 2
FROM MovieActor
GROUP BY aid
HAVING Count(*) > 2) S 
WHERE S.aid = Actor.id;


--Actors in the movie Deadly Voyage
SELECT Actor.*
FROM Actor, MovieActor, (
SELECT id
FROM Movie
WHERE title = 'Deadly Voyage') S
WHERE MovieActor.mid = S.id AND MovieActor.aid = Actor.id; 
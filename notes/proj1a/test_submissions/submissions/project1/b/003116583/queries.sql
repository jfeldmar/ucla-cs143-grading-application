SELECT Actor.last, Actor.first FROM Movie, MovieActor, Actor WHERE (Movie.id = MovieActor.mid && Actor.id = MovieActor.aid && Movie.title = 'Die Another Day');

SELECT MovieActor.aid FROM MovieActor HAVING COUNT(*) >1;

/* The below line gets the ID of all the actors who have acted in multiple movies and it shows how many movies they were in*/

SELECT aid, count(*)  Number_of_Movies FROM MovieActor GROUP BY aid HAVING COUNT(*) > 1;

-- The names of all the actors in the movie 'Die Another Day'
SELECT		last, first
FROM		Actor, Movie, MovieActor
WHERE		Actor.id=MovieActor.aid AND Movie.id=MovieActor.mid
			AND title='Die Another Day';

-- The count of all the actors who acted in multiple movies
SELECT count(*)
FROM	(	SELECT 		id, count(*)
		FROM 		Actor, MovieActor
		WHERE 		Actor.id=MovieActor.aid
		GROUP BY	id
		HAVING		count(*) > 1	) C;

-- The count of movies for a given year
SELECT 		year, count(*)
FROM		Movie
GROUP BY	year;

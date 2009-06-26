-- query.sql: Run a few SELECT statements to test the database --

-- Query 1: Give me the names of all the actors in the movie "Die Another Day."

SELECT last, first 
	FROM Actor, MovieActor, Movie
	WHERE Movie.id = MovieActor.mid AND
			Actor.id = MovieActor.aid AND
			Movie.title = 'Die Another Day';

-- Query 2: Give me the count of all the actors who acted in multiple movies.

SELECT COUNT(DISTINCT MA1.aid)
	FROM MovieActor MA1, MovieActor MA2
   WHERE MA1.aid = MA2.aid AND
	  		MA1.mid != MA2.mid;

-- Query 3: Give me the titles of all the action movies that are rated G.

SELECT title
	FROM Movie, MovieGenre
	WHERE Movie.id = MovieGenre.mid AND
			Movie.rating = 'G' AND
			MovieGenre.genre = 'Action';

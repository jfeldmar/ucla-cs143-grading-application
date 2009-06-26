SELECT first, last FROM Actor,Movie,MovieActor WHERE Actor.id = MovieActor.aid AND Movie.id = MovieActor.mid AND Movie.title ="Die Another Day";
/*Get the cartesian product of Actor,Movie,MovieActor and get the first and last names of the tuples where movie actor id's match with 
the movie and actor id's for the movie title 'Die Another Day'. The result is the first and last names of the actors who appeard in the movie 
Die Another Day.*/

SELECT COUNT(aid) FROM (SELECT aid FROM MovieActor GROUP BY aid HAVING COUNT(*) > 1) AS S; 
/* first, the subquery groups all unique actor id's from the MovieActor that appear more than once and puts them into table S. Next, the outer query counts all 
of these actor id's from S and comes up the the final value of 4829 actors who have appeared in more than one movie.*/   

SELECT first,last FROM Actor,(SELECT aid FROM MovieActor GROUP BY aid HAVING COUNT(*) > 20) AS S WHERE Actor.id=S.aid;
/* first, the subquery groups all unique actor id's from the MovieActor that appear more than twenty times and puts them into table S. Next, we take the cartesian product 
of S and the Actor table. Finally, the outer query matches the the actor id from S with first and last names of the actors in the Actor table. The result ia 
a list of actor first and last names who have appeared in more than 20 movies.*/ 
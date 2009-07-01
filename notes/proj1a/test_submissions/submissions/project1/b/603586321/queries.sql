SELECT A.last, A.first FROM Actor A, Movie M, MovieActor WHERE mid = M.id AND aid = A.id AND M.title = 'Die Another Day'; -- Returns the names of all the actors in the movie 'Die Another Day'.
SELECT COUNT(*) FROM (SELECT aid FROM MovieActor GROUP BY aid HAVING COUNT(DISTINCT mid) > 1) N; -- Returns the count of all the actors who acted in multiple movies.
SELECT A.last, A.first, N1.count FROM Actor A, (SELECT aid, COUNT(DISTINCT mid) count FROM MovieActor GROUP BY aid) N1 WHERE A.id = N1.aid AND N1.count >= (SELECT MAX(N2.count) FROM (SELECT aid, COUNT(DISTINCT mid) count FROM MovieActor GROUP BY aid) N2); -- Returns the name of the actor who has acted in the most movies and the number of movies he has acted in.
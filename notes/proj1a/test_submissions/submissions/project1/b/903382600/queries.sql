--Query 1: Finds the First Name and Last Name of actors in the movie 'Die Another Day'
--Starts by finding the title of the movie then looking up the A

SELECT first, last
FROM Movie M, Actor A, MovieActor X
WHERE M.title = 'Die Another Day' AND M.id = X.mid AND A.id = X.aid;

--Query 2: Finds the count of all the actors who acted in multiple movies
--Counts all instances of AID in the MovieActor table then subtracts the unique AID (that is actors who have only acted in one movie) thus returning a list of actors who have been in multiple movies

SELECT count(aid) - count(distinct aid)
FROM MovieActor;

--Query 3: Find the roles and movies that Brad Pitt has acted in.
--Finds the actor's AID then looks up the Role in the MovieActor table along with the corresponding Movie title received by comparing the movie ID in the MovieActor table against the Movie table.

SELECT MA.role, M.title
FROM MovieActor MA, Movie M, Actor A
WHERE A.first='Brad' AND A.last='Pitt' AND A.id=MA.aid AND MA.mid=M.id;
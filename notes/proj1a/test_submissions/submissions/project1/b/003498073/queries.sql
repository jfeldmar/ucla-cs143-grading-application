-- queries.sql
-- Richard Van

-- The following select statement will give the names of all the actors in the movie "Die Another Day"
select first,last from Actor where id in (
	select aid from MovieActor where mid = (
		select id from Movie where title = "Die Another Day"))
		
/* These were the results I got
Halle	Berry
Pierce	Brosnan
John	Cleese
Paul	Darrow
Judi	Dench
Thomas	Ho
Michael	Madsen
Rosamund	Pike
Toby	Stephens
Rick	Yune
*/	
-- This query works by finding all actors whoses ids are in the movie, the movie itself corresponds
-- to an id.



-- The following select statement will give the count of all the actors who acted in multiple movies

	
select count(*) from Actor where id in(
	select aid from MovieActor a group by aid having count(*) >= 2 )
	
-- This is the result -  4824
-- Sometimes this query takes a really long time
-- The main part of this query works by comparing all the ids of actors in the Actor table to the ids 
-- of people in more than one movie.




-- This is my query I came up with, it finds all the movies Peter Jackson has directed

select title from Movie where id in (
	select mid from MovieDirector where did = (
		select id from Director where last = "Jackson" AND first = "Peter"))
		
-- The Results are
/*Frighteners, The
Heavenly Creatures
Lord of the Rings: The Fellowship of the Ring, The
Lord of the Rings: The Two Towers, The
*.
-- We can see that this data is old, since he has directed more movies since The Two Towers
-- This query works by finding the titles whose id matches the id of the director with the name Peter Jackson
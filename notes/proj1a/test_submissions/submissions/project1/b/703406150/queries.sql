-- names of all the actors in the movie 'Die Another Day'
select distinct a.first, a.last
from Movie m, Actor a
where a.id IN
(select ma.aid
from Movie m, MovieActor ma
where m.title="Die Another Day" AND m.id=ma.mid);

-- count of all the actors who acted in multiple movies
select count(*) as Total
from (select count(*) as Movies
	  from MovieActor ma
	  group by ma.aid) m
where Movies > 1;

-- names of all the sci-fi movies that start with E
select m.title AS '"E" Sci-Fi Movies'
from Movie m, MovieGenre g
where m.id=g.mid AND m.title like "E%" AND g.genre="Sci-Fi";

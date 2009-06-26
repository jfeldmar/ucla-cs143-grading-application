
-- actors who acted in the movie die another day

select first,last
from Actor
where id in (
		select ma.aid
		from Movie m, MovieActor ma
		where m.id = ma.mid and m.title = 'Die Another Day' );


-- returns the number of actors that acter in multiple movies

select count(*)
from (
	select aid, count(*)
	from MovieActor
	group by aid
	having count(*) > 1) s;	

-- returns the id first, last names and movie titles for all the actors who
-- acted in a R rated Movie

select a.id, a.first, a.last, s.title
from Actor a, (select ma.aid id, m.title title
from Movie m, MovieActor ma
where m.id = ma.mid
and m.rating = "R") s
where a.id = s.id
group by a.id;
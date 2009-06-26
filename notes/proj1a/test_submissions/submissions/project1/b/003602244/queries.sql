-- show all the names(last, first) of the actors in the movie 'Die Another Day'
select last, first
from Actor A, MovieActor MA, Movie M
where M.id = MA.mid and A.id = MA.aid and M.title = 'Die Another Day'
group by last, first;

-- show the number of count of all the actors who acted in multiple movies
select count(*)
from (
select MA1.aid, MA1.mid
from MovieActor MA1, MovieActor MA2
where MA1.aid = MA2.aid and MA1.mid > MA2.mid
group by MA1.aid
having count(MA1.mid)>1
) P ;

-- show all the Horror movie that is rated R and ordered by year.
select year, title, rating, company 
from MovieGenre MG, Movie M
where MG.mid = M.id and MG.genre = 'Horror' and M.rating = 'R'
group by M.year;

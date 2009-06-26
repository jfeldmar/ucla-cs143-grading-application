-- select the first and last names of actors who were in the movie 'Die Another Day'
select first, last from (select aid from (select * from Movie where title='Die Another Day') MA, MovieActor where MA.id = mid) A, Actor where Actor.id = A.aid;
-- return the number of actors who have starred in more than one movie
select count(*) from (select aid from MovieActor group by aid having count(*) > 1) A;
-- my query:
-- select the first and last name of actors who have starred in 'Horror' films
select first, last from (select aid from (select mid from MovieGenre where genre='Horror') MG, MovieActor where MovieActor.mid=MG.mid group by aid having count(*)>1) A, Actor where A.aid=Actor.id;

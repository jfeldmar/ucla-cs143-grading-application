
#---------------------------------------
# Cody Prestwood 303596543
# 18 oct 2008
#
# query.sql contains queries for the databse tables:
#
# Movie(id, title, year, rating, company)
# Actor(id, last, first, sex, dob, dod)
# Director(id, last, first, dob, dod)
# MovieGenre(mid, genre)
# MovieDirector(mid, did)
# MovieActor(mid, aid, role)
# Review(name, time, mid, rating, comment)
#---------------------------------------

#  query the names of all the actors in the movie 'Die Another Day'. 
select last,first from Actor A,Movie M,MovieActor where title = 'Die Another Day' and M.id = mid and A.id = aid;

# query the count of all the actors who acted in multiple movies
select count(aid) from MovieActor group by aid having count(aid) > 1;

# what are the names of directors that that directed comedy movies
select distinct first,last from Director,MovieDirector M,MovieGenre G where genre="Comedy" and id=did and M.mid=G.mid;

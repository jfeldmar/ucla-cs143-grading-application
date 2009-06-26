select last,first from Movie m, MovieActor ma, Actor a WHERE m.id=ma.mid AND a.id=ma.aid AND title='Die Another Day';
--This query selects 3 databases: Movie, Actor and MovieActor.  The point of the three database is to connect
--Movie and Actor by using the database MovieActor and this is done by matching the ids.  Then I can proceed to
--apply any conditions I want


select count(*) from (select * from MovieActor ma GROUP BY aid HAVING count(mid)>1) mult
--This query basically takes MovieActors and groups them together based on their actor ids and does a check to see
--which actors have acted in more than 1 movie.  I then take the count from that query.


select a.last,a.first,m.title from MovieActor ma,MovieDirector md,Actor a,Movie m where ma.aid=md.did AND ma.mid=md.mid AND a.id=ma.aid AND m.id=md.mid
--This query finds all movies which have a Director who also acts in the movie
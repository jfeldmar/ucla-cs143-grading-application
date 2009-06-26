USE CS143

-- Initial Values
insert into MaxPersonID values (69000);
insert into MaxMovieID values (4750);

-- Movie Table
LOAD DATA LOCAL INFILE './data/movie.del' INTO TABLE Movie
FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';

-- Actor Table
LOAD DATA LOCAL INFILE './data/actor1.del' INTO TABLE Actor
FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';
LOAD DATA LOCAL INFILE './data/actor2.del' INTO TABLE Actor
FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';
LOAD DATA LOCAL INFILE './data/actor3.del' INTO TABLE Actor
FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';

-- Director Table
LOAD DATA LOCAL INFILE './data/director.del' INTO TABLE Director
FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';

-- MovieGenre Table
LOAD DATA LOCAL INFILE './data/moviegenre.del' INTO TABLE MovieGenre
FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';

-- MovieDirector Table
LOAD DATA LOCAL INFILE './data/moviedirector.del' INTO TABLE MovieDirector
FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';

-- MovieActor Table
LOAD DATA LOCAL INFILE './data/movieactor1.del' INTO TABLE MovieActor
FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';
LOAD DATA LOCAL INFILE './data/movieactor2.del' INTO TABLE MovieActor
FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';

-- Review Table: No Loadfile


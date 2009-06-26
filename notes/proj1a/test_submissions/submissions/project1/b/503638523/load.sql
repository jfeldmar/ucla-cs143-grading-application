LOAD DATA INFILE '/home/cs143/www/data/actor1.del' INTO TABLE Actor FIELDS TERMINATED BY ',' ENCLOSED BY '"';
LOAD DATA INFILE '/home/cs143/www/data/actor2.del' INTO TABLE Actor FIELDS TERMINATED BY ',' ENCLOSED BY '"';
LOAD DATA INFILE '/home/cs143/www/data/actor3.del' INTO TABLE Actor FIELDS TERMINATED BY ',' ENCLOSED BY '"';
LOAD DATA INFILE '/home/cs143/www/data/movie.del' INTO TABLE Movie FIELDS TERMINATED BY ',' ENCLOSED BY '"';
LOAD DATA INFILE '/home/cs143/www/data/director.del' INTO TABLE Director FIELDS TERMINATED BY ',' ENCLOSED BY '"';
LOAD DATA INFILE '/home/cs143/www/data/movieactor1.del' INTO TABLE MovieActor FIELDS TERMINATED BY ',' ENCLOSED BY '"';
LOAD DATA INFILE '/home/cs143/www/data/movieactor2.del' INTO TABLE MovieActor FIELDS TERMINATED BY ',' ENCLOSED BY '"';
LOAD DATA INFILE '/home/cs143/www/data/moviedirector.del' INTO TABLE MovieDirector FIELDS TERMINATED BY ',' ENCLOSED BY '"';
LOAD DATA INFILE '/home/cs143/www/data/moviegenre.del' INTO TABLE MovieGenre FIELDS TERMINATED BY ',' ENCLOSED BY '"';

INSERT INTO MaxPersonID VALUES(69000);
INSERT INTO MaxMovieID VALUES(4750);

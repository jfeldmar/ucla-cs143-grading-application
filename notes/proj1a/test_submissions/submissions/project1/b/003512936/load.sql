-- Name: Angel Darquea
-- ID: 003512936
-- Date: 10/15/2008
-- Fall 2008
-- Project 1B - load.sql

-- -- Load movie.del file into Movie table:
LOAD DATA LOCAL INFILE './data/movie.del' INTO TABLE Movie FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';
-- --------------------------------------------------------------------------------------------------------------------
-- -- Load actor1.del, actor2.del, actor3.del files into Actor table:
LOAD DATA LOCAL INFILE './data/actor1.del' INTO TABLE Actor FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';
LOAD DATA LOCAL INFILE './data/actor2.del' INTO TABLE Actor FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';
LOAD DATA LOCAL INFILE './data/actor3.del' INTO TABLE Actor FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';
-- --------------------------------------------------------------------------------------------------------------------
-- -- Load director.del file into Director table:
LOAD DATA LOCAL INFILE './data/director.del' INTO TABLE Director FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';
-- --------------------------------------------------------------------------------------------------------------------
-- -- Load moviegenre.del file into MovieGenre table:
LOAD DATA LOCAL INFILE './data/moviegenre.del' INTO TABLE MovieGenre FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';
-- --------------------------------------------------------------------------------------------------------------------
-- -- Load moviegenre.del file into MovieDirector table:
LOAD DATA LOCAL INFILE './data/moviedirector.del' INTO TABLE MovieDirector FIELDS TERMINATED BY ',';
-- --------------------------------------------------------------------------------------------------------------------
-- -- Load moviegenre.del file into MovieActor table:
LOAD DATA LOCAL INFILE './data/movieactor1.del' INTO TABLE MovieActor FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';
LOAD DATA LOCAL INFILE './data/movieactor2.del' INTO TABLE MovieActor FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';
-- --------------------------------------------------------------------------------------------------------------------
-- -- Load (69000) into MaxPerson table:
INSERT INTO MaxPersonID VALUES(69000);
-- --------------------------------------------------------------------------------------------------------------------
-- -- Load (69000) into MaxMovieID table:
INSERT INTO MaxMovieID VALUES(4750);
-- --------------------------------------------------------------------------------------------------------------------
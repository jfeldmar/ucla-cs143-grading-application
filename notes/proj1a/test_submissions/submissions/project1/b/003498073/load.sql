/* Loads all tuples into the tables
Loads the data from ./data directory
*/

LOAD DATA LOCAL INFILE './data/actor1.del'
INTO TABLE Actor
FIELDS
	TERMINATED BY ','
	ENCLOSED BY '"'
(id, last, first, sex, dob, dod);

LOAD DATA LOCAL INFILE './data/actor2.del'
INTO TABLE Actor
FIELDS
	TERMINATED BY ','
	ENCLOSED BY '"'
(id, last, first, sex, dob, dod);
	
LOAD DATA LOCAL INFILE './data/actor3.del'
INTO TABLE Actor
FIELDS
	TERMINATED BY ','
	ENCLOSED BY '"'
(id, last, first, sex, dob, dod);
	
LOAD DATA LOCAL INFILE './data/director.del'
INTO TABLE Director
FIELDS
	TERMINATED BY ','
	ENCLOSED BY '"'
(id, last, first ,dob, dod);

LOAD DATA LOCAL INFILE './data/movie.del'
INTO TABLE Movie
FIELDS
	TERMINATED BY ','
	ENCLOSED BY '"'
(id, title, year, rating, company);

LOAD DATA LOCAL INFILE './data/movieactor1.del'
INTO TABLE MovieActor
FIELDS
	TERMINATED BY ','
	ENCLOSED BY '"'
(mid, aid, role);

LOAD DATA LOCAL INFILE './data/movieactor2.del'
INTO TABLE MovieActor
FIELDS
	TERMINATED BY ','
	ENCLOSED BY '"'
(mid, aid, role);

LOAD DATA LOCAL INFILE './data/moviedirector.del'
INTO TABLE MovieDirector
FIELDS
	TERMINATED BY ','
	ENCLOSED BY '"'
(mid, did);

LOAD DATA LOCAL INFILE './data/moviegenre.del'
INTO TABLE MovieGenre
FIELDS
	TERMINATED BY ','
	ENCLOSED BY '"'
(mid, genre);

/* Insert the tuples to MaxPersonID and MaxMovieID */
INSERT INTO MaxPersonID 
	VALUES(69000);
	
INSERT INTO MaxMovieID
	Values(4750);
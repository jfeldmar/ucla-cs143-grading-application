CREATE TABLE Movie(
	id INT, 
	title VARCHAR(100), 
	year INT, 
	rating VARCHAR(10), 
	company VARCHAR(50),
	PRIMARY KEY(id) -- Primay Key Constraint: Every movie has a unique id number.
	) ENGINE = INNODB;
CREATE TABLE Actor(
	id INT, 
	last VARCHAR(20), 
	first VARCHAR(20), 
	sex VARCHAR(6), 
	dob DATE, 
	dod DATE,
	PRIMARY KEY(id) -- Primay Key Constraint: Every actor has a unique id number.
	) ENGINE = INNODB;
CREATE TABLE Director(
	id INT, 
	last VARCHAR(20), 
	first VARCHAR(20), 
	dob DATE, 
	dod DATE,
	PRIMARY KEY(id) -- Primay Key Constraint: Every director has a unique id number.
	) ENGINE = INNODB;
CREATE TABLE MovieGenre(
	mid INT, 
	genre VARCHAR(20),
	FOREIGN KEY(mid) REFERENCES Movie(id) -- Referential Integrity Constraint: Tuples must have references to existing movie id's.
	) ENGINE = INNODB;
CREATE TABLE MovieDirector(
	mid INT, 
	did INT,
	FOREIGN KEY(mid) REFERENCES Movie(id), -- Referential Integrity Constraint: Tuples must have references to existing movie id's.
	FOREIGN KEY(did) REFERENCES Director(id) -- Referential Integrity Constraint: Tuples must have references to existing director id's.
	) ENGINE = INNODB;
CREATE TABLE MovieActor(
	mid INT, 
	aid INT, 
	role VARCHAR(50),
	FOREIGN KEY(mid) REFERENCES Movie(id), -- Referential Integrity Constraint: Tuples must have references to existing movie id's.
	FOREIGN KEY(aid) REFERENCES Actor(id) -- Referential Integrity Constraint: Tuples must have references to existing actor id's.
	) ENGINE = INNODB;
CREATE TABLE Review(
	name VARCHAR(20), 
	time TIMESTAMP, 
	mid INT, 
	rating INT, 
	comment VARCHAR(500),
	FOREIGN KEY(mid) REFERENCES Movie(id), -- Referential Integrity Constraint: Tuples must have references to existing movie id's.
	CHECK(rating >= 0 AND rating <= 5) -- Rating must be between 0 and 5.
	) ENGINE = INNODB;
CREATE TABLE MaxPersonID(
	id INT,
	CHECK(id >= 0) -- The max ID nubmer assigned so far is greater than 0,
	) ENGINE = INNODB;
CREATE TABLE MaxMovieID(
	id INT,
	CHECK(id >= 0) -- The max ID nubmer assigned so far is greater than 0,
	) ENGINE = INNODB;
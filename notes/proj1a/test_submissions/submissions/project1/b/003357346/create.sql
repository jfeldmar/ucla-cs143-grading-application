CREATE TABLE Movie(
	id INT NOT NULL,
	title VARCHAR(100) NOT NULL,
	year INT NOT NULL,
	rating VARCHAR(10) NOT NULL,
	company VARCHAR(5) NOT NULL,

	-- Primary Key Constraint #1 ensure that id is unique
	PRIMARY KEY(id),

	-- Check Constraint #1 check that the id is not negative
	CHECK (id>0)
	
	) ENGINE = INNODB;

CREATE TABLE Actor(
	id INT NOT NULL, 
	last VARCHAR(20) NOT NULL, 
	first VARCHAR(20) NOT NULL,
	sex VARCHAR(6) NOT NULL,
	dob DATE NOT NULL, 
	dod DATE, 

	-- Primary Key Constraint #2 ensure that id is unique
	PRIMARY KEY(id),

	-- Check Constraint #2 check that the id is not negative
	CHECK (id>0)
	) ENGINE = INNODB;

CREATE TABLE Director(
	id INT NOT NULL, 
	last VARCHAR(20) NOT NULL, 
	first VARCHAR(20) NOT NULL, 
	dob DATE NOT NULL, 
	dod DATE, 

	-- Primary Key Constraint #3 ensure that id is unique
	PRIMARY KEY(id),

	-- Check Constraint #3 check that the id is not negative
	CHECK (id>0)
	) ENGINE = INNODB;

CREATE TABLE MovieGenre(
	mid INT NOT NULL,
	genre VARCHAR(20) NOT NULL, 

	-- Referential Constraint #1 - mid references id from Movie
	-- if Movie(id) is deleted/changed, delete/change our tuples 
	FOREIGN KEY(mid) REFERENCES Movie(id)
	ON DELETE CASCADE ON UPDATE CASCADE
	
	) ENGINE = INNODB;

CREATE TABLE MovieDirector(
	mid INT NOT NULL,
	did INT NOT NULL,

	-- Referential Constraint #2 - mid references id from Movie
	-- if Movie(id) is deleted/changed, delete/change our tuples 
	FOREIGN KEY(mid) REFERENCES Movie(id) 
	ON DELETE CASCADE ON UPDATE CASCADE,

	-- Referential Constraint #3 - did references id from Director
	-- if Director(id) is deleted/changed, delete/change our tuples 
	FOREIGN KEY(did) REFERENCES Director(id)
	ON DELETE CASCADE ON UPDATE CASCADE

	) ENGINE = INNODB;

CREATE TABLE MovieActor(
	mid INT NOT NULL,
	aid INT NOT NULL,
	role VARCHAR(50) NOT NULL, 

	-- Referential Constraint #4 - mid references id from Movie
	-- if Movie(id) is deleted/changed, delete/change our tuples 
	FOREIGN KEY(mid) REFERENCES Movie(id)
	ON DELETE CASCADE ON UPDATE CASCADE,

	-- Referential Constraint #5 - aid references id from Actor
	-- if Actor(id) is deleted/changed, delete/change our tuples 
	FOREIGN KEY(aid) REFERENCES Actor(id)
	ON DELETE CASCADE ON UPDATE CASCADE

	) ENGINE = INNODB;

CREATE TABLE Review(
	name VARCHAR(20), 
	time TIMESTAMP NOT NULL, 
	mid INT NOT NULL, 
	rating INT, 
	comment VARCHAR(500), 

	-- Referential Constraint #6 - mid references id from Movie
	FOREIGN KEY (mid) REFERENCES Movie(id)
	ON DELETE CASCADE ON UPDATE CASCADE

	) ENGINE = INNODB;

CREATE TABLE MaxPersonID (
	id INT NOT NULL
	) ENGINE = INNODB;

CREATE TABLE MaxMovieID (
	id INT NOT NULL
	) ENGINE = INNODB;

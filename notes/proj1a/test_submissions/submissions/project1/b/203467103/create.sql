-- create.sql

-- SUGGESTED CONSTRAINTS
-- Every movie must have a title (IMPLEMENTED)
-- Every actor must have a date of birth (IMPLEMENTED)
-- Every actor must have a first name (IMPLEMENTED)
-- Each director must have a date of birth (IMPLEMENTED)
-- Each director must have a first name (IMPLEMENTED)
-- KEYS
-- Every movie has a unique identification number (IMPLEMENTED)
-- Every actor has a unique identification number (IMPLEMENTED)
-- Every director has a unique identification number (IMPLEMENTED)
-- REFERENTIAL INTEGRITY
-- Every MovieActor entry must correspond with an existing actor (IMPLEMENTED)
-- Every MovieActor entry must correspond with an existing movie (IMPLEMENTED)
-- Every MovieDirector entry must correspond with an existing director (IMPLEMENTED)
-- Every MovieDirector entry must correspond with an existing movie(IMPLEMENTED)
-- Every MovieGenre entry must correspond with an existing movie (IMPLEMENTED)
-- Every Review entry must correspond with an existing movie (IMPLEMENTED)
-- CHECKS
-- An actor cannot be in a movie before their birth
-- An actor cannot be in a movie after their death
-- An actor cannot die before they were born (IMPLEMENTED)
-- A director cannot die before they were born (IMPLEMENTED)
-- A review can only be made during or after the movie's creation
-- Every actor must have been in at least one movie
-- Every director must have directed at least one movie
-- A movie's year cannot be in the future (IMPLEMENTED)
-- 

DROP TABLE IF EXISTS
Movie, Actor, Director,
MovieGenre, MovieDirector,
MovieActor, Review,
MaxPersonID, MaxMovieID;

CREATE TABLE Movie (
	id	INT NOT NULL,
	title	VARCHAR(100) NOT NULL,
	year	INT,
	rating	VARCHAR(10),
	company	VARCHAR(50),
	PRIMARY KEY(id),
	CHECK(year <= YEAR(CURDATE()))
) ENGINE=INNODB;
-- Every movie has a unique identification number
-- Every movie must have a title
-- A movie's year cannot be in the future

CREATE TABLE Actor(
	id	INT NOT NULL,
	last	VARCHAR(20),
	first	VARCHAR(20) NOT NULL,
	sex	VARCHAR(6),
	dob	DATE NOT NULL,
	dod	DATE,
	PRIMARY KEY(id),
	CHECK(dob < dod OR dod IS NULL)
) ENGINE=INNODB;
-- Every actor has a unique identification number (IMPLEMENTED)
-- Every actor must have a first name (IMPLEMENTED)
-- Every actor must have a date of birth (IMPLEMENTED)
-- An actor cannot die before they were born (IMPLEMENTED)

CREATE TABLE Director(
	id	INT NOT NULL,
	last	VARCHAR(20),
	first	VARCHAR(20) NOT NULL,
	dob	DATE NOT NULL,
	dod	DATE,
	PRIMARY KEY(id),
	CHECK(dob < dod OR dod IS NULL)
) ENGINE=INNODB;
-- Each director must have a first name (IMPLEMENTED)
-- A director cannot die before they were born (IMPLEMENTED)
-- Each director must have a date of birth (IMPLEMENTED)
-- Every director has a unique identification number (IMPLEMENTED)
-- A director cannot die before they were born (IMPLEMENTED)


CREATE TABLE MovieGenre(
	mid	INT,
	genre	VARCHAR(20),
	FOREIGN KEY (mid) REFERENCES Movie(id)
) ENGINE=INNODB;
-- Every MovieGenre entry must correspond with an existing movie (IMPLEMENTED)

CREATE TABLE MovieDirector(
	mid	INT,
	did	INT,
	FOREIGN KEY (mid) REFERENCES Movie(id),
	FOREIGN KEY (did) REFERENCES Director(id)
) ENGINE=INNODB;
-- Every MovieDirector entry must correspond with an existing director (IMPLEMENTED)
-- Every MovieDirector entry must correspond with an existing movie(IMPLEMENTED)

CREATE TABLE MovieActor(
	mid	INT,
	aid	INT,
	role	VARCHAR(50),
	FOREIGN KEY (mid) REFERENCES Movie(id),
	FOREIGN KEY (aid) REFERENCES Actor(id)
) ENGINE=INNODB;
-- Every MovieActor entry must correspond with an existing actor (IMPLEMENTED)
-- Every MovieActor entry must correspond with an existing movie (IMPLEMENTED)

CREATE TABLE Review(
	name	VARCHAR(20),
	time	TIMESTAMP,
	mid	INT,
	rating	INT,
	comment VARCHAR(500),
	FOREIGN KEY (mid) REFERENCES Movie(id)
) ENGINE=INNODB;
-- Every Review entry must correspond with an existing movie (IMPLEMENTED)

CREATE TABLE MaxPersonID(
	id	INT
) ENGINE=INNODB;

CREATE TABLE MaxMovieID(
	id	INT
) ENGINE=INNODB;

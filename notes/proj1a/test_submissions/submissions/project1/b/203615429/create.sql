/*Create Table*/
/*Chi Kin Cheang ~ 203-615-429*/

CREATE TABLE Movie (
	id INT NOT NULL COMMENT 'Movie ID',
	title VARCHAR(100) NOT NULL COMMENT 'Movie title',
	year INT NOT NULL COMMENT 'Release year',
	rating VARCHAR(10) NOT NULL COMMENT 'MPAA rating',
	company VARCHAR(50) NOT NULL COMMENT 'Production company',
	PRIMARY KEY(id),
	/*id in Movie must not greater than the id in MaxMovieID.*/
	CHECK(id <= ALL (SELECT id FROM MaxMovieID))
) ENGINE = innodb;

CREATE TABLE Actor (
	id INT NOT NULL COMMENT 'Actor ID',
	last VARCHAR(20) NOT NULL COMMENT 'Last name',
	first VARCHAR(20) NOT NULL COMMENT 'First name',
	sex VARCHAR(6) NOT NULL COMMENT 'Sex of the actor',
	dob DATE NOT NULL COMMENT 'Date of birth',
	dod DATE COMMENT 'Date of death',
	PRIMARY KEY(id),
	/*id in Actor must not greater than the id in MaxPersonID.*/
	CHECK(id <= ALL (SELECT id FROM MaxPersonID)),
	/*sex can be either Male or Female.*/
	CHECK(sex = 'Male' OR sex = 'Female'),
	/*date of birth must earlier than date of death*/
	CHECK(dob < dod)
) ENGINE = innodb;

CREATE TABLE Director (
	id INT NOT NULL COMMENT 'Director ID',
	last VARCHAR(20) NOT NULL COMMENT 'Last name',
	first VARCHAR(20) NOT NULL COMMENT 'First name',
	dob DATE NOT NULL COMMENT 'Date of birth',
	dod DATE COMMENT 'Date of death',
	PRIMARY KEY(id),
	/*id in Actor must not greater than the id in MaxPersonID.*/
	CHECK(id <= ALL (SELECT id FROM MaxPersonID)),
	/*date of birth must earlier than date of death.*/
	CHECK(dob < dod)
) ENGINE = innodb;

CREATE TABLE MovieGenre (
	mid INT NOT NULL COMMENT 'Movie ID',
	genre VARCHAR(20) NOT NULL COMMENT 'Movie genre',
	PRIMARY KEY(mid, genre),
	/*mid in MovieGenre must be the subset of id in Movie.*/
	FOREIGN KEY(mid) REFERENCES Movie(id)
) ENGINE = innodb;

CREATE TABLE MovieDirector (
	mid INT NOT NULL COMMENT 'Movie ID',
	did INT NOT NULL COMMENT 'Director ID',
	PRIMARY KEY(mid, did),
	/*mid in MovieDirector must be the subset of id in Movie.*/
	FOREIGN KEY(mid) REFERENCES Movie(id),
	/*did in MovieDirector must be the subset of id in Director.*/
	FOREIGN KEY(did) REFERENCES Director(id)
) ENGINE = innodb;

CREATE TABLE MovieActor (
	mid INT NOT NULL COMMENT 'Movie ID',
	aid INT NOT NULL COMMENT 'Actor ID',
	role VARCHAR(50) NOT NULL COMMENT 'Actor role in movie',
	PRIMARY KEY(mid, aid),
	/*mid in MovieActor must be the subset of id in Movie.*/
	FOREIGN KEY(mid) REFERENCES Movie(id),
	/*aid in MovieActor must be the subset of id in Actor.*/
	FOREIGN KEY(aid) REFERENCES Actor(id)
) ENGINE = innodb;

CREATE TABLE Review (
	name VARCHAR(20) NOT NULL COMMENT 'Reviewer name',
	time TIMESTAMP NOT NULL COMMENT 'Review time',
	mid INT NOT NULL COMMENT 'Movie ID',
	rating INT NOT NULL COMMENT 'Review rating',
	comment VARCHAR(500) NOT NULL COMMENT 'Reviewer comment',
	PRIMARY KEY(name, time, mid),
	/*mid in Review must be the subset of id in Movie.*/
	FOREIGN KEY(mid) REFERENCES Movie(id)
) ENGINE =innodb;

CREATE TABLE MaxPersonID(
	id INT NOT NULL COMMENT 'Max ID assigned to all persons',
	PRIMARY KEY(id),
	/*id must be positive.*/
	CHECK(id >= 0)
) ENGINE = innodb;

CREATE TABLE MaxMovieID(
	id INT NOT NULL COMMENT 'Max ID assigned to all movies',
	PRIMARY KEY(id),
	/*id must be positive.*/
	CHECK(id >= 0)
) ENGINE = innodb;
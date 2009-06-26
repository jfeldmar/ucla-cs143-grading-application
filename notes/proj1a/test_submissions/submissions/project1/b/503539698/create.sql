DROP TABLE IF EXISTS Movie;
CREATE TABLE Movie (
	id	INTEGER NOT NULL,
	title	VARCHAR(100) NOT NULL,
	year	INTEGER,
	rating	VARCHAR(10),
	company VARCHAR(50),
	PRIMARY KEY(id),
	UNIQUE(title),
	CHECK( year >= 1800 AND year <= 2008 )
);

DROP TABLE IF EXISTS Actor;
CREATE TABLE Actor (
	id	INTEGER NOT NULL,
	last	VARCHAR(20) NOT NULL,
	first	VARCHAR(20) NOT NULL,
	sex	VARCHAR(6),
	dob	DATE,
	dod	DATE,
	PRIMARY KEY(id),
	CHECK( dob < dod )
);

DROP TABLE IF EXISTS Director;
CREATE TABLE Director (
	id	INTEGER NOT NULL,
	last	VARCHAR(20) NOT NULL,
	first	VARCHAR(20) NOT NULL,
	dob	DATE,
	dod	DATE,
	PRIMARY KEY(id),
	CHECK( dob < dod )
);

DROP TABLE IF EXISTS MovieGenre;
CREATE TABLE MovieGenre (
	mid	INTEGER NOT NULL,
	genre	VARCHAR(20) NOT NULL,
	FOREIGN KEY(mid) references Movie(id),
	UNIQUE(genre)
);

DROP TABLE IF EXISTS MovieDirector;
CREATE TABLE MovieDirector (
	mid	INTEGER NOT NULL,
	did	INTEGER NOT NULL,
	FOREIGN KEY(mid) references Movie(id),
	FOREIGN KEY(did) references Director(id)
);

DROP TABLE IF EXISTS MovieActor;
CREATE TABLE MovieActor (
	mid	INTEGER NOT NULL,
	aid	INTEGER NOT NULL,
	role	VARCHAR(50),
	FOREIGN KEY(mid) references Movie(id),
	FOREIGN KEY(aid) references Actor(id)
);

DROP TABLE IF EXISTS Review;
CREATE TABLE Review (
	name	VARCHAR(20) NOT NULL,
	time	TIMESTAMP,
	mid	INTEGER NOT NULL,
	rating	INTEGER,
	comment	VARCHAR(500),
	UNIQUE(name),
	FOREIGN KEY(mid) references Movie(id)
);

DROP TABLE IF EXISTS MaxPersonID;
CREATE TABLE MaxPersonID (
	id	INTEGER,
	PRIMARY KEY(id)
);

DROP TABLE IF EXISTS MaxMovieID;
CREATE TABLE MaxMovieID (
	id	INTEGER,
	PRIMARY KEY(id)
);
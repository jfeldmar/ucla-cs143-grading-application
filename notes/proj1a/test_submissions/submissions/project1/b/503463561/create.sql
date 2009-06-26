SHOW ERRORS;
SHOW WARNINGS;


CREATE TABLE Movie(
	id int NOT NULL,
	title varchar(100) NOT NULL,
	year int,
	rating varchar(10),
	company varchar(50),
	PRIMARY KEY(id),
	UNIQUE(id, company),
	CHECK(year >= 1877) )
	ENGINE = INNODB;

CREATE TABLE Actor(
	id int NOT NULL, 
	last varchar(20), 
	first varchar(20) NOT NULL, 
	sex varchar(6)  NOT NULL, 
	dob date NOT NULL, 
	dod date,
	PRIMARY KEY(id),
	UNIQUE(id),
	CHECK(dod > dob) )
	ENGINE = INNODB;

CREATE TABLE Director(
	id int NOT NULL, 
	last varchar(20), 
	first varchar(20), 
	dob date, 
	dod date,
	PRIMARY KEY(id),
	UNIQUE(id),
	CHECK(dod > dob) )
	ENGINE = INNODB;

CREATE TABLE MovieGenre(
	mid int NOT NULL, 
	genre varchar(20), 
	FOREIGN KEY (mid) REFERENCES Movie(id)) 
	ENGINE = INNODB;

CREATE TABLE MovieDirector(
	mid int NOT NULL, 
	did int NOT NULL,
	FOREIGN KEY (mid) REFERENCES Movie(id),
	FOREIGN KEY (did) REFERENCES Director(id)) 
	ENGINE = INNODB;

CREATE TABLE MovieActor(
	mid int NOT NULL, 
	aid int NOT NULL, 
	role varchar(50),
	FOREIGN KEY (mid) REFERENCES Movie(id),
	FOREIGN KEY (aid) REFERENCES Actor(id)) 
	ENGINE = INNODB;

CREATE TABLE Review(
	name varchar(20), 
	time timestamp, 
	mid int NOT NULL, 
	rating int, 
	comment varchar(500),
	PRIMARY KEY(name, time),
	FOREIGN KEY (mid) REFERENCES Movie(id))
	ENGINE = INNODB;

CREATE TABLE MaxPersonID(
	id int )
	ENGINE = INNODB;

CREATE TABLE MaxMovieID(
	id int )
	ENGINE = INNODB;

CREATE TABLE Movie(
	id INTEGER NOT NULL,
	title VARCHAR(100) NOT NULL,
	year INTEGER NOT NULL,
	rating VARCHAR(10),
	company VARCHAR(50),
	PRIMARY KEY(id),
	-- checks to see that the rating falls within 
	-- the current rating system of 5 MPAA ratings
	CHECK(rating in ('G', 'PG', 'PG-13', 'R', 'NC-17'))
) ENGINE=INNODB;

CREATE TABLE Actor(
	id INTEGER NOT NULL,
	last VARCHAR(20),
	first VARCHAR(20),
	sex VARCHAR(6),
	dob DATE NOT NULL,
	dod DATE,
	PRIMARY KEY(id),
	-- if the Actor is also a Director, they should have 
	-- the same id number
	CHECK(id in (SELECT id
		FROM Director
		WHERE first = Director.first
		AND   last = Director.last)),
	-- checks to make sure actors are either male or female
	CHECK(sex in ('M', 'F'))
) ENGINE=INNODB;

CREATE TABLE Director(
	id INTEGER NOT NULL,
	last VARCHAR(20),
	first VARCHAR(20),
	dob DATE NOT NULL,
	dod DATE,
	PRIMARY KEY(id),
	-- if the Director is also an Actor, they should have 
	-- the same id number
	CHECK(id in (SELECT id
		FROM Actor
		WHERE Actor.first = first
		AND   Actor.last = last))
) ENGINE=INNODB;

CREATE TABLE MovieGenre(
	mid INTEGER NOT NULL,
	genre VARCHAR(20),
	FOREIGN KEY (mid) references Movie(id)
) ENGINE=INNODB;

CREATE TABLE MovieDirector(
	mid INTEGER NOT NULL,
	did INTEGER NOT NULL,
	FOREIGN KEY (mid) references Movie(id),
	FOREIGN KEY (did) references Director(id)
) ENGINE=INNODB;

CREATE TABLE MovieActor(
	mid INTEGER NOT NULL,
	aid INTEGER NOT NULL,
	role VARCHAR(50),
	FOREIGN KEY (mid) references Movie(id),
	FOREIGN KEY (aid) references Actor(id)
) ENGINE=INNODB;

CREATE TABLE Review(
	name VARCHAR(20),
	time TIMESTAMP,
	mid INTEGER NOT NULL,
	rating INTEGER,
	comment VARCHAR(100),
	FOREIGN KEY (mid) references Movie(id)
) ENGINE=INNODB;

CREATE TABLE MaxPersonID(
	id INTEGER NOT NULL
) ENGINE=INNODB;

CREATE TABLE MaxMovieID(
	id INTEGER NOT NULL
) ENGINE=INNODB;

INSERT INTO MaxMovieID values('4750');
INSERT INTO MaxPersonID values('69000');

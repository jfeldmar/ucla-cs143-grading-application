CREATE TABLE Movie(
	id	INTEGER 	NOT NULL,
	title	VARCHAR(100)	NOT NULL,
	year	INTEGER		NOT NULL,
	rating	VARCHAR(10),
	company	VARCHAR(50),
	-- id is a primary key constraint
	PRIMARY KEY(id),
	-- rating can only be one of five, 
	-- This is the 1st CHECK constraint
	CHECK( rating IN ('G','PG','PG-13','R','NC-17') )

) ENGINE=INNODB;

CREATE TABLE Actor(
	id	INTEGER		NOT NULL,
	last	VARCHAR(20)	NOT NULL,
	first	VARCHAR(20)	NOT NULL,
	sex 	VARCHAR(6)	NOT NULL,
	dob	DATE		NOT NULL,
	dod	DATE,
	-- id is a primary key constraint
	PRIMARY KEY(id),
	-- if a person is both actor and director,
	-- id for that person must be the same.
	-- This is the 2nd CHECK constraint
	CHECK( id IN (	SELECT 	id
	 	FROM 	Director
		WHERE 	first = Director.first AND
			last  = Director.last)),
	-- sex can only be one male or female, 
	-- This is the 3rd CHECK constraint
	CHECK( sex IN ('F','M'))
) ENGINE=INNODB;

CREATE TABLE Director(
	id	INTEGER		NOT NULL,
	last	VARCHAR(20)	NOT NULL,
	first	VARCHAR(20)	NOT NULL,
	dob	DATE		NOT NULL,
	dod	DATE,
	-- id is a primary key constraint
	PRIMARY KEY(id),
	-- if a person is both actor and director,
	-- id for that person must be the same.
	-- This is the 2nd CHECK constraint
	CHECK( id IN ( SELECT	id
		FROM 	Actor
		WHERE	Actor.first = Director.first AND
			Actor.last  = Director.last) )	
) ENGINE=INNODB;

CREATE TABLE MovieGenre(
	mid	INTEGER 	NOT NULL,
	genre	VARCHAR(20),
	-- movie id references to id of Movie table
	-- This is the 1st referential integrity constraint
	FOREIGN KEY(mid) references Movie(id)
) ENGINE=INNODB;

CREATE TABLE MovieDirector(
	mid	INTEGER		NOT NULL,
	did	INTEGER		NOT NULL,
	-- movie id references to id of Movie table
	-- This is the 2nd referential integrity constraint
	FOREIGN KEY(mid) references Movie(id),
	-- director id references to id of Director table
	-- This is the 3rd referential integrity constraint
	FOREIGN KEY(did) references Director(id)
) ENGINE=INNODB;

CREATE TABLE MovieActor(
	mid	INTEGER		NOT NULL,
	aid	INTEGER		NOT NULL,
	role	VARCHAR(50),
	-- movie id references to id of Movie table
	-- This is the 4th referential integrity constraint
	FOREIGN KEY(mid) references Movie(id),
	-- actor id references to id of Actor table
	-- This is the 5th referential integrity constraint
	FOREIGN KEY(aid) references Actor(id)
) ENGINE=INNODB;

CREATE TABLE Review(
	name	VARCHAR(20)	NOT NULL,
	time	TIMESTAMP,
	mid	INTEGER		NOT NULL,
	rating	INTEGER		NOT NULL,
	comment VARCHAR(500)	NOT NULL,
	-- movie id references to id of Movie table
	-- This is the 6th referential integrity constraint
	FOREIGN KEY(mid) references Movie(id)
) ENGINE=INNODB;

CREATE TABLE MaxPersonID(
	id	INTEGER 	NOT NULL
) ENGINE=INNODB;

CREATE TABLE MaxMovieID(
	id 	INTEGER		NOT NULL
	-- Preferrably this referential constraint is in place
	-- but it is giving a problem because the data set provided
 	-- have 4734 as the max movie id

	-- FOREIGN KEY(mid) references Movie(id)
) ENGINE=INNODB;

INSERT INTO MaxPersonID VALUES(69000);

INSERT INTO MaxMovieID VALUES(4750);
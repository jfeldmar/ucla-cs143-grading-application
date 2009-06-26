--CHECK 1. The id should be less than or equal to the MaxMovieID
--PRIMARY KEY 1. Each movie should have a different ID, so this is one Primary Key
CREATE TABLE Movie(
	id INT CHECK (id <= (SELECT id FROM MaxMovieID)),
	title VARCHAR(100), 
	year INT, 
	rating VARCHAR(10), 
	company VARCHAR(50),
	PRIMARY KEY(id)
) ENGINE=INNODB;

--CHECK 2. The id should be less than or equal to the MaxPersonID
--PRIMARY KEY 2. Each actor should have a different ID, so this is another Primary Key
CREATE TABLE Actor (
	id INT CHECK (id <= (SELECT id FROM MaxPersonID)),
	last VARCHAR(20),
	first VARCHAR(20),
	sex VARCHAR(6),
	dob DATE,
	dod DATE,	
	PRIMARY KEY(id)
) ENGINE=INNODB;

--CHECK 3. The id should be less than or equal to the MaxPersonID
--PRIMARY KEY 3. Each director should have a different ID, so this is another primary key
CREATE TABLE Director (	
	id INT CHECK (id <= (SELECT id FROM MaxPersonID)),
	last VARCHAR(20),
	first VARCHAR(20),
	dob DATE,
	dod DATE,	
	PRIMARY KEY(id)
) ENGINE=INNODB;

--REFERENTIAL 1. The mid in MovieGenre should reference an id in Movie
CREATE TABLE MovieGenre (
	mid INT,
	genre VARCHAR(20),
	FOREIGN KEY (mid) REFERENCES Movie(id)
) ENGINE=INNODB;

-- REFERENTIAL 2/3. The mid should reference a id in Movie. The did should reference a id in Director
CREATE TABLE MovieDirector (
	mid INT,
	did INT,
	FOREIGN KEY (mid) REFERENCES Movie(id),
	FOREIGN KEY (did) REFERENCES Director(id)
) ENGINE=INNODB;

-- REFERENTIAL 4/5. The mid and aid ids should both be in their respective tables
CREATE TABLE MovieActor (
	mid INT,
	aid INT,
	role VARCHAR(50),
	FOREIGN KEY (mid) REFERENCES Movie(id),
	FOREIGN KEY (aid) REFERENCES Actor(id)
) ENGINE=INNODB;

--CHECK 4. The rating has to be out of 5 so a CHECK could go here
--REFERENTIAL 6. The mid in the movie should reference a movie that's in the Movie table
CREATE TABLE Review (
	name VARCHAR(20),
	time TIMESTAMP,
	mid INT,	
	rating INT CHECK(0 <= rating and rating <=5),
	comment VARCHAR(500),	
	FOREIGN KEY (mid) REFERENCES Movie(id)
) ENGINE=INNODB;

CREATE TABLE MaxPersonID (
	id INT
) ENGINE=INNODB; 

CREATE TABLE MaxMovieID (
	id INT
) ENGINE=INNODB;
-- Name: Angel Darquea
-- ID: 003512936
-- Date: 10/15/2008
-- Fall 2008
-- Project 1B - create.sql

-- The Movie Table
-- -- Create the table:
CREATE TABLE Movie (
	id INT NOT NULL, -- Ensures key is not null.
	title VARCHAR(100) NOT NULL, -- Ensures title is not null.
	year INT,
	rating VARCHAR(10),
	company VARCHAR(50),
	PRIMARY KEY(id), -- Ensures there is a unique ID for each movie,
	CHECK(id > 0 AND (rating = 'G' OR rating = 'PG' OR rating = 'PG-13' OR rating = 'R' OR rating = 'NC-17')) -- Ensures ID is not a negative number and rating is valid.
)ENGINE=INNODB;
-- --------------------------------------------------------------------------------------------------------------------
-- The Actor Table
-- -- Create the table:
CREATE TABLE Actor (
	id INT NOT NULL, -- Ensures key is not null.
	last VARCHAR(20) NOT NULL, --  Ensures a last name is provided
	first VARCHAR(20) NOT NULL, -- Ensures a first name is provided
	sex VARCHAR(6),
	dob DATE NOT NULL, -- Ensures a dob is provided.
	dod DATE,
	PRIMARY KEY(id), -- Ensures there is a unique ID for each actor.
	UNIQUE (id, last, first), -- Ensures that each ID is associated to a unique actor.
	UNIQUE (last, first), -- Ensures that each actor's name is unique.
	CHECK(id > 0 AND (sex = 'Male' OR sex = 'Female')) -- Ensures ID is not a negative number and sex is valid.
)ENGINE=INNODB;
-- --------------------------------------------------------------------------------------------------------------------
-- The Director Table
-- -- Create the table:
CREATE TABLE Director (
	id INT NOT NULL, -- Ensures key is not null.
	last VARCHAR(20) NOT NULL,
	first VARCHAR(20) NOT NULL,
	dob DATE NOT NULL, -- Ensures a dob is provided. 
	dod DATE,
	PRIMARY KEY(id), -- Ensures there is a unique ID for each actor.
	UNIQUE (id, last, first), -- Ensures that each ID is associated to a unique director.
	UNIQUE (last, first), -- Ensures that each director's names is unique.
	CHECK(id > 0) -- Ensures ID is not a negative number
)ENGINE=INNODB;
-- --------------------------------------------------------------------------------------------------------------------
-- The MovieGenre Table
-- -- Create the table:
CREATE TABLE MovieGenre (
	mid INT NOT NULL, -- Ensures the movie ID is not null.
	genre VARCHAR(20),
	FOREIGN KEY (mid) REFERENCES Movie(id), -- Ensures that movie entered here exists in Movie table.
	CHECK(genre = 'Action' OR
			genre = 'Adult' OR
			genre = 'Adventure' OR
			genre = 'Animation' OR
			genre = 'Comedy' OR
			genre = 'Crime' OR
			genre = 'Documentary' OR
			genre = 'Drama' OR
			genre = 'Family' OR
			genre = 'Fantasy' OR
			genre = 'Horror' OR
			genre = 'Musical' OR
			genre = 'Mystery' OR
			genre = 'Romance' OR
			genre = 'Sci-Fi' OR
			genre = 'Short' OR
			genre = 'Thriller' OR
			genre = 'War' OR
			genre = 'Western') -- Ensures genre entered is valid.
)ENGINE=INNODB;
-- --------------------------------------------------------------------------------------------------------------------
-- The MovieDirector Table
-- -- Create the table:
CREATE TABLE MovieDirector (
	mid INT NOT NULL, -- Ensures the movie ID is not null.
	did INT, -- Ensures the director ID is not bull.
	FOREIGN KEY (mid) REFERENCES Movie(id), -- Ensures that the movie here exists in Movie table.
	FOREIGN KEY (did) REFERENCES Director(id) -- Ensures that director entered here exists in Director table.
)ENGINE=INNODB;
-- --------------------------------------------------------------------------------------------------------------------
-- The MovieActor Table
-- -- Create the table:
CREATE TABLE MovieActor (
	mid INT NOT NULL,
	aid INT NOT NULL,
	role VARCHAR(50),
	FOREIGN KEY (mid) REFERENCES Movie(id), -- Ensures that the movie here exists in Movie table.
	FOREIGN KEY (aid) REFERENCES Actor(id) -- Ensures that actor entered here exists in actor table.
)ENGINE=INNODB;
-- --------------------------------------------------------------------------------------------------------------------
-- The Review Table
-- -- Create the table:
CREATE TABLE Review (
	name VARCHAR(20),
	time TIMESTAMP,
	mid INT NOT NULL,
	rating INT,
	comment VARCHAR(500),
	FOREIGN KEY (mid) REFERENCES Movie(id) -- Ensures that the movie here exists in Movie table.
)ENGINE=INNODB;
-- --------------------------------------------------------------------------------------------------------------------
-- The MaxPerson Table
-- -- Create the table:
CREATE TABLE MaxPersonID (
	id INT
)ENGINE=INNODB;
-- --------------------------------------------------------------------------------------------------------------------
-- The MaxMovieID Table
-- -- Create the table:
CREATE TABLE MaxMovieID (
	id INT
)ENGINE=INNODB;
-- --------------------------------------------------------------------------------------------------------------------



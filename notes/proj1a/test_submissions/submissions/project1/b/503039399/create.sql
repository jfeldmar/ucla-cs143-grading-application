-- Create the Movie table
-- Constraints: Movie IDs must be unique, Every Movie ID must be greater than zero,
-- Every movie must have a title, Every movie must have a year
CREATE TABLE Movie(id int, title VARCHAR(100) NOT NULL, year int NOT NULL, rating VARCHAR(10), company VARCHAR(50),
	PRIMARY KEY (id),
	CHECK (id > 0))
	ENGINE=INNODB;

-- Create the Actor table.
-- Constraints: Actor IDs must be unique, Actors need a DOB, id must be positive
CREATE TABLE Actor(id int, last VARCHAR(20), first VARCHAR(20), sex VARCHAR(6)
, dob DATE NOT NULL, dod DATE, 
	PRIMARY KEY (id),
	CHECK (id > 0))
	ENGINE=INNODB;

-- Create the Director table
-- Constraints: Director ID must be unique, directors need a DOB, Director ID must be positive
CREATE TABLE Director(id int, last VARCHAR(20), first VARCHAR(20), dob DATE NOT NULL, dod DATE,
	PRIMARY KEY (id),
	CHECK (id > 0))
	ENGINE=INNODB;

-- Create the MovieGenre table
-- Constraints: MovieGenre entries must refer to a valid movie
CREATE TABLE MovieGenre(mid INT, genre VARCHAR(20),
	FOREIGN KEY (mid) references Movie(id))
	ENGINE=INNODB;

-- Create the MovieDirector table
-- Constraints: MovieDirector entries must refer to a valid movie and valid Director
CREATE TABLE MovieDirector(mid INT, did INT,
	FOREIGN KEY (mid) references Movie(id),
	FOREIGN KEY (did) references Director(id))
	ENGINE=INNODB;

-- Create the MovieActor table
-- Constraints: Actor entries must refer to a valid movie and Actor
CREATE TABLE MovieActor(mid INT, aid INT, role VARCHAR(20),
	FOREIGN KEY (mid) references Movie(id),
	FOREIGN KEY (aid) references Actor(id))
	ENGINE=INNODB;
	
-- Create the Review table	
-- Constraints: Review entries must refer to a valid movie	
CREATE TABLE Review(name VARCHAR(20), time TIMESTAMP, mid INT, rating INT, comment VARCHAR(500),
	FOREIGN KEY (mid) references Movie(id))
	ENGINE=INNODB;

-- Create the MaxPersonID table	
CREATE TABLE MaxPersonID(id INT) ENGINE=INNODB;

-- Create the MaxMovieID table	
CREATE TABLE MaxMovieID(id INT) ENGINE=INNODB;

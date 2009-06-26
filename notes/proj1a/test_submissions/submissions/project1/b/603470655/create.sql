-- Use Appropriate Database
USE CS143;

-- Drop Existing Tables
DROP TABLE IF EXISTS MovieGenre;
DROP TABLE IF EXISTS MovieDirector;
DROP TABLE IF EXISTS MovieActor;
DROP TABLE IF EXISTS Review;
DROP TABLE IF EXISTS Movie;
DROP TABLE IF EXISTS Actor;
DROP TABLE IF EXISTS Director;
DROP TABLE IF EXISTS MaxPersonID;
DROP TABLE IF EXISTS MaxMovieID;

-- Create Tables
CREATE TABLE Movie
(id INT NOT NULL,			-- Every movie must have an id
 title VARCHAR(100) NOT NULL,	-- Every movie must have a title
 year INT,
 rating VARCHAR(10),
 company VARCHAR(50),
 PRIMARY KEY(id),			-- Every movie id is unique
 CHECK(year >= 0))		-- Year can't be a negative number
ENGINE = INNODB;

CREATE TABLE Actor
(id INT NOT NULL,			-- Every actor must have an id
 last VARCHAR(20),
 first VARCHAR(20),
 sex VARCHAR(6),
 dob DATE NOT NULL,		-- Every actor must have a dob
 dod DATE,
 PRIMARY KEY(id),			-- Every actor id is unique
 CHECK(dob <= dod))		-- DOB can't exceed DOD
ENGINE = INNODB;

CREATE TABLE Director
(id INT NOT NULL,			-- Every director must have an id
 last VARCHAR(20),
 first VARCHAR(20),
 dob DATE NOT NULL,		-- Every director must have a dob
 dod DATE,
 PRIMARY KEY(id),			-- Every director id is unique
 CHECK(dob <= dod))		-- DOB can't exceed DOD
ENGINE = INNODB;

CREATE TABLE MovieGenre
(mid INT NOT NULL,		-- Every movie must have an id
 genre VARCHAR(20),
 FOREIGN KEY (mid) references Movie(id)) -- Associate with movie that
						     -- already exists in Movie table
ENGINE = INNODB;

CREATE TABLE MovieDirector
(mid INT NOT NULL,		-- Every movie must have an id
 did INT NOT NULL,		-- Every director must have an id
 FOREIGN KEY (mid) references Movie(id), -- Associate with movie that
						     -- already exists in Movie table
 FOREIGN KEY (did) references Director(id)) -- Associate with director that
							  -- already exists in Director table
ENGINE = INNODB;

CREATE TABLE MovieActor
(mid INT NOT NULL,		-- Every movie must have an id
 aid INT NOT NULL,		-- Every actor must have an id
 role VARCHAR(50),
 FOREIGN KEY (mid) references Movie(id), -- Associate with movie that
						     -- already exists in Movie table
 FOREIGN KEY (aid) references Actor(id)) -- Associate with actor that
						     -- already exists in Actor table
ENGINE = INNODB;

CREATE TABLE Review
(name VARCHAR(20),
 time TIMESTAMP,
 mid INT NOT NULL,		-- Every movie must have an id
 rating INT,
 comment VARCHAR(500),
 FOREIGN KEY (mid) references Movie(id)) -- Associate with movie that
						     -- already exists in Movie table
ENGINE = INNODB;

CREATE TABLE MaxPersonID
(id INT NOT NULL);		-- Max Person ID must always be an actual value

CREATE TABLE MaxMovieID
(id INT NOT NULL);		-- Max Movie ID must always be an actual value

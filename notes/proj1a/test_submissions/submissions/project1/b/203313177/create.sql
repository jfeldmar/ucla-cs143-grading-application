-- Creates the Movie Table with the constraint that all Movie IDs (id) must be unique using a Primary Key Constraint
-- and checks that the title, year, and production company are all specified and not NULL.
CREATE TABLE Movie
(
	id INT,
	title VARCHAR(100),
	year INT,
	rating VARCHAR(10),
	company VARCHAR(50),
	PRIMARY KEY(id),
	CHECK(title <> NULL),
	CHECK(year <> NULL),
	CHECK(company <> NULL)
) ENGINE = INNODB;

-- Creates the Actor Table with the constraint that all Actor IDs (id) must be unique using a Primary Key Constraint
-- and checks that the date of birth (dob) and gender (sex) are both specified and not NULL.
CREATE TABLE Actor
(
	id INT,
	last VARCHAR(20),
	first VARCHAR(20),
	sex VARCHAR(6),
	dob DATE,
	dod DATE,
	PRIMARY KEY (id),
	CHECK(dob <> NULL),
	CHECK(sex <> NULL)
) ENGINE = INNODB;

-- Creates the Director Table with the constraint that all Director IDs (id) must be unique using a Primary Key Constraint
-- and checks that every director has a date of birth (dob) specified.
CREATE TABLE Director
(
	id INT,
	last VARCHAR(20),
	first VARCHAR(20),
	dob DATE,
	dod DATE,
	PRIMARY KEY (id),
	CHECK(dob <> NULL)
) ENGINE = INNODB;

-- Creates the MovieGenre Table ensuring the mid corresponds to an actual movie in the Movie Table 
-- by using a Foreign Key Constraint on mid referring to Movie IDs.
CREATE TABLE MovieGenre
(
	mid INT,
	genre VARCHAR(20),
	FOREIGN KEY (mid) references Movie(id)
) ENGINE = INNODB;

-- Creates the MovieDirector Table ensuring the mid corresponds to an actual movie in the Movie Table
-- and the did corresponds to an actual directer in the Director Table by using Foreign Key Constraints
-- on mid and did, respectively, referring them to Movie IDs and Director IDs respectively.
CREATE TABLE MovieDirector
(
	mid INT,
	did INT,
	FOREIGN KEY (mid) references Movie(id),
	FOREIGN KEY (did) references Director(id)
) ENGINE = INNODB;

-- Creates the MovieActor Table ensuring the mid corresponds to an actual movie in the Movie Table
-- and the aid corresponds to an actual actor in the Actor Table by using Foreign Key Constraints
-- on mid and aid, respectively, referring them to Movie IDs and Actor IDs respectively.
CREATE TABLE MovieActor
(
	mid INT,
	aid INT,
	role VARCHAR(50),
	FOREIGN KEY (mid) references Movie(id),
	FOREIGN KEY (aid) references Actor(id)
) ENGINE = INNODB;

-- Creates the Review Table ensuring the mid corresponds to an actual movie in the Movie Table 
-- by using a Foreign Key Constraint on mid referring to Movie IDs.
CREATE TABLE Review
(
	name VARCHAR(20),
	time TIMESTAMP,
	mid INT,
	rating INT,
	comment VARCHAR(50),
	FOREIGN KEY (mid) references Movie(id)
) ENGINE = INNODB;

-- Creates the MaxPersonID Table.
CREATE TABLE MaxPersonID
(
	id INT,
	PRIMARY KEY (id)
);

-- Creates the MaxMovieID Table.
CREATE TABLE MaxMovieID
(
	id INT,
	PRIMARY KEY (id)
);
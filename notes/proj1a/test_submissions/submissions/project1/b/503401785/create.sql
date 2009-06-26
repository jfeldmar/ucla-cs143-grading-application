CREATE TABLE Movie (
	id int NOT NULL,
	title varchar(100) NOT NULL,
	year int,
	rating varchar(10),
	company varchar(50),
	PRIMARY KEY (id),
	CHECK (id > 0)) ENGINE = INNODB;
--Movie IDs and Titles cannot be NULL values, movie ID is set to
--a primary key and checks that they are not negative values.


CREATE TABLE Actor (
	id int NOT NULL,
	last varchar(20) NOT NULL,
	first varchar(20) NOT NULL,
	sex varchar(6) NOT NULL,
	dob date,
	dod date,
	PRIMARY KEY (id),
	CHECK (sex = 'Female' OR sex = 'Male'),
	CHECK (id > 0)) ENGINE = INNODB;
--Actor IDs, names and sex cannot be NULL values, the primary key
--is set to the actor ID and checks that no IDS are negative and
--that the sex is either male or female.


CREATE TABLE Director (
	id int NOT NULL,
	last varchar(20) NOT NULL,
	first varchar(20) NOT NULL,
	dob date,
	dod date,
	PRIMARY KEY (id),
	CHECK (id > 0)) ENGINE = INNODB;
--Director IDs and names cannot be NULL values, the primary key is
--set to the director ID and it is checked to be a positive value.


CREATE TABLE MovieGenre (
	mid int NOT NULL,
	genre varchar(20),
	FOREIGN KEY (mid) REFERENCES Movie (id)) ENGINE = INNODB;
--Movie ID cannot be NULL and refers to the Movie table.


CREATE TABLE MovieDirector (
	mid int NOT NULL,
	did int NOT NULL,
	FOREIGN KEY (mid) REFERENCES Movie (id),
	FOREIGN KEY (did) REFERENCES Director (id)) ENGINE = INNODB;
--Movie ID and director ID cannot be NULL and refers to the Movie
--and Director tables respectively.



CREATE TABLE MovieActor (
	mid int NOT NULL,
	aid int NOT NULL,
	role varchar(50),
	FOREIGN KEY (mid) REFERENCES Movie (id),
	FOREIGN KEY (aid) REFERENCES Actor (id)) ENGINE = INNODB;
--Movie ID and actor ID cannot be NULL ad refers to the Movie and
--Actor tables respectively.


CREATE TABLE Review (
	name varchar(20) NOT NULL,
	time timestamp,
	mid int NOT NULL,
	rating int NOT NULL,
	comment varchar(500),
	FOREIGN KEY (mid) REFERENCES Movie (id)) ENGINE = INNODB;
--Name of the reviewer, the movie being reviewed, and the rating
--cannot be NULL. Refers the movie id to the Movie table.


CREATE TABLE MaxPersonID (
	id int,
	CHECK (id > 0)
);


CREATE TABLE MaxMovieID (
	id int,
	CHECK (id > 0)
);
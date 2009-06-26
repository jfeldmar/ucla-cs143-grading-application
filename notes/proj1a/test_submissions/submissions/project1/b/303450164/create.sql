CREATE TABLE Movie(
	id 			int NOT NULL, 
	title 		varchar(100), 
	year 		int, 
	rating 		varchar(10), 
	company 	varchar(50), 
	PRIMARY KEY(id),
	CHECK(company IS NOT NULL))
	ENGINE=INNODB;
--KEY CONSTRAINT #1: every movie has a unique id number
--CHECK CONSTRAINT #1: every movie must be made by a company

CREATE TABLE Actor(
	id 			int NOT NULL, 
	last 		varchar(20), 
	first 		varchar(20), 
	sex 		varchar(6), 
	dob 		date, 
	dod 		date, 
	PRIMARY KEY(id),
	CHECK(sex='Female' OR sex='Male'))
	ENGINE=INNODB;
--KEY CONSTRAINT #2: every actor has a unique id number
--CHECK CONSTRAINT #2: every actor must have a sex of either Male or Female

CREATE TABLE Director(
	id 			int NOT NULL, 
	last 		varchar(20), 
	first 		varchar(20), 
	dob 		date, 
	dod 		date, 
	PRIMARY KEY(id),
	CHECK(dob IS NOT NULL))
	ENGINE=INNODB;
--KEY CONSTRAINT #3: every director has a unique id number
--CHECK CONSTRAINT #3: every director must have a date of birth

CREATE TABLE MovieGenre(
	mid 		int NOT NULL, 
	genre 		varchar(20) NOT NULL, 
	PRIMARY KEY(mid,genre),
	FOREIGN KEY(mid) REFERENCES Movie(id))
	ENGINE=INNODB;
--FOREIGN KEY #1: every movie id in the moviegenre table must be in the movie table

CREATE TABLE MovieDirector(
	mid 		int NOT NULL, 
	did 		int NOT NULL, 
	PRIMARY KEY(mid,did),
	FOREIGN KEY(did) REFERENCES Director(id),
	FOREIGN KEY(mid) REFERENCES Movie(id))
	ENGINE=INNODB;
--FOREIGN KEY #2: every director id in the moviedirector table must be in the director table
--FOREIGN KEY #3: every movie id in the moviedirector table must be in the movie table

CREATE TABLE MovieActor(
	mid 		int NOT NULL, 
	aid 		int NOT NULL, 
	role 		varchar(50), 
	PRIMARY KEY(mid,aid),
	FOREIGN KEY(aid) REFERENCES Actor(id),
	FOREIGN KEY(mid) REFERENCES Movie(id))
	ENGINE=INNODB;
--FOREIGN KEY #4: every actor id in the movieactor table must be in the actor table
--FOREIGN KEY #5: every movie id in the movieactor table must be in the movie table

CREATE TABLE Review(
	name 		varchar(20) NOT NULL, 
	time 		timestamp NOT NULL, 
	mid 		int, 
	rating 		int, 
	comment 	varchar(500), 
	PRIMARY KEY(name, time),
	FOREIGN KEY(mid) REFERENCES Movie(id))
	ENGINE=INNODB;
--FOREIGN KEY #6: every movie id in the review table must be in the movie table

CREATE TABLE MaxPersonID(
	id 			int NOT NULL, 
	PRIMARY KEY(id));

CREATE TABLE MaxMovieID(
	id 			int NOT NULL, 
	PRIMARY KEY(id));
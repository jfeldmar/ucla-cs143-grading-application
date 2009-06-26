CREATE TABLE Movie (
	id	int		NOT NULL,
	title	varchar(100)	NOT NULL,
	year	int		NOT NULL,
	rating	varchar(10)	NOT NULL,
	company	varchar(50)	NOT NULL,
	PRIMARY KEY (id), -- ids are unique
	-- ERROR 1062 (23000) at line 2: Duplicate entry '12' for key 1
	CHECK(year >= 0 AND year <= 2008 AND id > 0) -- ids are positive, and the year should be meaningful, but not in the future
) ENGINE=INNODB;

CREATE TABLE Actor (
	id	int		NOT NULL,
	last	varchar(20)	NOT NULL,
	first	varchar(20)	NOT NULL,
	sex	varchar(6)	NOT NULL,
	dob	date		NOT NULL,
	dod	date,
	PRIMARY KEY (id), -- ids are unique
	-- ERROR 1062 (23000) at line 2: Duplicate entry '1' for key 1
	CHECK(id > 0), -- ids are positive
	CHECK(sex = "Male" OR sex = "Female")
) ENGINE=INNODB;

CREATE TABLE Director (
	id	int		NOT NULL,
	last	varchar(20)	NOT NULL,
	first	varchar(20)	NOT NULL,
	dob	date		NOT NULL,
	dod	date,
	PRIMARY KEY (id), -- ids are unique
	-- ERROR 1062 (23000) at line 2: Duplicate entry '16' for key 1
	CHECK(id > 0) -- ids are positive
) ENGINE=INNODB;

CREATE TABLE MovieGenre (
	mid	int		NOT NULL,
	genre	varchar(20)	NOT NULL,
	FOREIGN KEY (mid) references Movie(id)) ENGINE=INNODB; -- the movie id referenced must be in Movie
	-- ERROR 1452 (23000) at line 7: Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

CREATE TABLE MovieDirector (
	mid	int		NOT NULL,
	did	int		NOT NULL,
	FOREIGN KEY (mid) references Movie(id), -- the movie id referenced must be in Movie
	-- ERROR 1452 (23000) at line 8: Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
	FOREIGN KEY (did) references Director(id)) ENGINE=INNODB; -- the director id referenced must be in Director

CREATE TABLE MovieActor (
	mid	int		NOT NULL,
	aid	int		NOT NULL,
	role	varchar(50)	NOT NULL,
	FOREIGN KEY (mid) references Movie(id), -- the movie id referenced must be in Movie
	-- ERROR 1452 (23000) at line 10: Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
	FOREIGN KEY (aid) references Actor(id)) ENGINE=INNODB; -- the actor id referenced must be in Actor
	-- ERROR 1452 (23000) at line 11: Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))

CREATE TABLE Review (
	name	varchar(20)	NOT NULL,
	time	timestamp	NOT NULL,
	mid	int		NOT NULL,
	rating	int		NOT NULL,
	comment	varchar(500)	NOT NULL,
	FOREIGN KEY (mid) references Movie(id), -- the movie id referenced must be in Movie
	-- ERROR 1452 (23000) at line 10: Cannot add or update a child row: a foreign key constraint fails (`CS143/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
	CHECK(rating >= 0 AND rating <= 5 AND comment <> "")) ENGINE=INNODB; -- rating must be between 0 and 5 inclusive, and the comment is not empty

CREATE TABLE MaxPersonID (
	id	int		NOT NULL,
	CHECK(id >= 0)
);

CREATE TABLE MaxMovieID (
	id	int		NOT NULL,
	CHECK(id >= 0)
);
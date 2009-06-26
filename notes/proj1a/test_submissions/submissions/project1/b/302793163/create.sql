CREATE TABLE Movie (
	id 	int ,
	title	varchar(100) not null,	-- C1:movie must have a title
	year	int,
	rating	varchar(10),
	company	varchar(50),
	primary key(id)			-- P1:each movie has unique id
	)
	ENGINE = INNODB;
CREATE TABLE Actor (
	id	int ,
	last	varchar(20) not null,	-- C3:actor must have a last name
	first	varchar(20) not null,	-- C3:actor must have a first name
	sex	varchar(6),
	dob	date not null,		-- C2:must have a date of birth
	dod	date,
	primary key(id),		-- P2:each actor id must be unique
	check( sex = 'Male' or sex = 'Female' )	-- C4:sex is male/female
	)
	ENGINE = INNODB;
CREATE TABLE Director (
	id	int ,
	last 	varchar(20) not null,	-- must have a last name
	first	varchar(20) not null,	-- must have a first name
	dob	date not null,		-- must have a birth date
	dod	date,
	primary key(id)			-- P3:each director id must be unique
	)
	ENGINE = INNODB;
CREATE TABLE MovieGenre (
	mid	int ,
	genre	varchar(20),
	unique(mid),
	foreign key(mid) references Movie(id)	-- R1:movie ids are linked
	)
	ENGINE = INNODB;
CREATE TABLE MovieDirector (
	mid	int,
	did	int,
	unique(mid, did),
	foreign key(mid) references Movie(id),	-- R2:movie ids are linked
	foreign key(did) references Director(id) -- R3:director ids are linked 
	)
	ENGINE = INNODB;
CREATE TABLE MovieActor (
	mid	int,
	aid	int,
	role	varchar(50),
	foreign key(mid) references Movie(id),	-- R4:movie ids are linked
	foreign key(aid) references Actor(id)	-- R5:actor ids are linked
	)
	ENGINE = INNODB;
CREATE TABLE Review (
	name	varchar(20),
	time	timestamp,
	mid	int,
	rating	int,
	comment	varchar(500),
	foreign key(mid) references Movie(id)	-- R6:movie ids aree linked
	)
	ENGINE = INNODB;
CREATE TABLE MaxPersonID(
	id	int
	)
	ENGINE = INNODB;
CREATE TABLE MaxMovieID (
	id	int
	)
	ENGINE = INNODB;

INSERT INTO MaxPersonID
	VALUES( 69000 );
INSERT INTO MaxMovieID
	VALUES( 4750 );
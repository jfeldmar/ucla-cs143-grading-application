CREATE TABLE Movie(
id int NOT NULL,
title varchar(100),
year int,
rating varchar(10),
company varchar(50),
PRIMARY KEY (id),
CHECK(year < 2009 AND id < 4750 AND title <> NULL)
) ENGINE=INNODB;
-- Every movie has a unique identification number. 
-- The year of movie should be bofore 2009.
-- All movie ids are less than 4750.
-- Every movie must have a title.
-- 1 Primary key, 1 Check constraints

CREATE TABLE Actor(
id int NOT NULL,
last varchar(20),
first varchar(20),
sex varchar(6),
dob date,
dod date,
PRIMARY KEY (id),
CHECK(id < 69000)
) ENGINE=INNODB;
-- Every actor has a unique identification number. 
-- All actor ids are less than 69000.
-- 1 Primary key, 1 Check constraints

CREATE TABLE Director(
id int NOT NULL,
last varchar(20),
first varchar(20),
dob date,
dod date,
PRIMARY KEY (id),
CHECK(id < 69000)
) ENGINE=INNODB;
-- Every director has a unique identification number.
-- All directer ids are less than 69000.
-- 1 Primary key, 1 Check constraints

CREATE TABLE MovieGenre(
mid int,
genre varchar(20),
FOREIGN KEY (mid) REFERENCES Movie(id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB;
-- A 'mid' in MovieGenre is a reference to an 'id' in Movie.
-- Movie 'id' is a parent, and MovieGenre 'mid' is a child.
-- If a Movie 'id' is deleted, then its child 'mid' tuple is also deleted.
-- Update is also. but the opposite direction is not.
-- 1 referential integrity constraint

CREATE TABLE MovieDirector(
mid int,
did int,
FOREIGN KEY (mid) REFERENCES Movie(id) ON DELETE CASCADE ON UPDATE CASCADE,
FOREIGN KEY (did) REFERENCES Director(id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB;
-- A 'mid' in MovieDirector is a reference to an 'id' in Movie on any time (delete and update).
-- Movie 'id' is a parent, and MovieDirector 'mid' is a child.
-- If a Movie 'id' is deleted or updated, then this 'mid' tuple is also deleted or updated.
-- A 'did' in MovieDirector is a reference to an 'id' in Director on any time (delete and update).
-- Director 'id' is a parent, and MovieDirector 'did' is a child.
-- If a Director 'id' is deleted or updated, then this 'did' tuple is also deleted or updated.
-- 2 referential integrity constraints

CREATE TABLE MovieActor(
mid int,
aid int,
role varchar(50),
FOREIGN KEY (mid) REFERENCES Movie(id) ON DELETE CASCADE ON UPDATE CASCADE,
FOREIGN KEY (aid) REFERENCES Actor(id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB;
-- A 'mid' in MovieActor is a reference to an 'id' in Movie on any time (delete and update).
-- Movie 'id' is a parent, and MovieActor 'mid' is a child.
-- If a Movie 'id' is deleted or updated, then this 'mid' tuple is also deleted or updated.
-- An 'aid' in MovieActor is a reference to an 'id' in Actor on any time (delete and update).
-- Actor 'id' is a parent, and MovieActor 'aid' is a child.
-- If an Actor 'id' is deleted or updated, then this 'aid' tuple is also deleted or updated.
-- 2 referential integrity constraints

CREATE TABLE Review(
name varchar(20),
time timestamp,
mid int,
rating int,
comment varchar(500),
FOREIGN KEY (mid) REFERENCES Movie(id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB;
-- A 'mid' in Review is a reference to a 'id' in Movie on any time (delete and update).
-- Movie 'id' is a parent, and Review 'mid' is a child.
-- If a Movie 'id' is deleted or updated, then this 'mid' tuple is also deleted or updated.
-- 1 referential integrity constraint

CREATE TABLE MaxPersonID(
id int
) ENGINE=INNODB;

CREATE TABLE MaxMovieID(
id int
) ENGINE=INNODB;

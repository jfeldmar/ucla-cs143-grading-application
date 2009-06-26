--Primary key constraints
--  Every movie has a unique id
--  Every actor has a unique id
--  Every director has a unique if
--Referential integrity contraints
--  MovieGenre(mid) must be in Movie
--  MovieDirector(mid) must be in Movie
--  MovieDirector(did) must benin Director
--  MovieActor(mid) must be in Movie
--  MovieActor(aid) must be in Actor
--  Review(mid) must be in Movie
--CHECK constraints
--  MaxPersonID(id) must be greater than 0
--  MaxMovieID(id) must be greater than 0
--  Review(rating) must be greater than 0 or less than or equal to 5

--Every movie has a unique id
CREATE TABLE Movie(id int PRIMARY KEY, title varchar(100), year int, rating varchar(10), company varchar(50)) ENGINE = INNODB;

--Every actor has a unique id
CREATE TABLE Actor(id int PRIMARY KEY, last varchar(20), first varchar(20), sex varchar(6), dob date, dod date) ENGINE = INNODB;

--Every director has a unique id
CREATE TABLE Director(id int PRIMARY KEY, last varchar(20), first varchar(20), dob date, dod date) ENGINE = INNODB;

--MovieGenre(mid) must be in Movie
CREATE TABLE MovieGenre(mid int, genre varchar(20), FOREIGN KEY (mid) REFERENCES Movie(id)) ENGINE = INNODB;

--MovieDirector(mid) must be in Movie
--MovieDirector(did) must be in Director
CREATE TABLE MovieDirector(mid int, did int, FOREIGN KEY (mid) REFERENCES Movie(id), FOREIGN KEY (did) REFERENCES Director(id)) ENGINE = INNODB;

--MovieActor(mid) must be in Movie
--MovieActor(aid) must be in Actor
CREATE TABLE MovieActor(mid int, aid int, role varchar(50), FOREIGN KEY (mid) REFERENCES Movie(id), FOREIGN KEY (aid) REFERENCES Actor(id)) ENGINE = INNODB;

--Review(mid) must be in Movie
--Review(rating) must be greater than 0 or less than or equal to 5
CREATE TABLE Review(name varchar(20), time timestamp, mid int, rating int, comment varchar(50), FOREIGN KEY (mid) REFERENCES Movie(id), CHECK(rating > 0 AND rating < 6)) ENGINE = INNODB;

--MaxPersonID(id) must be greater than 0
CREATE TABLE MaxPersonID(id int, CHECK(id > 0)) ENGINE = INNODB;

--MaxMovieID(id) must be greater than 0
CREATE TABLE MaxMovieID(id int, CHECK(id > 0)) ENGINE = INNODB;
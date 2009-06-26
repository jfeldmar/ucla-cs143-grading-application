CREATE TABLE Movie(id INT,
title VARCHAR(100),
year INT,
rating VARCHAR(10),
company VARCHAR(50),
UNIQUE(id),
CHECK(title<>NULL))ENGINE = INNODB;
--Every Movie id must be UNIQUE
--Every Movie must have a title

CREATE TABLE Actor(id INT,
last VARCHAR(20),
first VARCHAR(20),
sex VARCHAR(6),
dob DATE,
dod DATE,
UNIQUE(id),
CHECK(dob<>NULL))ENGINE = INNODB;
--Every Actor id must be UNIQUE
--Every Actor must have a Date of Birth

CREATE TABLE Director(id INT,
last VARCHAR(20),
first VARCHAR(20),
dob DATE,
dod DATE,
UNIQUE(id),
CHECK(dob<>NULL))ENGINE = INNODB;
--Every Director id must be UNIQUE
--Every Director must have a Date of Birth

CREATE TABLE MovieGenre(mid INT,
genre VARCHAR(20),
FOREIGN KEY(mid) REFERENCES Movie(id))ENGINE = INNODB;
--Every Movie has to have a Genre

CREATE TABLE MovieDirector(mid INT,
did INT,
FOREIGN KEY(mid) REFERENCES Movie(id),
FOREIGN KEY(did) REFERENCES Director(id))ENGINE = INNODB;
--Every Movie must have Director
--Every Director must have Directed a movie

CREATE TABLE MovieActor(mid INT,
aid INT,
role VARCHAR(50),
FOREIGN KEY(mid) REFERENCES Movie(id),
FOREIGN KEY(aid) REFERENCES Actor(id))ENGINE = INNODB;
--Every Movie must have an Actor
--Every Actor must have been in a Movie
--Every Role must be played by an Actor

CREATE TABLE Review(name VARCHAR(20),
time TIMESTAMP,
mid INT,
rating INT,
comment VARCHAR(500),
FOREIGN KEY(mid) REFERENCES Movie(id))ENGINE = INNODB;

CREATE TABLE MaxPersonID(id INT)ENGINE = INNODB;

CREATE TABLE MaxMovieID(id INT)ENGINE = INNODB;

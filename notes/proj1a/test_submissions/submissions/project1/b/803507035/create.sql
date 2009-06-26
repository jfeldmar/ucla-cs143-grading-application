-- create.sql
-- This script creates all the tables needed

--Constrains:
--1. Movie table has primary key id and id is unique, and title can't be null.  
--2. Check in Movie table: year > 1878 (when first movie ever made)
--3. Actor table has primary key id, and no actor with same first and last name have the same id. 
--4. Check in Actor table: sex is 'female' or 'male'
--5. Director table has primary key id, and no director with same first and last name have the same id
--6. Movie Genre has foreign key mid referenced from the id in the table Movie
--7. MovieDirector table has foreign key mid referenced from the id from Movie and foreign key did referenced from Director.  
--8. Check in MovieDirector: mid must be in table Movie and 
--9. did in MovieDirector must be in table Director
--10. MovieActor has foreign key mid referenced from Movie and 
--11. aid in MovieActor referenced from Actor.  
--12. Check in MovieActor:  mid must be in table Movie and 
--13. aid in MovieActor must be in table Actor

CREATE TABLE Movie(
id INT NOT NULL, 
title VARCHAR(100) NOT NULL, 
year INT, 
rating VARCHAR(10), 
company VARCHAR(50),
PRIMARY KEY(id),
CHECK(year > 1878)) ENGINE=INNODB;

CREATE TABLE Actor(
id INT NOT NULL, 
last VARCHAR(20), 
first VARCHAR(20) NOT NULL, 
sex VARCHAR(6), 
dob DATE, 
dod DATE,
PRIMARY KEY(id),
UNIQUE(id, last, first),
CHECK (sex = 'Female' OR sex = 'Male')) ENGINE=INNODB;

CREATE TABLE Director(
id INT NOT NULL, 
last VARCHAR(20), 
first VARCHAR(20) NOT NULL, 
dob DATE, 
dod DATE,
PRIMARY KEY (id),
UNIQUE (id, last, first)) ENGINE=INNODB;

CREATE TABLE MovieGenre(
mid INT NOT NULL,	 
genre VARCHAR(20),
FOREIGN KEY (mid) REFERENCES Movie (id) ON DELETE CASCADE ON UPDATE CASCADE) ENGINE=INNODB;

CREATE TABLE MovieDirector(
mid INT NOT NULL, 
did INT NOT NULL,
FOREIGN KEY (mid) REFERENCES Movie(id) ON DELETE CASCADE ON UPDATE CASCADE,
FOREIGN KEY (did) REFERENCES Director(id) ON DELETE CASCADE ON UPDATE CASCADE,
CHECK (mid IN (SELECT id FROM Movie)),
CHECK (did IN (SELECT id FROM Director))) ENGINE=INNODB;

CREATE TABLE MovieActor(
mid INT NOT NULL, 
aid INT NOT NULL, 
role VARCHAR(50),
FOREIGN KEY (mid) REFERENCES Movie(id) ON DELETE CASCADE ON UPDATE CASCADE,
FOREIGN KEY (aid) REFERENCES Actor(id) ON DELETE CASCADE ON UPDATE CASCADE,
CHECK (mid IN (SELECT id FROM Movie)),
CHECK (aid IN (SELECT id FROM Actor))) ENGINE=INNODB;

CREATE TABLE Review(
name VARCHAR(20), 
time TIMESTAMP, 
mid INT, 
rating INT, 
comment VARCHAR(500),
FOREIGN KEY (mid) REFERENCES Movie(id) ON DELETE CASCADE ON UPDATE CASCADE);

CREATE TABLE MaxPersonID(id INT);

CREATE TABLE MaxMovieID(id INT);
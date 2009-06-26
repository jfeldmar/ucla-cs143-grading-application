--tANYA gILLIS cs 143 Project 1B
--create.sql

--Each movie must have a unique identification number.
CREATE TABLE Movie (
  id      INT NOT NULL, 
  title   VARCHAR(100), 
  year    INT, 
  rating  VARCHAR(10), 
  company VARCHAR(50),
  PRIMARY KEY(id)) ENGINE=INNODB;

--Each actor must have a unique identification number.
--The sex of the actor must either be female or male.
--Each actor must have a valid date of birth.
CREATE TABLE Actor (
  id    INT NOT NULL, 
  last  VARCHAR(20), 
  first VARCHAR(20), 
  sex   VARCHAR(6), 
  dob   DATE, 
  dod   DATE,
  PRIMARY KEY(id),
  CHECK(sex = 'f' OR sex = 'm'),
  CHECK(dob <> NULL)) ENGINE=INNODB;

--Each director must have a unique identification number.
--Each director must have a valid date of birth.
CREATE TABLE Director (
  id    INT NOT NULL, 
  last  VARCHAR(20), 
  first VARCHAR(20), 
  dob   DATE,  
  dod   DATE,
  PRIMARY KEY(id),
  CHECK(dob <> NULL)) ENGINE=INNODB;

--The id represented here references the id from the Movie table.
CREATE TABLE MovieGenre (
  mid   INT,  
  genre VARCHAR(20),
  FOREIGN KEY (mid) references Movie(id)) ENGINE=INNODB;

--The mid references the Movie id. 
--The did references the Director's id.
CREATE TABLE MovieDirector (
  mid   INT, 
  did   INT,
  FOREIGN KEY (mid) references Movie(id),
  FOREIGN KEY (did) references Director(id)) ENGINE=INNODB;

--The mid references the Movie id.
--The aid references the Actor id.
CREATE TABLE MovieActor (
  mid   INT, 
  aid   INT, 
  role  VARCHAR(50),
  FOREIGN KEY (mid) references Movie(id),
  FOREIGN KEY (aid) references Actor(id)) ENGINE=INNODB;

--The mid references the Movie id.
CREATE TABLE Review (
  name    VARCHAR(20), 
  time    TIMESTAMP, 
  mid     INT, 
  rating  INT, 
  comment VARCHAR(500),
  FOREIGN KEY (mid) references Movie(id)) ENGINE=INNODB;

CREATE TABLE MaxPersonID (
  id   INT);

CREATE TABLE MaxMovieID (
  id   INT);  


CREATE TABLE Movie(
--Every movie has a unique identification number.
id INT PRIMARY KEY, 
title VARCHAR(100), 
year INT, 
rating VARCHAR(10), 
company VARCHAR(50),
--Every movie must have a title
--Every movie must have a year of production
CHECK(title != '' and year > 1900)
)ENGINE = INNODB;

CREATE TABLE Actor(
--Every actor must have a unique id number
id INT PRIMARY KEY, 
last VARCHAR(20), 
first VARCHAR(20), 
sex VARCHAR(6), 
dob DATE, 
dod DATE,
--Every actor must have a sex defined
CHECK(sex = 'Male' or sex = 'Female')
)ENGINE = INNODB;

CREATE TABLE Director(
--Every director must have a unique id number
id INT PRIMARY KEY, 
last VARCHAR(20), 
first VARCHAR(20), 
dob DATE, 
dod DATE
)ENGINE = INNODB;

CREATE TABLE MovieGenre(
mid INT, 
genre VARCHAR(20),
--Every mid in MovieGenre must reference a valid id in Movie table
FOREIGN KEY (mid) REFERENCES Movie(id)
#ON UPDATE CASCADE,
)ENGINE = INNODB;

CREATE TABLE MovieDirector(
mid INT, 
did INT,
--Every mid in MovieDirector must reference a valid id in Movie table
FOREIGN KEY (mid) REFERENCES Movie(id),
--Every did in MovieDirector must reference a valid id in Director table
FOREIGN KEY (did) REFERENCES Director(id)
#ON UPDATE CASCADE,
)ENGINE = INNODB;

CREATE TABLE MovieActor(
mid INT, 
aid INT, 
role VARCHAR(50),
--Every mid in MovieActor must reference a valid id in Director table
FOREIGN KEY (mid) REFERENCES Movie(id),
#ON UPDATE CASCADE,
--Every aid in MovieActor must reference a valid id in Actor table
FOREIGN KEY (aid) REFERENCES Actor(id)
#ON UPDATE CASCADE
)ENGINE = INNODB;

CREATE TABLE Review(
name VARCHAR(20), 
time TIMESTAMP, 
mid INT, 
rating INT, 
comment VARCHAR(500),
--Every mid in Review must reference a valid id in Movie table
FOREIGN KEY (mid) REFERENCES Movie(id),
--Every rating must be from 0 to 5
CHECK (rating > 0 AND rating < 5)
)ENGINE = INNODB;

CREATE TABLE MaxPersonID(id INT)ENGINE = INNODB;
CREATE TABLE MaxMovieID(id INT)ENGINE = INNODB;

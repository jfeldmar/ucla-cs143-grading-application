CREATE TABLE Movie(
       id INT NOT NULL,
       title VARCHAR(100) NOT NULL,
       /*has to be at most 4 digits*/
       year INT CHECK (year < 9999),
       rating VARCHAR(10),
       company VARCHAR(50),
       PRIMARY KEY(id),
       /*has to be one of the known ratings (these are made up, there could be more I don't know)*/
       CHECK (rating='R' OR rating='PG-13' OR rating='PG' OR rating='NC-17' OR rating='G')
) ENGINE=INNODB;
CREATE TABLE Actor(
       id INT NOT NULL,
       last VARCHAR(20),
       first VARCHAR(20),
       sex VARCHAR(6),
       dob DATE,
       dod DATE,
       PRIMARY KEY(id),
       /*not supporting non-male or non-female entries*/
       CHECK (sex='Male' OR sex='Female'),
       /*cannot die before born*/
       CHECK (dod > dob)
) ENGINE=INNODB;
CREATE TABLE Director(
       id INT NOT NULL,
       last VARCHAR(20),
       first VARCHAR(20),
       dob DATE,
       dod DATE,
       PRIMARY KEY(id),
       /*cannot die before born*/
       CHECK (dod > dob)
) ENGINE=INNODB;
CREATE TABLE MovieGenre(
       mid INT,
       genre VARCHAR(20),
       FOREIGN KEY(mid) REFERENCES Movie(id)
) ENGINE=INNODB;
CREATE TABLE MovieDirector(
       mid INT,
       did INT,
       FOREIGN KEY(mid) REFERENCES Movie(id),
       FOREIGN KEY(did) REFERENCES Director(id)
) ENGINE=INNODB;
CREATE TABLE MovieActor(
       mid INT,
       aid INT,
       role VARCHAR(50),
       PRIMARY KEY(mid, aid),
       FOREIGN KEY(mid) REFERENCES Movie(id),
       FOREIGN KEY(aid) REFERENCES Actor(id)
) ENGINE=INNODB;
CREATE TABLE Review(
       name VARCHAR(20),
       time TIMESTAMP,
       mid INT,
       rating INT,
       comment VARCHAR(500),
       FOREIGN KEY(mid) REFERENCES Movie(id)
) ENGINE=INNODB;
CREATE TABLE MaxPersonID(
       id INT NOT NULL,
       PRIMARY KEY(id)
) ENGINE=INNODB;
CREATE TABLE MaxMovieID(
       id INT NOT NULL,
       PRIMARY KEY(id)
) ENGINE=INNODB;
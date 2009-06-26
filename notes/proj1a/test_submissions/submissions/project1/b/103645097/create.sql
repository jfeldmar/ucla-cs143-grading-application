-- Every movie must have a unique ID
CREATE TABLE Movie (
id INT, title VARCHAR(100), year INT, rating VARCHAR(10), company VARCHAR(50),
PRIMARY KEY (id)
) ENGINE=INNODB;

-- Every actor must have a unique ID
-- If a date of death is listed for an actor, it must come *after* the actor's date of birth
CREATE TABLE Actor(
id INT, last VARCHAR(20), first VARCHAR(20), sex VARCHAR(6), dob DATE, dod DATE,
PRIMARY KEY (id),
CHECK ((dod IS NULL) or (dod > dob))
) ENGINE=INNODB;

-- Every director must have a unique ID
-- If a date of death is listed for a director, it must come *after* the director's date of birth
CREATE TABLE Director(
id INT, last VARCHAR(20), first VARCHAR(20), dob DATE, dod DATE,
PRIMARY KEY (id),
CHECK ((dod IS NULL) or (dod > dob))
) ENGINE=INNODB;

-- Every entry in MovieGenre must reference a valid Movie
CREATE TABLE MovieGenre(
mid INT, genre VARCHAR(20),
FOREIGN KEY (mid) REFERENCES Movie(id)
) ENGINE=INNODB;

-- Every entry in MovieDirector must reference a valid Movie
-- Every entry in MovieDirector must reference a valid Director
CREATE TABLE MovieDirector(
mid INT, did INT,
FOREIGN KEY (mid) REFERENCES Movie(id),
FOREIGN KEY (did) REFERENCES Director(id)
) ENGINE=INNODB;

-- Every entry in MovieActor must reference a valid Movie
-- Every entry in MovieActor must reference a valid Actor
CREATE TABLE MovieActor(
mid INT, aid INT, role VARCHAR(50),
FOREIGN KEY (mid) REFERENCES Movie(id),
FOREIGN KEY (aid) REFERENCES Actor(id)
) ENGINE=INNODB;

-- Every Review must reference a valid Movie
-- The rating number must be between 0 and 5 (inclusive)
CREATE TABLE Review(
name VARCHAR(20), time TIMESTAMP, mid INT, rating INT, comment VARCHAR(500),
FOREIGN KEY (mid) REFERENCES Movie(id),
CHECK ((rating >= 0) and (rating <= 5))
) ENGINE=INNODB;

CREATE TABLE MaxPersonID(
id INT
) ENGINE=INNODB;

CREATE TABLE MaxMovieID(
id INT
) ENGINE=INNODB;
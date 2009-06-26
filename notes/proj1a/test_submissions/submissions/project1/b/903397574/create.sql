-- Constraints:
-- Primary Key:
-- The Actor, Movie and Director tables all have IDs that are unique, and thus can be primary keys.
-- For the Review table,  we can enforce a primary key on name, time and movie id.

-- Referential Integrity:
-- For the MovieGenre, MovieDirector, MovieActor tables mid should reference Movie(id) as a foreign key.
-- Also, the MovieDirector table did should reference Director(id) as a foreign key and the MovieActor table aid should reference Actor(id) as a foreign key.
-- 
-- Check Constraints:
-- A few constraints that can be checked are that the date of birth is logically less than the date of death in both the Actor and Director tables.
-- We can also check to make sure that the movie table has a positive year value.

CREATE TABLE Movie ( id INT NOT NULL, title VARCHAR(100) NOT NULL, year INT NOT NULL, rating VARCHAR(10) NOT NULL, company VARCHAR(50) NOT NULL, PRIMARY KEY(id), CHECK (year > 0)) ENGINE = InnoDB;
CREATE TABLE Actor ( id INT NOT NULL, last VARCHAR(20) NOT NULL, first VARCHAR(20) NOT NULL, sex VARCHAR(6) NOT NULL, dob DATE NOT NULL, dod DATE NOT NULL, PRIMARY KEY(id), CHECK (dob <= dod)) ENGINE = InnoDB;
CREATE TABLE Director ( id INT NOT NULL, last VARCHAR(20) NOT NULL, first VARCHAR(20) NOT NULL, dob DATE, dod DATE, PRIMARY KEY(id), CHECK (dob <= dod)) ENGINE = InnoDB;
CREATE TABLE MovieGenre (mid INT NOT NULL, genre VARCHAR(20) NOT NULL, FOREIGN KEY mid REFERENCES Movie(id)) ENGINE = InnoDB;);
CREATE TABLE MovieDirector (mid INT NOT NULL, did INT NOT NULL, FOREIGN KEY mid REFERENCES Movie(id), FOREIGN KEY did REFERENCES Director(id),PRIMARY KEY(`mid`, `did`)) ENGINE = InnoDB;
CREATE TABLE MovieActor (mid INT, aid INT, role VARCHAR(50), FOREIGN KEY mid REFERENCES Movie(id), FOREIGN KEY aid REFERENCES Actor(id), PRIMARY KEY(`mid`, `aid`)) ENGINE = InnoDB;
CREATE TABLE Review (name VARCHAR(20) NOT NULL, time TIMESTAMP NOT NULL, mid INT NOT NULL, rating INT NOT NULL, comment VARCHAR(500) NOT NULL, FOREIGN KEY mid REFERENCES Movie(id), PRIMARY KEY(`name`,`time`,`mid`)) ENGINE = InnoDB;
CREATE TABLE MaxPersonID(id INT NOT NULL) ENGINE = InnoDB;
CREATE TABLE MaxMovieID(id INT NOT NULL) ENGINE = InnoDB;

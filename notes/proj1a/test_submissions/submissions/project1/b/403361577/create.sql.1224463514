-- Movie must have every attribute specified
-- Movie id and year must be Unique and > 0
CREATE TABLE Movie (
    id INT NOT NULL,
    title VARCHAR(100) NOT NULL,
    year INT NOT NULL,
    rating VARCHAR(10) NOT NULL,
    company VARCHAR(50) NOT NULL,
    PRIMARY KEY(id),
    CHECK(id > 0 AND year > 0))
    ENGINE = INNODB;

-- Actor must have every attribute specified except date of death
-- Actor id must be Unique and > 0
CREATE TABLE Actor (
    id INT NOT NULL,
    last VARCHAR(20) NOT NULL,
    first VARCHAR(20) NOT NULL,
    sex VARCHAR(6) NOT NULL,
    dob DATE NOT NULL,
    dod DATE,
    PRIMARY KEY(id),
    CHECK(id > 0))
    ENGINE = INNODB;

-- Director must have every attribute specified except date of death
-- Director id must be Unique and > 0
CREATE TABLE Director (
    id INT NOT NULL,
    last VARCHAR(20) NOT NULL,
    first VARCHAR(20) NOT NULL,
    dob DATE NOT NULL,
    dod DATE,
    PRIMARY KEY(id),
    CHECK(id > 0))
    ENGINE = INNODB;

-- MovieGenre must have every attribute specified
-- Movie genre refers to movie
CREATE TABLE MovieGenre (
    mid INT NOT NULL,
    genre VARCHAR(20) NOT NULL,
    -- Delete/Update movie genre when a movie is deleted.
    FOREIGN KEY(mid) REFERENCES Movie(id)
    ON UPDATE CASCADE
    ON DELETE CASCADE)
    ENGINE = INNODB;

-- MovieDirector must have every attribute specified
-- movie director refers to movie and director
CREATE TABLE MovieDirector (
    mid INT NOT NULL,
    did INT NOT NULL,
    -- Delete/Update movie director when a movie is deleted.
    FOREIGN KEY(mid) REFERENCES Movie(id)
    ON UPDATE CASCADE
    ON DELETE CASCADE,
    -- Delete/Update movie director when a director is deleted.
    FOREIGN KEY(did) REFERENCES Director(id)
    ON UPDATE CASCADE
    ON DELETE CASCADE)
    ENGINE = INNODB;

-- MovieActor must have every attribute specified
-- Movie actor refers to movie and actor
CREATE TABLE MovieActor (
    mid INT NOT NULL,
    aid INT NOT NULL,
    role VARCHAR(50),
    -- Delete/Update movie actor when a movie is deleted.
    FOREIGN KEY(mid) REFERENCES Movie(id)
    ON UPDATE CASCADE
    ON DELETE CASCADE,
    -- Delete/Update movie actor when an actor is deleted.
    FOREIGN KEY(aid) REFERENCES Actor(id)
    ON UPDATE CASCADE
    ON DELETE CASCADE)
    ENGINE = INNODB;

-- Review must have a movie id
-- review refers to a movie
CREATE TABLE Review (
    name VARCHAR(20),
    time TIMESTAMP,
    mid INT NOT NULL,
    rating INT,
    comment VARCHAR(500),
    -- Delete/Update movie genre when a movie is deleted.
    FOREIGN KEY(mid) REFERENCES Movie(id)
    ON UPDATE CASCADE
    ON DELETE CASCADE)
    ENGINE = INNODB;

-- MaxPersonID must have an id
CREATE TABLE MaxPersonID (
    id INT NOT NULL) ENGINE = INNODB;

-- MaxPersonID must have an id
CREATE TABLE MaxMovieID (
    id INT NOT NULL) ENGINE = INNODB;


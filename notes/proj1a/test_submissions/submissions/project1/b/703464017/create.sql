/*
Movie
    Constrains
        id is PRIMARY KEY
        title cannot be NULL
        year cannot be negative
        rating is in range [1,5]
*/
CREATE TABLE Movie (
    id INT PRIMARY KEY,
    title VARCHAR(100) NOT NULL,
    year INT,
    rating VARCHAR(10),
    company VARCHAR(50),
    CHECK(year >= 0),
    CHECK(rating >= 1 AND rating <= 5)) ENGINE=INNODB;
/*
Actor
    Constrains
        id is PRIMARY KEY
        last cannot be NULL
        first cannot be NULL
        dob cannot be NULL
*/
CREATE TABLE Actor (
    id INT PRIMARY KEY,
    last VARCHAR(20) NOT NULL,
    first VARCHAR(20) NOT NULL,
    sex VARCHAR(6),
    dob DATE NOT NULL,
    dod DATE) ENGINE=INNODB;

/*
Director
    Constrains
        id is PRIMARY KEY
        last cannot be NULl
        first cannot be NULL
        dob cannot be NULL
*/
CREATE TABLE Director (
    id INT PRIMARY KEY,
    last VARCHAR(20) NOT NULL,
    first VARCHAR(20) NOT NULL,
    dob DATE NOT NULL,
    dod DATE) ENGINE=INNODB;

/*
MovieGenre
    Constrains
        mid refers to Movie(id)
*/
CREATE TABLE MovieGenre (
    mid INT,
    genre VARCHAR(20),
    FOREIGN KEY(mid) REFERENCES Movie(id) ON DELETE CASCADE) ENGINE=INNODB;

/*
MovieDirector
    Constrains
        mid refers to Movie(id)
        did refers to Director(id)
*/
CREATE TABLE MovieDirector (
    mid INT,
    did INT,
    FOREIGN KEY(mid) REFERENCES Movie(id) ON DELETE CASCADE,
    FOREIGN KEY(did) REFERENCES Director(id) ON DELETE CASCADE) ENGINE=INNODB;

/*
MovieActor
    Constrains
        mid refers to Movie(id)
        aid refers to Actor(id)
*/
CREATE TABLE MovieActor (
    mid INT,
    aid INT,
    role VARCHAR(50) NOT NULL,
    FOREIGN KEY(mid) REFERENCES Movie(id) ON DELETE CASCADE,
    FOREIGN KEY(aid) REFERENCES Actor(id) ON DELETE CASCADE) ENGINE=INNODB;

/*
Review
    Constrains
        name cannot be NULL
        rating cannot be NULL
        mid refers to Movie(id)
        rating is in range [1,5]
*/
CREATE TABLE Review (
    name VARCHAR(20) NOT NULL,
    time TIMESTAMP,
    mid INT,
    rating INT NOT NULL,
    comment VARCHAR(500),
    FOREIGN KEY(mid) REFERENCES Movie(id) ON DELETE CASCADE,
    CHECK(rating >= 1 AND rating <= 5)) ENGINE=INNODB;

/*
MaxPersonID
    No constrain
*/
CREATE TABLE MaxPersonID(id INT) ENGINE=INNODB;

/*
MaxMovieID
    No constrain
*/
CREATE TABLE MaxMovieID(id INT) ENGINE=INNODB;

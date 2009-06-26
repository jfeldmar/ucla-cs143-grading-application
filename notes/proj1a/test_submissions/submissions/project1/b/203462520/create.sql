CREATE TABLE Movie (
	id INTEGER NOT NULL, -- id must contain value
	title VARCHAR(100),
	year INTEGER,
	rating VARCHAR(10),
	company VARCHAR(50),
	CHECK (id <= (SELECT id FROM MaxMovieID)), -- new id must be <= max movie id
	PRIMARY KEY(id) -- id should be unique and not null
) ENGINE=INNODB;

CREATE TABLE Actor (
	id INTEGER NOT NULL, -- must exist
	last VARCHAR(20),
	first VARCHAR(20),
	sex VARCHAR(6),
	dob DATE NOT NULL, -- must exist
	dod DATE,
	CHECK (dod > dob), -- death must be later than birth
	CHECK (id <= (SELECT id FROM MaxPersonID)), -- id <= max person id
	PRIMARY KEY(id) -- id s.b. unique and not null
) ENGINE=INNODB;

CREATE TABLE Director (
	id INTEGER NOT NULL, -- must exist
	last VARCHAR(20),
	first VARCHAR(20),
	dob DATE NOT NULL, -- must exist
	dod DATE,
	CHECK (dod > dob), -- death must be later than birth
	CHECK (id <= (SELECT id FROM MaxPersonID)), -- id m.b. <= max person id
	PRIMARY KEY(id) -- id must be unique and not null
) ENGINE=INNODB;

CREATE TABLE MovieGenre (
	mid INTEGER NOT NULL, -- must exist
	genre VARCHAR(20),
	FOREIGN KEY (mid) REFERENCES Movie(id), -- mid must exist in Movie.id
	PRIMARY KEY(mid) -- must be unique and not null
) ENGINE=INNODB;

CREATE TABLE MovieDirector (
	mid INTEGER NOT NULL, -- must exist
	did INTEGER NOT NULL, -- must exist
	FOREIGN KEY (did) REFERENCES Director(id), -- did must exist in Director.id
	FOREIGN KEY (mid) REFERENCES Movie(id) -- mid must exist in Movie.id
) ENGINE=INNODB;

CREATE TABLE MovieActor (
	mid INTEGER NOT NULL, -- must exist
	aid INTEGER NOT NULL, -- must exist
	role VARCHAR(50),
	FOREIGN KEY (aid) REFERENCES Actor(id), -- aid must exist in Actor.id
	FOREIGN KEY (mid) REFERENCES Movie(id) -- mid must exist in Movie.id
) ENGINE=INNODB;

CREATE TABLE Review (
	name VARCHAR(20),
	time TIMESTAMP,
	mid INTEGER NOT NULL, -- must exist
	rating INTEGER,
	comment VARCHAR(500) NOT NULL, -- must exist
	FOREIGN KEY (mid) REFERENCES Movie(id) -- mid must exist in Movie.id (reference real movie)
) ENGINE=INNODB;

CREATE TABLE MaxPersonID (
	id INTEGER NOT NULL, -- must exist
	CHECK (id > 0)
) ENGINE=INNODB;

CREATE TABLE MaxMovieID (
	id INTEGER NOT NULL, -- must exist
	CHECK (id > 0)
) ENGINE=INNODB;

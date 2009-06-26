CREATE TABLE Movie (id INT,
                    title VARCHAR (100),
                    year INT,
                    rating VARCHAR (10),
                    company VARCHAR (50),
  -- Every movie has a unique identification number.
                    PRIMARY KEY (id),
                    INDEX (id))
      ENGINE = INNODB;

CREATE TABLE Actor (id INT,
                    last VARCHAR (20),
                    first VARCHAR (20),
                    sex VARCHAR (6),
                    dob DATE,
                    dod DATE,
  -- Every actor has a unique identification number.
                    PRIMARY KEY (id),
  -- Every actor's dod comes after their dob or is NULL.
                    CONSTRAINT act_dates CHECK (dod > dob OR dod = NULL))
      ENGINE = INNODB;

CREATE TABLE Director (id INT,
                       last VARCHAR (20),
                       first VARCHAR (20),
                       dob DATE,
                       dod DATE,
  -- Every director has a unique identification number.
                       PRIMARY KEY (id),
  -- Every director's dod comes after their dob or is NULL.
                       CONSTRAINT dir_dates CHECK (dod > dob OR dod = NULL))
      ENGINE = INNODB;

CREATE TABLE MovieGenre (mid INT,
                         genre VARCHAR (20),
                         PRIMARY KEY (mid),
  -- Every genre references a valid movie.
                         CONSTRAINT movgen_mov FOREIGN KEY (mid) REFERENCES Movie (id))
      ENGINE = INNODB;

CREATE TABLE MovieDirector (mid INT,
                            did INT,
                            PRIMARY KEY (mid),
  -- Every movie/director references a valid movie.
                            CONSTRAINT movdir_mov FOREIGN KEY (mid) REFERENCES Movie (id),
  -- Every movie/director references a valid director.
                            CONSTRAINT movdir_dir FOREIGN KEY (did) REFERENCES Director (id))
      ENGINE = INNODB;

CREATE TABLE MovieActor (mid INT,
                         aid INT,
                         role VARCHAR (50),
  -- Every movie/actor references a valid movie.
                         CONSTRAINT movact_mov FOREIGN KEY (mid) REFERENCES Movie (id),
  -- Every movie/actor references a valid actor.
                         CONSTRAINT movact_act FOREIGN KEY (aid) REFERENCES Actor (id))
      ENGINE = INNODB;

CREATE TABLE Review (name VARCHAR (20),
                     time TIMESTAMP,
                     mid INT,
                     rating INT,
                     comment VARCHAR (500),
  -- Every review references a valid movie.
                     CONSTRAINT rev_mov FOREIGN KEY (mid) REFERENCES Movie (id),
  -- Every rating must be >= 0.
                     CONSTRAINT rev_rate CHECK (rating >= 0))
      ENGINE = INNODB;

CREATE TABLE MaxPersonID (id INT)
      ENGINE = INNODB;

CREATE TABLE MaxMovieID (id INT)
      ENGINE = INNODB;

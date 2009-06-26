CREATE TABLE MaxPersonID(id int NOT NULL);
INSERT INTO MaxPersonID VALUES (69000);

CREATE TABLE MaxMovieID(id int NOT NULL);
INSERT INTO MaxMovieID VALUES (4750);

/* The ID must be unique where two Movies cannot have the same ID.  Also, hte ID must be between and 1000000000. It also can't be negative. */
CREATE TABLE Movie(id int NOT NULL, title varchar(100) NOT NULL, year int NOT NULL, rating varchar(10) NOT NULL, company varchar(50) NOT NULL, PRIMARY KEY(id), CHECK(id >= 1 AND id <=1000000000)) ENGINE=INNODB;

/* Every Actor must have a unique ID. Also the ID must be between 1 and 1000000000. Also, it cannot be negative.*/
CREATE TABLE Actor(id int NOT NULL, last varchar(20) NOT NULL, first varchar(20) NOT NULL, sex varchar(6) NOT NULL, dob date NOT NULL, dod date, PRIMARY KEY(id), CHECK(id >= 1 AND id <=1000000000)) ENGINE=INNODB;

/* The director ID must be unique.  Also the ID must be between 1 and 1000000000. Also, it cannot be negative. */
CREATE TABLE Director(id int NOT NULL, last varchar(20) NOT NULL, first varchar(20) NOT NULL, dob date NOT NULL, dod date, PRIMARY KEY(id), CHECK(id >= 1 AND id <=1000000000)) ENGINE=INNODB;

/* Every movie ID must be in the Movie Table. Meaning the Movie must exist for a genre to be given.*/
CREATE TABLE MovieGenre(mid int NOT NULL, genre varchar(20) NOT NULL, FOREIGN KEY (mid) references Movie(id)) ENGINE=INNODB;

/* For a Movie and Director to have a relationship, both the movie and director must exist in their tables.  If both or one of them dont exist, then they cannot exist in the MovieDirector. */
CREATE TABLE MovieDirector(mid int NOT NULL, did int NOT NULL, FOREIGN KEY (mid) references Movie(id), FOREIGN KEY (did) references Director(id)) ENGINE=INNODB;

/* For a Movie and Actor to have a relationship, both the movie and Actor must exist in their tables.  If both or one of them dont exist, then they cannot
 exist in the MovieActor. */
CREATE TABLE MovieActor(mid int NOT NULL, aid int NOT NULL, role varchar(50) NOT NULL, FOREIGN KEY (mid) references Movie(id), FOREIGN KEY (aid) references Actor(id)) ENGINE=INNODB;

/* For a review to be put into the Review tables, the movie must exist. If the movie doesnt exist, no review can be given! */
CREATE TABLE Review(name varchar(20) NOT NULL, time timestamp NOT NULL, mid int NOT NULL, rating int NOT NULL, comment varchar(500), FOREIGN KEY (mid) references Movie(id)) ENGINE=INNODB;


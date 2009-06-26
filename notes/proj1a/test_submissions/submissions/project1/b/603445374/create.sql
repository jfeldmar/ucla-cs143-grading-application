CREATE TABLE Movie(id INTEGER NOT NULL, title varchar(100), year int, rating varchar(10), company varchar(50), PRIMARY KEY(id), CHECK (id > 0 & id <= MaxMovieID.id & year > 0)) ENGINE=INNODB;
--the primary key is the id since every movie must be unique, and the id and year cannot be negative, and the id cannot surpass the largest id number
CREATE TABLE Actor(id INTEGER NOT NULL, last varchar(20), first varchar(20), sex varchar(6), dob date, dod date, PRIMARY KEY(id), CHECK (id > 0 & (sex='Male' OR sex='Female'))) ENGINE=INNODB;
--the primary key is the id since every movie must be unique, and the id must be positive, and the sex of the actor must be either male or female
CREATE TABLE Director(id INTEGER NOT NULL, last varchar(20), first varchar(20), dob date, dod date, PRIMARY KEY(id)) ENGINE=INNODB;
--the primary key is the id since every director is unique
CREATE TABLE MovieGenre(mid int, genre varchar(20), FOREIGN KEY (mid) references Movie (id))  ENGINE=INNODB;
--mid must refer to a real Movie id otherwise it doesn't make sense
CREATE TABLE MovieDirector(mid int, did int, FOREIGN KEY (mid) references Movie (id), FOREIGN KEY (did) references Director (id)) ENGINE=INNODB;
--mid must refer to a real Movie id and did must refer to a real Director id
CREATE TABLE MovieActor(mid int, aid int, role varchar(50), FOREIGN KEY (mid) references Movie (id), FOREIGN KEY (aid) references Actor (id))  ENGINE=INNODB;
--mid must refer to a real Movie id and aid must refer to a real Actor id
CREATE TABLE Review(name varchar(20), time timestamp, mid int, rating int, comment varchar(500), FOREIGN KEY (mid) references Movie (id), CHECK(mid > 0 & rating >= 0 & rating < 6)) ENGINE=INNODB;
--mid must refer to a real Movie id, mid must be positive, and the rating has to be between 0 and 5
CREATE TABLE MaxPersonID(id int);
CREATE TABLE MaxMovieID(id int);
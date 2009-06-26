#---------------------------------------
# Cody Prestwood 303596543
# 18 oct 2008
#
# violate.sql checks database constraints of:
# Three primary key constraints 
# Six referential integrity constraints, and 
# Three CHECK constraints.
#
# The general structure of tables is:
# Movie(id, title, year, rating, company)
# Actor(id, last, first, sex, dob, dod)
# Director(id, last, first, dob, dod)
# MovieGenre(mid, genre)
# MovieDirector(mid, did)
# MovieActor(mid, aid, role)
# Review(name, time, mid, rating, comment)
#---------------------------------------
# Creating the Movie table with the following contraints:
# unique id as primary key, nonnull title, and verification that id is within maximum range of MaxMovieID
# CREATE table Movie(id int, title varchar(100) NOT NULL, year int, rating varchar(10), company varchar(50), primary key(id),
#  check(id > 0 and id <= (select id from MaxMovieID)) ENGINE=INNODB;

  insert into Movie values(0,"junk movie",2008);

# creating Actor table with the following constraints:
# unique id as primary key, nonnull name, and verification that id is within maximum range of MaxPersonID
# create table Actor(id int, last varchar(20) NOT NULL, first varchar(20) NOT NULL, sex varchar(6), dob date NOT NULL, 
# dod date, primary key (id),
# check(id > 0 and id <= (select id from MaxPersonID)) ENGINE=INNODB;
  insert into Movie values(0,"junk movie",2008);
  
# creating Actor table with the following constraints:
# unique id as primary key, nonnull name, and verification that id is within maximum range of MaxPersonID
 create table Director(id int, last varchar(20) NOT NULL, first varchar(20), dob date NOT NULL, dod date, primary key(id),
 check(id > 0 and id <= (select id from MaxPersonID)) ENGINE=INNODB;

# creating MovieGenre table with the following constraints:
# mid can have multiple entries, nonnull genre and foreign dependency to the Movie table for mid
 create table MovieGenre(mid int NOT NULL, genre varchar(20) NOT NULL,
 FOREIGN KEY (mid) references Movie(mid)) ENGINE=INNODB;

# creating MovieDirector table with the following constraints:
# mid can have multiple entries, did can have multiple entries, foreign dependency to the Movie table for mid
# and foreign dependency to the Director table for did
 create table MovieDirector(mid int, did int, 
 FOREIGN KEY (mid) references Movie(mid),
 FOREIGN KEY (did) references Director(id)) ENGINE=INNODB;

# creating MovieActor table with the following constraints:
# mid can have multiple entries, aid can have multiple entries, foreign dependency to the Movie table for mid
# and foreign dependency to the Actor table for aid
 create table MovieActor(mid int, aid int, role varchar(50),
 FOREIGN KEY (mid) references Movie(mid),
 FOREIGN KEY (aid) references Actor(id)) ENGINE=INNODB;

 # create Review table for storing customer reviews of movies
 create table Review(name varchar(20), time timestamp, mid int, rating int, comment varchar(500),
 FOREIGN KEY (mid) references Movie(mid)) ENGINE=INNODB;

 # create MaxPersonID table to limit the available unique id's for Actor and Director
 create table MaxPersonID(id int);
 INSERT INTO MaxPersonID VALUES(69000);

# create MaxMovieID table to limit the available unique id's for Movie
 create table MaxMovieID(id int);
 INSERT INTO MaxMovieID VALUES(4750);

show tables;
DESCRIBE Movie;
DESCRIBE Actor;
DESCRIBE Director;
DESCRIBE MovieGenre;
DESCRIBE MovieDirector;
DESCRIBE MovieActor;
DESCRIBE Review;
DESCRIBE MaxMovieID;

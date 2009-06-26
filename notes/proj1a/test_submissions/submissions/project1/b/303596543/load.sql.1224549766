#---------------------------------------
# Cody Prestwood 303596543
# 18 oct 2008
#
# load.sql load the text files into a database.
#---------------------------------------
# CREATE table Movie(id int, title varchar(100), year int, rating varchar(10), company varchar(50), primary key(id));
 LOAD DATA LOCAL INFILE 'movie.del' INTO TABLE Movie FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';

# create and add data to Actor table
# create table Actor(id int, last varchar(20), first varchar(20), sex varchar(6), dob date, dod date, primary key (id));
 LOAD DATA LOCAL INFILE 'actor1.del' INTO TABLE Actor FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';
 LOAD DATA LOCAL INFILE 'actor2.del' INTO TABLE Actor FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';
 LOAD DATA LOCAL INFILE 'actor3.del' INTO TABLE Actor FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';

# create table Director(id int, last varchar(20), first varchar(20), dob date, dod date, primary key(id));
 LOAD DATA LOCAL INFILE 'director.del' INTO TABLE Director FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';

# create table MovieGenre(mid int, genre varchar(20),primary key(mid));
 LOAD DATA LOCAL INFILE 'moviegenre.del' INTO TABLE MovieGenre FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';

# create table MovieDirector(mid int, did int, primary key(mid,did));
 LOAD DATA LOCAL INFILE 'moviedirector.del' INTO TABLE  MovieDirector FIELDS TERMINATED BY ',' ;

# create table MovieActor(mid int, aid int, role varchar(50),primary key(mid,aid));
 LOAD DATA LOCAL INFILE 'movieactor1.del' INTO TABLE MovieActor FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';
 LOAD DATA LOCAL INFILE 'movieactor2.del' INTO TABLE MovieActor FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"';

# create table Review(name varchar(20), time timestamp, mid int, rating int, comment varchar(500));

# create table MaxPersonID(id int);
# INSERT INTO MaxPersonID VALUES(69000);

# create table MaxMovieID(id int);
# INSERT INTO MaxMovieID VALUES(4750);

USE CS143

create table Movie (
id int not null,
title varchar(100) not null,
year int,
rating varchar(10),
company varchar(50),
-- each movie must have a unique id
primary key (id),
-- each movie must have a standardized rating
check (rating='G' OR rating='PG' OR rating="PG-13" OR
	  rating='R' OR rating='NC-17' OR rating="UR")) ENGINE = INNODB;

create table Actor (
id int not null,
last varchar(20),
first varchar(20),
sex varchar(6),
dob date,
dod date,
-- each actor or director must have a unique id
primary key (id),
-- sex must be Male or Female
check (sex='Male' OR sex='Female')) ENGINE = INNODB;

create table Director (
-- actors and directors share a unique set of ids
id int references Actor,
last varchar(20),
first varchar(20),
dob date,
dod date,
-- each actor and director must have a unique id
primary key (id)) ENGINE = INNODB;

create table MovieGenre (
mid int not null,
genre varchar(20) not null,
-- no movie can be listed with the same genre more than once
primary key (mid, genre),
-- there should be no entries in MovieGenre for a movie not in the database
foreign key (mid) references Movie (id)) ENGINE = INNODB;

create table MovieDirector (
mid int not null,
did int,
primary key (mid, did),
-- there should only be MovieDirector entries for movies in the database
foreign key (mid) references Movie (id),
-- there should only be MovieDirector entries for directors in the database
foreign key (did) references Director (id)) ENGINE = INNODB;

create table MovieActor (
mid int not null,
aid int not null,
role varchar(50),
-- there should be no 2 actors playing the same role in the same movie
primary key (mid, role),
-- there should be no MovieActor entry for a movie not in the database
foreign key (mid) references Movie (id),
-- there should be no MovieActor entry for an actor not in the database
foreign key (aid) references Actor (id)) ENGINE = INNODB;

create table Review (
name varchar(20) not null,
time timestamp,
mid int not null,
rating int,
comment varchar(500),
-- the same person can't review the same movie more than once
primary key (name, mid),
-- there should only be reviews for movies in the database
foreign key (mid) references Movie (id),
-- rating should be only 0,1,2,3,4 or 5
check (rating >= 0 AND rating <= 5)) ENGINE = INNODB;

create table MaxPersonID (id int) ENGINE = INNODB;

create table MaxMovieID (id int) ENGINE = INNODB;

--Movie table containing the movie ID as its primary key; ID must be at least 1
create table Movie(id int, title varchar(100), year int, rating varchar(10), company varchar(50),
PRIMARY KEY (id),
CHECK(id >= 1))
ENGINE=INNODB;

--Actor table containing the actor ID as its primary key; ID must be at least 1
create table Actor(id int, last varchar(20), first varchar(20), sex varchar(6), dob date, dod date,
PRIMARY KEY (id),
CHECK(id >= 1))
ENGINE=INNODB;

--Director table containing the director ID as its primary key; ID must be at least 1
create table Director(id int, last varchar(20), first varchar(20), dob date, dod date,
PRIMARY KEY (id),
CHECK(id >= 1))
ENGINE=INNODB;

--Movie Genre table with foreign key linked to movie ID
create table MovieGenre(mid int, genre varchar(20),
FOREIGN KEY(mid) references Movie(id))
ENGINE=INNODB;

--Movie director table with foreign keys linked to movie ID and director ID
create table MovieDirector(mid int, did int,
FOREIGN KEY (mid) references Movie(id),
FOREIGN KEY (did) references Director(id))
ENGINE=INNODB;

--Movie actor table with foreign keys linked to movie ID and actor ID
create table MovieActor(mid int, aid int, role varchar(50),
FOREIGN KEY (mid) references Movie(id),
FOREIGN KEY (aid) references Actor(id))
ENGINE=INNODB;

--Review table with foreign key linked to movie ID
create table Review(name varchar(20), time timestamp, mid int, rating int, comment varchar(500),
FOREIGN KEY (mid) references Movie(id))
ENGINE=INNODB;

create table MaxPersonID(id int);

create table MaxMovieID(id int);
create table Movie(id INT, title VARCHAR(100), year INT, rating VARCHAR(10), company VARCHAR(50), PRIMARY KEY(id))ENGINE=INNODB;
create table Actor(id INT, last VARCHAR(20), first VARCHAR(20), sex VARCHAR(6), dob DATE, dod DATE, PRIMARY KEY(id), CHECK(sex='MALE' OR sex='FEMALE'), CHECK(dob < dod))ENGINE=INNODB;
create table Director(id INT, last VARCHAR(20), first VARCHAR(20), dob DATE, dod DATE, PRIMARY KEY(id), CHECK(dob < dod))ENGINE=INNODB;
create table MovieGenre(mid INT, genre VARCHAR(20), UNIQUE(mid, genre), FOREIGN KEY (mid) references Movie(id))ENGINE=INNODB;
create table MovieDirector(mid INT, did INT, UNIQUE(mid, did), FOREIGN KEY (mid) references Movie(id), FOREIGN KEY (did) references Director(id))ENGINE=INNODB;
create table MovieActor(mid INT, aid INT, role VARCHAR(50), UNIQUE(mid, aid), FOREIGN KEY (mid) references Movie(id), FOREIGN KEY (aid) references Actor(id))ENGINE=INNODB;
create table Review(name VARCHAR(20), time TIMESTAMP, mid INT, rating INT, comment VARCHAR(500), FOREIGN KEY (mid) references Movie(id))ENGINE=INNODB;
create table MaxPersonID(id INT)ENGINE=INNODB;
create table MaxMovieID(id INT)ENGINE=INNODB;


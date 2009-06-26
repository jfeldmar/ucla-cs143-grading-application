create table Movie (
	id int not null, title varchar(100) not null, year int, rating varchar(10), company varchar(50),
	primary key (id)
--	check(0 < year and year < 2009)
	)ENGINE = INNODB;
-- primary key's been added & id and title cannot be null
create table Actor (
	id int not null, last varchar(20), first varchar(20), sex varchar(6), dob date not null, dod date,
	primary key (id)
--	check(sex<>'male' or sex<>'female')
--	check(0 < dob and dob < 20090101)
	)ENGINE = INNODB;
-- primary key's been added & id and dob cannot be null
create table Director (
	id int not null, last varchar(20), first varchar(20), dob date not null, dod date,
	primary key (id)
--	check(0 < dob and dob < 20090101)
	)ENGINE = INNODB;
-- primary key's been added & id and dob cannot be null
create table MovieGenre (
	mid int, genre varchar(20),
	foreign key (mid) references Movie(id)
	)ENGINE = INNODB;
-- foreign key's been added to the mid references to the Movie's id
create table MovieDirector (
	mid int, did int,
	foreign key (mid) references Movie(id),
	foreign key (did) references Director(id)
	)ENGINE = INNODB;
-- foreign key's been added to the mid references to the Movie's id
-- foreign key's been added to the did references to the Director's id
create table MovieActor (
	mid int, aid int, role varchar(50),
	foreign key (mid) references Movie(id),
	foreign key (aid) references Actor(id)
	)ENGINE = INNODB;
-- foreign key's been added to the mid references to the Movie's id
-- foreign key's been added to the aid references to the Actor's id
create table Review (
	name varchar(20) not null, time timestamp, mid int, rating int, comment varchar(500),
	foreign key (mid) references Movie(id)
	)ENGINE = INNODB;
-- foreign key's been added to the mid references to the Movie's id
create table MaxPersonID (
	id int
	)ENGINE = INNODB;
create table MaxMovieID (
	id int
	)ENGINE = INNODB;

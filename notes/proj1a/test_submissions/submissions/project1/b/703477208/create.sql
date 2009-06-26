create table Movie (
	id 	integer NOT NULL,
	title 	varchar(100) not null,
	year	integer,
	rating	varchar(10),
	company varchar(50),
	PRIMARY KEY(id),
-- check to make sure rating is in the given set only
	check(rating in ('G','PG','PG-13','R','NC-17'))
	) ENGINE=INNODB;
create table Actor (
	id 	integer not NULL,
	last 	varchar(20) not null,
	first 	varchar(20) not null,
	sex 	varchar(6),
-- check to see that sex entered is only between the given set
	check( sex in ('male','female')),
	dob 	date	not NULL,
	dod     date,
	PRIMARY KEY(id),
	-- checks to see if Actor and Director is the same
	-- then they have same id
	check ( id in (select d.id from Director d where first
	      = d.first and last=d.last))
	) ENGINE=INNODB; 
create table Director (
	id 	integer not null,
	last 	varchar(20), 
	first 	varchar(20),
	dob	date	not NULL,
	dod	date,
	PRIMARY KEY(id),
	-- checks to see if Actor and Director is the same
	-- then they have same id
	check ( id in (select a.id from Actor a where first
	      = a.first and last=a.last))
	) ENGINE=INNODB;
create table MovieGenre (
	mid	integer not null,
	genre	varchar(20),
	FOREIGN KEY(mid) REFERENCES Movie(id)
	) ENGINE=INNODB;
create table MovieDirector (
	mid	integer not null,
	did	integer	not null,
	FOREIGN KEY(mid) REFERENCES Movie(id),
	FOREIGN KEY(did) REFERENCES Director(id)
	-- every movie has a director
	-- check not exists (select distinct M.id from Movie M)-
	-- MovieDirector.mid
	) ENGINE=INNODB;
create table MovieActor (
	mid	integer not null,
	aid	integer not null,
	role	varchar(50),
	FOREIGN KEY(mid) REFERENCES Movie(id),
	FOREIGN KEY(aid) REFERENCES Actor(id)
	) ENGINE=INNODB;
create table Review (
	name 	varchar(20) not null,
	time 	timestamp,
	mid	integer not null,
	rating	integer,
	comment	varchar(500),
	FOREIGN KEY(mid) REFERENCES Movie(id)
	) ENGINE=INNODB;
create table MaxPersonID (
	id	integer not null
	) ENGINE=INNODB;
create table MaxMovieID (
	id	integer not null

-- PLEASE NOTE
-- preferably FOREIGN KEY should be there, however, since the 
-- value 4750 is not there in Movie, therefore it returns an error
--	FOREIGN KEY(id) REFERENCES Movie(id)

	) ENGINE=INNODB;

insert into MaxPersonID values (69000);
insert into MaxMovieID values (4750);
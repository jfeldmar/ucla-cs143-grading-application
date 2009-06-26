create table MaxMovieID(id int); 
create table MaxPersonID(id int);

create table Director(id int, last varchar(20), first varchar(20), dob date check(date < curdate()), dod date, primary key (id)) engine = innodb;
 
--set id as the primary key
--checks date of birth is in the past

create table Actor(id int, last varchar(20), first varchar(20), sex varchar(6) check(sex = 'female' or sex = 'male' or sex = 'Male' or sex = 'Female'),dob date, dod date check(dod = '' or dod > dob), primary key (id)) engine = innodb;
--set id as the primary key
--checks sex is male or female
--checks date of death is after date of birth


create table Movie(id int, title varchar(100), year int, rating varchar(10), company varchar(50), primary key (id)) engine = innodb;
 
--set id as the primary key

create table Review(name varchar(20), time timestamp, mid int references Movie(id), rating int, comment varchar(500)) engine = innodb;

--mid is set to reference movie's id

create table MovieActor(mid int references Movie(id), aid int references Actor(id), roll varchar(50)) engine = innodb; 
--mid is set to reference movie's id
--aid is set to reference Actor's id

create table MovieGenre(mid int references Movie(id), genre varchar(20)) engine = innodb; 
--mid is set to reference movie's id

create table MovieDirector(mid int references Movie(id), did int references Director(id)) engine = innodb; 
--mid is set to reference movie's id
--did is set to reference director's id


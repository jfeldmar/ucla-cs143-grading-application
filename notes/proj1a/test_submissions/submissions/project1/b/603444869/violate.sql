--Primary Key Violations:
insert into Movie (id,title,year,rating,company) values (2,'blah',2008,4,'test');
--there is already a movie with id 2.  contradicts the primary key constraint.
--ERROR 1062 (23000): Duplicate entry '2' for key 1

insert into Actor (id,last,first,sex,dob,dod) values (1,'blah','blah','MALE',2008-02-03,2008-10-01);
--there is already an actor with id 1.  contradicts the primary key constraint.
--ERROR 1062 (23000): Duplicate entry '1' for key 1

insert into Director (id,last,first,dob,dod) values (16,'blah','blah',2008-02-03,2008-10-01);
--there is already a director with id 16.  contradicts the primary key constraint.
--ERROR 1062 (23000): Duplicate entry '16' for key 1

--Foreign Key Violations:
insert into MovieGenre (mid,genre) values (999999,'Horror');
--inserts a Movie id that does not exist into MovieGenre
--ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

insert into MovieDirector (mid,did) values (999998,25);
--inserts a Movie id that does not exist into MovieDirector
--ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

insert into MovieDirector (mid,did) values (2,13);
--inserts a Director id that does not exist into MovieDirector
--ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

insert into MovieActor (mid,aid,role) values (999988, 100, 'dummy');
--insert a Movie id into MovieActor that doesn't exist
--ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

insert into MovieActor (mid,aid,role) values (2, 1111111, 'dummy');
--insert an Actor id into MovieActor that doesn't exist
--ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

insert into Review (name, time, mid, rating) values (2, '2008-02-01 00:00:00', 9999982, 4);
--insert a Movie id into Review that doesn't exist
--ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

--Check Violations:
insert into Actor (id,last,first,sex,dob,dod) values (0,'blah','blah','SMALE',2008-02-03,2008-10-01);
--cannot insert an Actor whose sex is 'SMALE'. violates a check condition.

insert into Actor (id,last,first,sex,dob,dod) values (0,'blah','blah','SMALE',2008-02-03,2008-02-01);
--cannot insert an Actor who has a dob that is > dod. 

insert into Director (id,last,first,dob,dod) values (0,'blah','blah',2008-02-03,2008-02-01);
--cannot insert a Director who has a dob that is > dod. 


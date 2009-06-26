--  violate.sql
--  My Project has 
----  Three primary key constraints
----  Six Referential Integrity Constraints, and
----  4 Triggers


--  Statements that will violate the three primary key constraints
insert into Actor values (1,"A","B","Male",1999-01-12,NULL);
--  ERROR 1062 (23000): Duplicate entry '1' for key 1
----  Primary key keeps unique entries
insert into Director values (16,"A","B",1999-01-12,NULL);
--  ERROR 1062 (23000): Duplicate entry '16' for key 1
----  Primary key keeps unique entries
insert into Movie values (2,"A",2000,"R","ME");
--  ERROR 1062 (23000): Duplicate entry '2' for key 1
----  Primary key keeps unique entries


--  Statements that will violate the six referential key constraints

insert into MovieGenre values (1,"Horror");
--  ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`) ON DELETE CASCADE)
----  Id number 1 is not in Movie
insert into MovieDirector values (2,1);
--  ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_2` FOREIGN KEY (`did`) REFERENCES `Director` (`id`))
----  Id number 1 is not in Director
insert into MovieActor values (1,1,"haha");
--  ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`) ON DELETE CASCADE)
----  Id number 1 is no in Movie
insert into MovieActor values (2,0,"haha");
--  ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieActor`, CONSTRAINT `MovieActor_ibfk_2` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))
----  ID number 0 is not in Actor
insert into Review values ("Me",now(),1,2,"none");
--  ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`) ON DELETE CASCADE)
----  ID number 1 is not in Movie
insert into MovieDirector values (1,16);
--  ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`) ON DELETE CASCADE)
----  ID number 1 is not in Movie


--  Statements that will violate my two triggers

insert into Review values ("Me",now(),2,6,"none");
--  ERROR 1442 (HY000): Can't update table 'Review' in stored function/trigger because it is already used by statement which invoked this stored function/trigger.
----  Rating must be <1 or >5

insert into Review values ("Til There was you Reivew",now(),2,5,"good");
--  Query OK, 1 row affected, 1 warning (0.02 sec)
Update Review set rating = 6;
--  ERROR 1442 (HY000): Can't update table 'Review' in stored function/trigger because it is already used by statement which invoked this stored function/trigger.
----  First needed to test the second statement,  update can't change the rating to something else other than between 1 and 5
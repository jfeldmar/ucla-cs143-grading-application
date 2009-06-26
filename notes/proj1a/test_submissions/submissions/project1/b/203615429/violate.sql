/*violate*/
/*Chi Kin Cheang ~ 203-615-429*/


/*
mid in MovieActor must be the subset of id in Movie.
aid in MovieActor must be the subset of id in Actor.
*/
INSERT INTO MovieActor VALUES (1000000, 1000000, 'Mother');
UPDATE MovieActor SET mid = 1000000 WHERE mid = 2 AND aid = 162;
UPDATE MovieActor SET aid = 1000000 WHERE mid = 2 AND aid = 162;
/*
#1452 - Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
*/


/*
mid in MovieDirector must be the subset of id in Movie.
did in MovieDirector must be the subset of id in Director.
*/
INSERT INTO MovieDirector VALUES (1000000, 1000000);
UPDATE MovieDirector SET mid = 1000000 WHERE mid = 3 AND did = 112;
UPDATE MovieDirector SET aid = 1000000 WHERE mid = 3 AND aid = 112;
/*
#1452 - Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
*/


/*
mid in MovieGenre must be the subset of id in Movie.
*/
INSERT INTO MovieGenre VALUES (1000000, 'Comedy');
UPDATE MovieGenre SET mid = 1000000 WHERE mid = 2;
/*
#1452 - Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
*/


/*
mid in Review must be the subset of id in Movie.
*/
INSERT INTO Review VALUES ('Sunny', NOW(), 1000000, 10, 'This is comment');
/*
#1452 - Cannot add or update a child row: a foreign key constraint fails (`CS143/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
*/

/*
id in Actor is unique
*/
INSERT INTO Actor VALUES (1, 'Sunny', 'Cheang', 'Male', '1999-1-1', NULL);
UPDATE Actor SET id = 1 WHERE id = 10;
/*
#1062 - Duplicate entry '1' for key 1
*/


/*
id in Movie is unique
*/
INSERT INTO Movie VALUES (2, 'Sunny', 1999, 'PG-13', 'abc');
/*
#1062 - Duplicate entry '2' for key 1
*/

/*
id in Director is unique
*/
INSERT INTO Director VALUES (76, 'Sunny', 'Cheang', '1999-1-1', NULL);
/*
#1062 - Duplicate entry '76' for key 1
*/


/*
Cannot update/delete referenced table without setting ON DELETE/ON UPDATE.
*/
UPDATE Movie SET id =2 WHERE id = 3;
DELETE FROM Movie WHERE id = 2;
/*
#1451 - Cannot delete or update a parent row: a foreign key constraint fails (`CS143/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
*/
UPDATE Actor SET id =99 WHERE id = 12;
DELETE FROM Actor WHERE id = 12;
/*
#1451 - Cannot delete or update a parent row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))
*/
UPDATE Director SET id =79 WHERE id = 76;
DELETE FROM Director WHERE id = 76;
/*
#1451 - Cannot delete or update a parent row: a foreign key constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`did`) REFERENCES `Director` (`id`))
*/



/*
id in Movie must not greater than the id in MaxMovieID. 
*/
INSERT INTO Movie VALUES (100000, 'abc', '1999', 'PG-13', 'bca');


/*
id in Actor must not greater than the id in MaxPersonID.
sex can be either Male or Female.
date of birth must earlier than date of death.
*/
INSERT INTO Actor VALUES (100000, 'Sunny', 'Cheang', 'Dont know', '1999-1-1', '1988-1-1');


/*
id in Director must not greater than the id in MaxPersonID.
date of birth must earlier than date of death.
*/
INSERT INTO Director VALUES (100000, 'Sunny', 'Cheang', '1999-1-1', '1988-1-1');
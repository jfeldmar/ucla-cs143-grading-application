INSERT INTO Movie VALUES (999999999, 'cs143TESTMOVIE', 2008, 'G', 'SEASNET');
INSERT INTO Movie VALUES (999999999, 'cs143TESTMOVIE', 2008, 'G', 'SEASNET');
/*
 This is one of the primary key constraints to check that there are no duplicates in the movie ID field.  As in the real world, it may be possible that two movies have the same title.  Whether this is legal or not, if there are two seperate movies(even with the same titles), their movie IDs must be different.  So this checks that there are no duplicate movie IDs. I inserted a movie with a random movie ID and I inserted the same movie with the same ID again.  This would not be possible so it gives the following error:

ERROR 1062 (23000): Duplicate entry '999999999' for key 1


*/

INSERT INTO Actor VALUES (999999999, 'Song', 'Andre', 'Male', '1984-13-07', '2090-12-07');
INSERT INTO Actor VALUES (999999999, 'Song', 'Andre', 'Male', '1984-13-07', '2090-12-07');
/*
This is the second primary key constraint that checks that the same actor does not get added to the database twice.  In the real world, it is possible that two actors have the same name.  However, if they are two different people, they must have different actor IDs.  This checks that the IDs are never the same.  I once again inserted a random actor with a random ID and I inserted the same actor again.  The first insertion works but the second insertion gives the following error:

ERROR 1062 (23000): Duplicate entry '999999999' for key 1

*/

INSERT INTO Director VALUES (999999999, 'Song', 'Andre', '1984-13-07', '2090-12-07');
INSERT INTO Director VALUES (999999999, 'Song', 'Andre', '1984-13-07', '2090-12-07');
/*
This is the third primary key constraint that checks that the same director is not added twice.  In the real world, it is possible for two directors to have the same name but their IDs must be different.  I once again added a random director and added the same director once again.  The first insertion works but the second one gives an error because it already exists in the table.  The error is as follows:

ERROR 1062 (23000): Duplicate entry '999999999' for key 1

*/

INSERT INTO MovieDirector VALUES (999999991, 1234567);
INSERT INTO MovieDirector VALUES (999999999, 1234567111111);
/*This is the first 2 referential integrity constraints.  First, I inserted a query where the movie does not exist in the Movie Table.  If there is no movie existing, a director cannot obviously direct this non-existing movie.  This gives the following error:

ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint f
ails (`TEST/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

I also inserted a query where the movie does exist(from the previous insertions) but the director does not exist in the Director Table.This is pretty obvious that a director that does not exist will not be able to direct the movie.  So adding a director into teh MovieDirector when he/she doesn't exist in the Director Table will not work. It produces the following error:

ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint f
ails (`TEST/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_2` FOREIGN KEY (`did`) REFERENCES `Director` (`id`))

*/

INSERT INTO MovieActor VALUES (-45729357, 12345668970911, 'PEASANT');
INSERT INTO MovieActor VALUES (-87596055, 12345668970911, 'PEASANT');
/*
This is third and fourth referential integrity checks.  First, I inserted a query where the movie does not exist.  Since the movie does not exist in the Movie Table, a director should not be able to direct a non-existing movie.  It gives the following error;

ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

The fourth referential integrity check is when the movie does exist(from the previous insertions) but the director does not exist in the Director Table.  A movie cannot be directed by a non-existing director! So it gives the following error: 

ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint f
ails (`TEST/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

*/
INSERT INTO Review VALUES ('AndreSong', '19:02', -45735028, 1234, 'no comment');
/*
The fifth referential integrity check is when a review is inserted where the movie does not exist in the Movie Table.  The user should not be able to give a review about a non-existing movie.  So the following error is given:

ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint f
ails (`TEST/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `
Movie` (`id`))

*/
INSERT INTO MovieGenre VALUES (-38558301, 'dont care');
/*
The sixth referential integrity check is when a genre is given to a movie that does not exist in the Movie Table.  If a movie doesn't exist, the user should not be able to specify a genre for a non-existing movie!  So the following error is given:

ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint f
ails (`TEST/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFE
RENCES `Movie` (`id`))

*/

INSERT INTO Movie VALUES (1000000001, 'cstester', 2008, 'R', 'SEASNET');
INSERT INTO Actor VALUES (1000000002, 'cs', 'tester', 'male', '1987-12-06', '2007-12-06');
INSERT INTO Director VALUES (-9998, 'cs1', 'tester1', '1987-12-06', '2007-12-06');
/* The last three queries are Insertions into the tables Movie, Actor, and Director. Since the id numbers for all of these are too large or too small, they will not work.  The range for the ids are 1 to 1,000,000,000.  In the real world, a person's ID should not be negative and it should not be over 1 million.  After one million IDs have been used, the range can be increased but until it is, there is no need for the id numbers to be greater than 1,000,000,000.  1 million numbers is sufficient enough for all the people(i would think so).
It produces the following since MySQL does not support CHECK.

Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.02 sec)

*/
  



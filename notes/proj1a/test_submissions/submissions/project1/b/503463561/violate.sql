-- violate.sql: a list of possible database modifications that violate the
-- integrity constraints we put in place when the database was created

--------------------------------------------------------------------------------
-- Primary key constraints
--------------------------------------------------------------------------------

-- Attempting to add a movie with id 4700 here violates the primary key
-- constraint in Movie (which has a primary key of id), since the movie id 4700
-- is already taken by "Zooman" (1995).

INSERT INTO Movie
	VALUES (4700,"10 Things I Hate About You",1999,"PG-13","Touchstone Pictures");

-- Resulting error:
-- ERROR 1062 (23000) at line 12: Duplicate entry '4700' for key 1

-- Attempting to add an actor with no actor ID results in an error, since
-- a primary key cannnot have a NULL value. This would violate the primary key
-- constraint in Actor (which has a primary key of id)

INSERT INTO Actor
	VALUES( NULL, "Doe","John","Male","March 2, 1999",NULL);

-- Resulting error:
-- ERROR 1048 (23000) at line 19: Column 'id' cannot be null

-- Attempting to change the primary key of a tuple in Director (id) to that of
-- an existing tuple will result in an error.

UPDATE Director
	SET id = 16
	WHERE id = 76;

-- Resulting error:
-- ERROR 1062 (23000) at line 25: Duplicate entry '16' for key 1

--------------------------------------------------------------------------------
-- Referential integrity constraints
--------------------------------------------------------------------------------

-- Attempting to add a tuple in the MovieGenre table that doesn't refer to an
-- existing movie results in an error, since the key 314159265 doesn't exist in
-- the Movie table.

INSERT INTO MovieGenre(mid,genre)
	VALUES (314159265,"Nonsense");

-- Resulting error:
-- ERROR 1452 (23000) at line 37: Cannot add or update a child row: a foreign
-- key constraint fails (`CS143/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1`
-- FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- Attempting to remove a tuple from the Movie table that has an entry in
-- MovieGenre should result in an error, since the foreign key in the
-- corresponding MovieGenre table would be invalid.

DELETE FROM Movie
	WHERE title = "Inspector Gadget 2";

-- Resulting error:
-- ERROR 1451 (23000) at line 44: Cannot delete or update a parent row: a
-- foreign key constraint fails (`CS143/MovieGenre`, CONSTRAINT
-- `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- Attempting to change a tuple in the Movie table to have a different (valid)
-- primary key will result in an error, since the primary key of Movie is also
-- the foreign key used by MovieGenre, MovieActor, and MovieDirector to keep
-- track of movies.

UPDATE Movie
	SET id = 314159265
	WHERE title = "Inspector Gadget 2";

-- Resulting error:
-- ERROR 1451 (23000) at line 52: Cannot delete or update a parent row: a
-- foreign key constraint fails (`CS143/MovieGenre`, CONSTRAINT
-- `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- Inserting unused mid and did into MovieDirector results in a foreign key error.

INSERT INTO MovieDirector VALUES (-5,-6);

-- Resulting error:
-- ERROR 1452 (23000) at line 58: Cannot add or update a child row: a foreign
-- key constraint fails (`CS143/MovieDirector`, CONSTRAINT
-- `MovieDirector_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- Inserting unused aid only into MovieActor results in a foreign key error
-- (even if the mid is valid).

INSERT INTO MovieActor VALUES (3938,100000003590,"Droid");

-- Resulting error:
-- ERROR 1452 (23000) at line 63: Cannot add or update a child row: a foreign
-- key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_2`
-- FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))

-- Inerting a mid not present in Movie into a tuple in Review causes an error.

INSERT INTO Review VALUES ("John Smith","January 5, 2008",9999999999,1,"What a
turkey!");

-- Resulting error:
-- ERROR 1452 (23000) at line 67: Cannot add or update a child row: a foreign
-- key constraint fails (`CS143/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY
-- (`mid`) REFERENCES `Movie` (`id`))

--------------------------------------------------------------------------------
-- CHECK contraints (commented out to prevent data corruption)
--------------------------------------------------------------------------------

-- Inserting a new tuple into Movie with year <= 1876 (the year before the first
-- movie ever created, the "moving horse") should result in an error if CHECK
-- were properly implemented.
/*
INSERT INTO Movie VALUES(4750,"The Time Machine",1875,"PG","Wells Incorporated");
*/
-- Modifying a review to have a negative score should also result in an error
-- with a proper implementation of CHECK.
/*
UPDATE Review
	SET rating = -rating;
*/
-- Adding an actor that was born after he/she died should result in an error.
/*
INSERT INTO Actor VALUES(69000,"Button","Benjamin","January 10, 2008","February
12, 1954");
*/


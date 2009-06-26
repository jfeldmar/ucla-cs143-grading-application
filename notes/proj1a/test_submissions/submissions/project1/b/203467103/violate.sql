-- violate.sql

-- Every movie must have a title
INSERT INTO Movie
	VALUES(43515555, NULL, 1995, "R", "Movie Co.");
-- Column 'title' is null
-- ERROR 1048 (23000): Column 'title' cannot be null

-- Every actor must have a date of birth
INSERT INTO Actor
	VALUES(4442222, "Smith", "Haggis", "Male", NULL, NULL);
-- Column 'dob' is null
-- ERROR 1048 (23000): Column 'dob' cannot be null

-- Every actor must have a first name
INSERT INTO Actor
	VALUES(4442223, "Smith", NULL, "Male", date(19991010), NULL);
-- Column 'first' is null
-- ERROR 1048 (23000): Column 'first' cannot be null
	
-- Each director must have a date of birth
INSERT INTO Director
	VALUES(4442224, "Smith", "Haggis", "Male", NULL, NULL);
-- Column 'dob' is null
-- ERROR 1048 (23000): Column 'dob' cannot be null

-- Every director must have a first name
INSERT INTO Director
	VALUES(4442225, "Smith", NULL, "Male", date(19991010), NULL);
-- Column 'first' is null
-- ERROR 1048 (23000): Column 'first' cannot be null
	

-- Every movie has a unique identification number
INSERT INTO Movie
	VALUES(2, "A movie", 1995, "R", "Movie Co.");
-- This is trying to insert a movie with the id of 2 into the table, but id 2
-- is taken
-- ERROR 1062 (23000): Duplicate entry '2' for key 1

-- Every actor has a unique identification number
INSERT INTO Actor
	VALUES(1, "Smith", "Haggis", "Male", date(19991010), NULL);
-- This is trying to insert an actor with the id of 1 into the table, but id 1
-- is taken
-- ERROR 1062 (23000): Duplicate entry '1' for key 1

-- Every director has a unique identification number
INSERT INTO Director
	VALUES(16, "Smith", "Haggis", "Male", date(19991010), NULL);
-- This is trying to insert a director with the id of 16 into the table, but id 16
-- is taken
-- ERROR 1062 (23000): Duplicate entry '16' for key 1

-- Every MovieActor entry must correspond with an existing actor
INSERT INTO MovieActor
	VALUES(1836, 4442226, "Psychic Chef");
-- This references actor 4442226, who does not exist
-- ERROR 1452 (23000): Cannot add or update a child row:
-- a foreign key constraint fails ('CS143/MovieActor',
-- CONSTRAINT 'MovieActor_ibfk_2' FOREIGN KEY ('aid')
-- REFERENCES 'Actor' ('id'))

-- Every MovieActor entry must correspond with an existing movie
INSERT INTO MovieActor
	VALUES(4442221, 1, "Psychic Chef");
-- This references movie 4442221, which does not exist in the movie table
-- SIMILAR ERROR MESSAGE TO THE ONE ABOVE

-- Every MovieDirector entry must correspond with an existing director
INSERT INTO MovieDirector
	VALUES(1836, 4442227);
-- This references director 4442227, who does not exist
-- SIMILAR ERROR MESSAGE TO THE ONE ABOVE
	
-- Every MovieDirector entry must correspond with an existing movie
INSERT INTO MovieDirector
	VALUES(-1, 16)
-- This references movie -1, which does not exist
-- SIMILAR ERROR MESSAGE TO THE ONE ABOVE
	
-- Every MovieGenre entry must correspond with an existing movie
INSERT INTO MovieGenre
	VALUES(-1, "Zombie");
-- This references movie -1, which does not exist
-- SIMILAR ERROR MESSAGE TO THE ONE ABOVE
	
-- Every Review entry must correspond with an existing movie
INSERT INTO Review
	VALUES("xXCoolio1224Xx", 123, -1, 10, "pretty good");
-- This is a review for the movie with the id -1, which does not exist
	
-- An actor cannot die before they were born
INSERT INTO Actor
	VALUES(4442228, "Smith", "Haggis", "Male", date(19991010), date(19801010));
-- The date of death of this actor is before the date of birth
	
-- A director cannot die before they were born
INSERT INTO Director
	VALUES(4442229, "Smith", "Haggis", "Male", date(19991010), date(19801010));
-- The date of death of this director is before the date of birth
	
-- A movie's year cannot be in the future
INSERT INTO Movie
	VALUES(441232131, "A movie", 3000, "R", "Movie Co.");
-- This movie's year is in the future

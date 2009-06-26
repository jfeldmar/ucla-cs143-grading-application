-- Name: Angel Darquea
-- ID: 003512936
-- Date: 10/19/2008
-- Fall 2008
-- Project 1B - violate.sql

-- Modifications that violate primary key constrains:
-- -- This attempts to insert a movie with an ID that belongs to another movie (Zooman)
INSERT INTO Movie VALUES(4700, 'Rambo vs Rocky', 2008, 'NC-17', 'CrazyAmesome Films');
-- -- This attempts to insert a director with an ID that belongs to another director (Zyablikova, Aida)
INSERT INTO Director VALUES(68626, 'Mouse', 'Mighty', '19000912', '19990912');
-- -- This attempts to insert an actor with an ID that belongs to another actor (Alvarez, Marian)
INSERT INTO Actor VALUES(68635, 'Mouse', 'Mighty', 'Male', '19000912', '19990912');

-- Modifications that violate referential integrity constrains:
-- -- This attempts to insert a MovieGenre record for a mid that does not exist in the Movie table
INSERT INTO MovieGenre VALUES(4800, 'Adventure');
-- -- This attempts to insert a MovieDirector record for a mid that does not exist in the Movie table
INSERT INTO MovieDirector VALUES(4800, 68626);
-- -- This attempts to insert a MovieDirector record for a did that does not exist in the Director table
INSERT INTO MovieDirector VALUES(4700, 69999);
-- -- This attempts to insert a MovieActor record for a mid that does not exist in the Movie table
INSERT INTO MovieActor VALUES(4800, 68635, 'Mayor');
-- -- This attempts to insert a MovieActor record for a aid that does not exist in the Actor table
INSERT INTO MovieActor VALUES(4700, 69837, 'Mail man');
-- -- This attempts to insert a Review record for a mid that does not exist in the Movie table
INSERT INTO Review VALUES('Angel', 20070101103004, 4800, 5, 'Movie was very good');
-- -- This attempts to delete an Actor who is referenced in other tables (e.g. MovieActor)
DELETE FROM Actor WHERE id = 58668;
-- -- This attempts to delete a Movie which is referenced in other tables (e.g. MovieActor)
DELETE FROM Movie WHERE id = 4700;

-- Modifications that violate CHECK constrains:
-- -- This attempts to insert a MovieGenre record with an invalid Genre
INSERT INTO MovieGenre VALUES(4700, 'Sci-Fun');
-- -- This attempts to insert an actor with a negative ID
INSERT INTO Actor VALUES(-68635, 'Mouse', 'Mighty', 'Male', '19000912', '19990912');
-- -- This attempts to insert an actor with an invalid sex value that belongs to another actor (Alvarez, Marian)
INSERT INTO Actor VALUES(68635, 'Mouse', 'Mighty', 'Nope', '19000912', '19990912');































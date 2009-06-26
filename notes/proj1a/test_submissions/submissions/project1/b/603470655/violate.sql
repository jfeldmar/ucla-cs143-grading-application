-- Use Appropriate Database
USE CS143;

-- 1) Violate PRIMARY KEY constraint: add a movie entry with
--    a movie id that already exists within the Movie table
INSERT INTO Movie (id) Value (2);
-- ERROR 1062 (23000): Duplicate entry '2' for key 1

-- 2) Violate CHECK constraint: add a movie entry with a
--    negative value for the year column
INSERT INTO Movie (year) Value (-2008);

-- 3) Violate PRIMARY KEY constraint: add an actor entry with
--    an actor id that already exists within the Actor table
INSERT INTO Actor (id) Value (1);
-- ERROR 1062 (23000): Duplicate entry '1' for key 1

-- 4) Violate CHECK constraint: add an actor entry with a
--    dob value greater than the dod value
INSERT INTO Actor (dob, dod) Value (20080101, 20070101);

-- 5) Violate PRIMARY KEY constraint: add a director entry with
--    a director id that already exists within the Director table
INSERT INTO Director (id) Value (16);
-- ERROR 1062 (23000): Duplicate entry '16' for key 1

-- 6) Violate CHECK constraint: add a director entry with a
--    dob value greater than the dod value
INSERT INTO Director (dob, dod) Value (20080101, 20070101);

-- 7) Violate FOREIGN KEY constraint: add an entry to the
--    MovieGenre table with an mid value that does not equal
--    any id value within the Movie table
INSERT INTO MovieGenre (mid) Value (0);
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key
-- constraint fails ('CS143/MovieGenre', CONSTRAINT 'MovieGenre_ibfk_1'
-- FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))

-- 8) Violate FOREIGN KEY constraint: add an entry to the
--    MovieDirector table with an mid value that does not equal
--    any id value within the Movie table
INSERT INTO MovieDirector (mid) Value (0);
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key
-- constraint fails ('CS143/MovieDirector', CONSTRAINT 'MovieDirector_ibfk_1'
-- FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))

-- 9) Violate FOREIGN KEY constraint: add an entry to the
--    MovieDirector table with an mid value that corresponds with
--    an id value within the Movie table, yet also a did value that
--    does not equal any id value within the Director table
INSERT INTO MovieDirector (mid, did) Value (2, 0);
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key
-- constraint fails ('CS143/MovieDirector', CONSTRAINT 'MovieDirector_ibfk_2'
-- FOREIGN KEY ('did') REFERENCES 'Director' ('id'))

-- 10) Violate FOREIGN KEY constraint: add an entry to the
--    MovieActor table with an mid value that does not equal
--    any id value within the Movie table
INSERT INTO MovieActor (mid) Value (0);
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key
-- constraint fails ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_1'
-- FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))

-- 11) Violate FOREIGN KEY constraint: add an entry to the
--    MovieActor table with an mid value that corresponds with
--    an id value within the Movie table, yet also an aid value
--    that does not equal any id value within the Actor table
INSERT INTO MovieActor (mid, aid) Value (2, 0);
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key
-- constraint fails ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_2'
-- FOREIGN KEY ('aid') REFERENCES 'Actor' ('id'))

-- 12) Violate FOREIGN KEY constraint: add an entry to the
--    Review table with an mid value that does not equal
--    any id value within the Movie table
INSERT INTO Review (mid) Value (0);
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key
-- constraint fails ('CS143/Review', CONSTRAINT 'Review_ibfk_1'
-- FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))

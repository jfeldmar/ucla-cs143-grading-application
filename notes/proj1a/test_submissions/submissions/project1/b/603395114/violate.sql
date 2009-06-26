-- Constrainst:
-- Primary Key
-- 1. The movie id should be unique and therefore a primary key.
INSERT INTO Movie VALUES
(SELECT * FROM Movie LIMIT 1);
-- 2. The reviewer name, tile and movie id should uniquely identify  a review.
INSERT INTO Review VALUES
(SELECT * FROM Review LIMIT 1);
-- 3. The id in the actor table should be a primary key.
INSERT INTO Actor VALUES
(SELECT * FROM Actor LIMIT 1);
-- 4. The id in the director table should be a primary key.
INSERT INTO Director VALUES
(SELECT * FROM Director LIMIT 1);
-- 
-- Referential Integrity
-- 1.  In the MovieGenre table mid should reference Movie(id) as a foreign key.
--	VIOLATION: Inserting a tuple for which the mid does not exist as an id in the Movie table.
INSERT INTO MovieGenre VALUES
((SELECT (SELECT * FROM MaxMovieID LIMIT 1) + 1), 'foobar');
-- 2. In the MovieDirector table mid should reference Movie(id) as a foreign key.
--	VIOLATION: Inserting a tuple for which the mid does not exist as an id in the Movie table.
INSERT INTO MovieDirector VALUES
((SELECT (SELECT * FROM MaxMovieID LIMIT 1) + 1), 1);
-- 3. In the MovieActor table mid should reference Movie(id) as a foreign key.
--	VIOLATION: Inserting a tuple for which the mid does not exist as an id in the Movie table.
INSERT INTO MovieActor VALUES
((SELECT (SELECT * FROM MaxMovieID LIMIT 1) + 1), 1);
-- 4. In the Review table mid should reference Movie(id) as a foreign key.
-- 5. In the MovieDirector table did should reference Director(id) as a foreign key.
--	VIOLATION: Inserting a tuple for which the did does not exist as an id in the Director table.
INSERT INTO MovieDirector VALUES
(1,(SELECT (SELECT * FROM MaxPersonID LIMIT 1) + 1));
-- 6. In the MovieActor table aid should reference Actor(id) as a foreign key.
--	VIOLATION: Inserting a tuple for which the aid does not exist as an id in the Actor table.
INSERT INTO MovieActor VALUES
(1,(SELECT (SELECT * FROM MaxPersonID LIMIT 1) + 1));
-- 
-- Check Constraints
-- 1.  Check that dob is logically less than dod in the Director table.
INSERT INTO Director VALUES
((SELECT * FROM MaxPersonID LIMIT 1),'John','Doe',CURRENT_DATE(), '2000-02-02');
-- 2. Check that dob is logically less than dod in the Actor table.
INSERT INTO Actor VALUES
((SELECT * FROM MaxPersonID LIMIT 1),'John','Doe',CURRENT_DATE(), '2000-02-02');
-- 3. Check that the year in the Movie table is positive.
INSERT INTO Movie Values
((SELECT * FROM MaxMovieID LIMIT 1),'FOOBAR',-2,'N/A', 'FUBAR INC');
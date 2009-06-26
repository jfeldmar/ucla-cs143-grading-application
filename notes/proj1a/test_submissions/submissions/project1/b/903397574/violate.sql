--Constraints
--Primary Key: The following statements will violate the primary key constraints as they each try to insert a key that is not unique to the database
INSERT INTO Movie VALUES(SELECT * FROM Movie LIMIT 1);
INSERT INTO Review VALUES(SELECT * FROM Review LIMIT 1);
INSERT INTO Actor VALUES(SELECT * FROM Actor LIMIT 1);
INSERT INTO Director VALUES(SELECT * FROM Director LIMIT 1);
-- 
-- Referential Integrity
-- This violates referential integrity as this tries to insert a non-valid mid into the MovieGenre, MovieDirector and MovieActor tables
INSERT INTO MovieGenre VALUES (4800, 'violation');
INSERT INTO MovieDirector VALUES(4800, 'violation');
INSERT INTO MovieActor VALUES(4800, 'violation');
INSERT INTO Review VALUES('violation', CURRENT_DATE(), 4800);
INSERT INTO MovieDirector VALUES('violation', 70000);
INSERT INTO MovieActor VALUES('violation', 70000);
-- 
-- Check Constraints
-- 1.  Check that dob is logically less than dod in the Director table.
INSERT INTO Director VALUES(4250,'Jessica','Bruin','2000-03-10', '2004-03-10');
-- 2. Check that dob is logically less than dod in the Actor table.
INSERT INTO Actor VALUES(4250,'Jessica','Bruin','2000-03-10', '2004-03-10');
-- 3. Check that the year in the Movie table is positive.
INSERT INTO Movie Values(4800, 'CS143 Movie', -1942, 5, 'PKVSEED, INC');
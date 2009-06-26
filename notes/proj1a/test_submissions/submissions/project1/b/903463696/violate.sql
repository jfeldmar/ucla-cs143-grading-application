--Tanya Gillis CS 143 Project 1B
--violate.sql

--Violates the primary key set in Movie; it cannot be NULL since it is unique.
UPDATE Movie
SET id=NULL
WHERE year=2000;
--Returns error: ERROR 1062 (23000): Duplicate entry '0' for key 1.

--Violates the primary key set in Actor; it cannot be NULL since it is unique.
UPDATE Actor
SET id=NULL
WHERE sex='Male';
--Returns error: ERROR 1062 (23000): Duplicate entry '0' for key 1.

--Violates CHECK statement in Actor; we were checking that sex was 'm' or 'f'.
--Inserting a NULL would violate this condition.
INSERT INTO Actor VALUES (100, 'Brown', 'Chris', NULL, 5/5/1989, NULL);

--Violates CHECK statement in Actor; we were checking that dob was not NULL.
--Inserting a NULL would violate this condition.
INSERT INTO Actor VALUES (100, 'Brown', 'Chris', 'm', NULL, NULL);

--Violates primary key set in Director; id cannot be NULL since it is unique.
UPDATE Director
SET id=NULL
WHERE sex='Male';
--Returns error: ERROR 1062 (23000): Duplicate entry '0' for key 1.

--Violates CHECK statement in Director; we were checking that dob was not NULL.
--Inserting NULL here violates that condition.
INSERT INTO Director VALUES (100, 'Spielberg', 'Steven', NULL, NULL);

--Violates foreign key constraint because we deleted from Movie but not from
--MovieActor, even though that mid references the Movie id.
DELETE FROM Movie
WHERE id < 20;
--Returns error: ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))

--Violates foreign key constraint because we deleted from Actor but not from
--MovieActor, even though aid references the Actor id.
DELETE FROM Actor
WHERE id < 20;
--Returns error: ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_2' FOREIGN KEY ('aid') REFERENCES 'Actor' ('id'))

--Violates foreign key constraint because we deleted from Director but not from
--MovieDirector, even though aid references the Actor id.
DELETE FROM Director
WHERE id < 20;
--Returns error: ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint ('CS143/MovieDirector', CONSTRAINT 'MovieDirector_ibfk_1' FOREIGN KEY ('did') REFERENCES 'Director' ('id'))

--Violates foreign key constraint because we updated Actor but not MovieActor
--and aid references the Actor id.
UPDATE Actor
SET id=1
WHERE id=20;
--Returns error: ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_3' FOREIGN KEY ('aid') REFERENCES 'Actor' ('id'))

--Violates foreign key constraint because we updated Movie but not MovieActor
--and mid references the Movie id.
UPDATE Movie
SET id=1
WHERE id=20;
--Returns error: ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_4' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))

--Violates foreign key constraint because we updated Director but not 
--MovieDirector and did references the Director id.
UPDATE Director
SET id=1
WHERE id=20;
--Returns error: ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint ('CS143/MovieDirector', CONSTRAINT 'MovieDirector_ibfk_1' FOREIGN KEY ('did') REFERENCES 'Director' ('id'))




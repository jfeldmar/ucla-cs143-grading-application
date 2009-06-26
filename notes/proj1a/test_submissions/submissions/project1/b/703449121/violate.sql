INSERT INTO Movie VALUES (2024, 'Independence Array', 1996, 'PG-13', '20th Century Fox');
-- This statement violates the constraint of all Movies having unique ids. This tuple shares the same id as the movie 'Independence Day'.
-- The output from this statement is: ERROR 1062 (23000) at line 1: Duplicate entry '2024' for key 1

INSERT INTO Actor VALUES (58144, 'Smith', 'Bill', NULL, '1968-09-25', NULL);
-- This statement violates the constraint of all Actors having unique ids. This tuple shares the same id as actor Will Smith.
-- The output from this statement is: ERROR 1062 (23000) at line 5: Duplicate entry '58144' for key 1

INSERT INTO Actor VALUES (69000, 'Moy', 'Raymond', 'Malew', '1988-03-18', NULL);
-- This statement violates the constraint of all Actors having sex = 'Male' OR sex = 'Female' OR sex = NULL. This tuple has sex = 'Malew'.

INSERT INTO Actor VALUES (69001, 'Reaper', 'Grim', 'Male', '2008-01-01', '1950-01-01');
-- This statement violates the constraint of all Actors having dod after the dob. This tuple has dod (1950-01-01) after dob (2008-01-01).

UPDATE Actor
SET first = 'Raymond', last = 'Moy'
WHERE id =18363;
-- This statement violates the constraint of Actors and Directors having the same id if and only if they are the same person.
-- The tuple that this statement updates will have the same id as Director Roland Emmerich, but the attributes first and last will be different.

INSERT INTO Director VALUES (37931, 'Lucas', 'Jorge', '1944-05-14', NULL);
-- This statement violates the constraint of all Actors having unique ids. This tuple shares the same id as Director George Lucas.
-- The output from this statement is: ERROR 1062 (23000) at line 21: Duplicate entry '37931' for key 1

INSERT INTO Director VALUES (69001, 'Reaper', 'Grim', '2008-01-01', '1950-01-01');
-- This statement violates the constraint of all Directors having dod after the dob. This tuple has dod (1950-01-01) after dob (2008-01-01).

UPDATE Director
SET first = 'Jeffrey', last = 'Su'
WHERE id =37931;
-- This statement violates the constraint of Actors and Directors having the same id if and only if they are the same person.
-- The tuple that this statement updates will have the same id as Actor George Lucas, but the attributes first and last will be different.

INSERT INTO MovieGenre VALUES (2024, 'Action');
-- This statement violates the constraint of all tuples in MovieGenre being unique. This tuple is a duplicate of a tuple already in MovieGenre.
-- The output from this statement is: ERROR 1062 (23000) at line 34: Duplicate entry '2024-Action' for key 1

INSERT INTO MovieGenre VALUES (999, 'Action');
-- This statement violates the constraint of all mids in MovieGenre referencing an id from Movie. This tuple's mid is not found as an id in Movie.
-- The output from this statement is: ERROR 1452 (23000) at line 38: Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_ 1`FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

INSERT INTO MovieDirector VALUES (999,18363);
-- This statement violates the constraint of all mids in MovieDirector referencing an id from Movie. This tuple's mid is not found as an id in Movie.
-- The output from this statement is: ERROR 1452 (23000) at line 42: Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_ 1`FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

INSERT INTO MovieDirector VALUES (2024, 999);
-- This statement violates the constraint of all dids in MovieDirector referencing an id from Director. This tuple's mid is not found as an id in Movie.
-- The output from this statement is: ERROR 1452 (23000) at line 46: Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_ 2`FOREIGN KEY (`did`) REFERENCES `Director` (`id`))

INSERT INTO MovieActor VALUES (999,37931, 'Pirate');
-- This statement violates the constraint of all mids in MovieActor referencing an id from Movie. This tuple's mid is not found as an id in Movie.
-- The output from this statement is: ERROR 1452 (23000) at line 50: Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieActor`, CONSTRAINT `MovieActor_ibfk_ 1`FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

INSERT INTO MovieActor VALUES (2024, 65000, 'Pirate');
-- This statement violates the constraint of all aids in MovieActor referencing an id from Actor. This tuple's aid is not found as an id in Actor.
-- The output from this statement is: ERROR 1452 (23000) at line 54: Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieActor`, CONSTRAINT `MovieActor_ibfk_ 2`FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))

INSERT INTO Review VALUES ('Danny Choi', '2000-01-01 12:34:56', 1074, -1, 'I hate this movie.  It never should have been made.  The acting is lame.  Therefore, it gets a -1.');
-- This statement violates the constraint of all ratings in Review having values from 0 to 5. This tuple's ratings value is -1.

INSERT INTO Review VALUES ('Danny Choi', '2000-01-01 12:34:56', 999, 0, 'I hate this movie.  It never should have been made.  The acting is lame.  Therefore, it gets a 0.');
-- This statement violates the constraint of all mids in Review referencing an id from Movie. This tuple's mid, 999, is not found as an id in Movie.
-- The output from this statement is: ERROR 1452 (23000) at line 61: Cannot add or update a child row: a foreign key constraint fails (`TEST/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
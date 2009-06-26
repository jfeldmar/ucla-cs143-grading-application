INSERT INTO Movie VALUES (3, 'Random Movie', 1988, 'Mature', 'Production Company');
--violates primary key for Movie since 3 is an id that is already used and id has to be unique
--output: ERROR 1062 (23000): Duplicate entry '3' for key 1

INSERT INTO Actor VALUES (10, 'Franco', 'James', 'Male', 1988-01-21, '2000-10-20');
--violates primary key for Actor since 10 is an id that is already used and id has to be unique
--output: ERROR 1062 (23000): Duplicate entry '10' for key 1

INSERT INTO Director VALUES (146, 'Scrosese', 'Martin', 1956-11-30, '1958-01-01');
--violates primary key for Director since 146 is an id that is already used and id has to be unique
--output: ERROR 1062 (23000): Duplicate entry '146' for key 1

DELETE FROM Movie WHERE id=3;
---violates the referential integrity where MovieGenre mid=3 has to refer to Movie id=3
--output: ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint fails (`CS143/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

DELETE FROM Movie WHERE id=126;
---violates the referential integrity where MovieDirector mid=126 has to refer to Movie id=126
--output: ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint fails (`CS143/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

UPDATE Director SET id=4 WHERE id=9255
--violates the referential integrity where MovieDirector did=9255 referred to Director id=9255
--output: ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_2` FOREIGN KEY (`did`) REFERENCES `Director` (`id`))

INSERT INTO MovieActor VALUES(7, 205, 'Protagonist');
--violates the referntial integrity where MovieActor mid has to refer to a real id in Movie, but id=7 does not exist in Movie
--output: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

DELETE FROM Actor WHERE id=19;
--violates the referntial integrity of MovieActor because MovieActor aid has to refer to Actor id=19
--output: ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_2` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))

INSERT INTO Review VALUES('Jono', 1:20:50, 7, 2, 'My Review');
--violates the referential integrity of Review because mid should refer to a real movie, but Movie id=7 is not in the db
--output: ERROR 1064 (42000): You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near ':20:50, 7, 2, 'My')' at line 1

INSERT INTO Movie VALUES (-1, 'Random Movie', -1988, 'Mature', 'Production Company');
--violates Movie CHECK statement because both id and year cannot be negative

INSERT INTO Actor VALUES (10, 'Franco', 'James', 'Alien', 1988-01-21, '\N');
--violates Actor CHECK statement because sex must be either male or female

INSERT INTO Review VALUES('Jono', 1:20:50, 7, -2, 'My Review');
--violates Review CHECK statement because Review must be between 0 and 5
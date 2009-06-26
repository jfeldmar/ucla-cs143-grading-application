-- Violates the constraint that Actor IDs must be unique
INSERT INTO Actor VALUES (1, 'Black', 'Jack', 'Male', NULL, NULL);
--ERROR 1048 (23000): Column 'dob' cannot be null

-- Violates the constraint that a movie must have a title
INSERT INTO Movie VALUES (7, NULL, 2004, 'PG-13', 'Metro-Goldwyn-Mayer (MGM)');
--ERROR 1048 (23000): Column 'title' cannot be null

-- Violates the constraint that a movie must have a year
INSERT INTO Movie VALUES (975, 'Tropic Thunder', NULL, 'R', 'Metro-Goldwyn-Mayer (MGM)');
--ERROR 1048 (23000): Column 'year' cannot be null

-- Violates the constraint MovieActor entries must refer to a valid movie
INSERT INTO MovieActor VALUES (24234234, 10208, 'Pool Boy');
--ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails 
--(`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- Violates the constraint MovieActor entries must refer to a valid actor
INSERT INTO MovieActor VALUES (1002, 4512181, 'George H.W. Bush');
--ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails 
--(`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_2` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))

-- Violates the constraint MovieDirector entries must refer to a valid movie
INSERT INTO MovieDirector VALUES (24234234, 112);
--ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails
--(`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- Violates the constraint MovieDirector entries must refer to a valid actor
INSERT INTO MovieDirector VALUES (1002, 2134685);
--ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails
--(`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_2` FOREIGN KEY (`did`) REFERENCES `Director` (`id`))

-- Violates the constraint MovieGenre entries must refer to a valid movie
INSERT INTO MovieGenre VALUES (24234234, 'Comedy');
--ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails
--(`CS143/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- Violates the constraint Reviews must refer to a valid movie
INSERT INTO Review VALUES ('Rob', NULL, 24234234, 3, 'Great Movie!');
--ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails
--(`CS143/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))


-- Violates the constraint Every Movie ID must be greater than zero
INSERT INTO Movie VALUES (-5, 'Good Movie', 2004, 'PG-13', 'Metro-Goldwyn-Mayer (MGM)');

-- Violates the constraint Every Actor ID must be greater than zero
INSERT INTO Actor VALUES (-54, 'Black', 'Jack', 'Male', NULL, NULL);

-- Violates the constraint Every Director ID must be greater than zero
INSERT INTO Director VALUES (-54, 'Black', 'Jack', 'Male', NULL);

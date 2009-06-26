-- examples of each type of violations are shown below.

-- NOT NULL violations
-- here are some examples of it

-- Movie id must have a value
-- ERROR 1048 (23000): Column 'id' cannot be null
INSERT INTO Movie VALUES (\N, 'a', 1, 'a', 'a');
-- Movie title must have a value
-- ERROR 1048 (23000): Column 'title' cannot be null
INSERT INTO Movie VALUES (1, \N, 1, 'a', 'a');
-- Movie year must have a value
-- ERROR 1048 (23000): Column 'year' cannot be null
INSERT INTO Movie VALUES (1, 'a', \N, 'a', 'a');
-- Movie rating must have a value
-- ERROR 1048 (23000): Column 'rating' cannot be null
INSERT INTO Movie VALUES (1, 'a', 1, \N, 'a');
-- Movie company must have a value
-- ERROR 1048 (23000): Column 'company' cannot be null
INSERT INTO Movie VALUES (1, 'a', 1, 'a', \N);


-- 3 Primary key violations

-- Movie id must be unique
-- ERROR 1062 (23000): Duplicate entry '4734' for key 1
INSERT INTO Movie VALUES (4734, 'a', 1, 'a', 'a');
-- Actor id must be unique
-- ERROR 1062 (23000): Duplicate entry '68635' for key 1
INSERT INTO Actor VALUES (68635, 'a', 'a', 'a', 1978-04-01, \N);
-- Director id must be unique
-- ERROR 1062 (23000): Duplicate entry '68626' for key 1
INSERT INTO Director VALUES (68626, 'a', 'a', 1940-03-16, \N);


-- 6 Referential violations

-- can't insert a non-existent movie
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`) ON DELETE CASCADE ON UPDATE CASCADE)
INSERT INTO MovieGenre VALUES (4735, 'Foreign Test');
-- can't insert a non-existent movie
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`) ON DELETE CASCADE ON UPDATE CASCADE)
INSERT INTO MovieDirector VALUES (4735, 68626);
-- can't insert a non-existent director
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_2` FOREIGN KEY (`did`) REFERENCES `Director` (`id`) ON DELETE CASCADE ON UPDATE CASCADE)
INSERT INTO MovieDirector VALUES (4734, 68627);
-- can't insert a non-existent movie
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`) ON DELETE CASCADE ON UPDATE CASCADE)
INSERT INTO MovieActor VALUES (4735, 68635, 'Foreign Test');
-- can't insert a non-existent actor
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_2` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`) ON DELETE CASCADE ON UPDATE CASCADE)
INSERT INTO MovieActor VALUES (4734, 68636, 'Foreign Test');
-- can't insert a non-existent movie
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`CS143/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`) ON DELETE CASCADE ON UPDATE CASCADE)
INSERT INTO Review VALUES('Foreign Test', 1, 4735, 1, 'Foreign Test');


-- 3 Check constraint violations

-- Movie id must be positive and year
-- However, MySQL doesn't support CHECK constraints, so there's no error report for this.
INSERT INTO Movie VALUES (-1, 'a', -1, 'a', 'a');
-- Actor id must be positive
-- However, MySQL doesn't support CHECK constraints, so there's no error report for this.
INSERT INTO Actor VALUES (-68635, 'a', 'a', 'a', 1978-04-01, \N);
-- Director id must be positive
-- However, MySQL doesn't support CHECK constraints, so there's no error report for this.
INSERT INTO Director VALUES (-68626, 'a', 'a', 1940-03-16, \N);


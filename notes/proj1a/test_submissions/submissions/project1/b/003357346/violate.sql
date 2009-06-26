-- Primary constraint #1 violation
-- Violates the primary key constraint (id) for Movie. Cannot have duplicates
-- Duplicate entry '200' for key 1
INSERT INTO Movie VALUES (200, 'My movie', 2000, 'R', 'Company');

-- Primary constraint #2 violation
-- Violates the primary key constraint (id) for Actor. Cannot have duplicates
-- Duplicate entry '2' for key 1
INSERT INTO Actor VALUES (2, 'Hsieh', 'Albert', 'M', 1987-04-23, NULL);

-- Primary constraint #3 violation
-- Violates the primary key constraint (id) for Director. Cannot have duplicates
-- Duplicate entry '63' for key 1
INSERT INTO Director VALUES (63, 'Hsieh', 'Albert', 1987-04-23, NULL);

-- Referential Constraint #1
-- Violates foreign key constraint (mid) for MovieGenre. mid must exist in Movie
-- Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`) ON DELETE CASCADE ON UPDATE CASCADE)
INSERT INTO MovieGenre VALUES (1, 'Comedy');

-- Referential Constraint #2
-- Violates foreign key constraint (mid) for MovieDirector. mid must exist in Movie
-- Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`) ON DELETE CASCADE ON UPDATE CASCADE)
INSERT INTO MovieDirector VALUES (1, 16);

-- Referential Constraint #3
-- Violates foreign key constraint (did) for MovieDirector. did must exist in Director
-- Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_2` FOREIGN KEY (`did`) REFERENCES `Director` (`id`) ON DELETE CASCADE ON UPDATE CASCADE)
insert into MovieDirector values (3, 1);

-- Referential Constraint #4
-- Violates foreign key constraint (mid) for MovieActor. mid must exist in Movie
-- Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`) ON DELETE CASCADE ON UPDATE CASCADE)
INSERT INTO MovieActor VALUES (1, 52831, 'Police');

-- Referential Constraint #5
-- Violates foreign key constraint (mid) for MovieActor. aid must exist in Actor
-- Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_2` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`) ON DELETE CASCADE ON UPDATE CASCADE)
INSERT INTO MovieActor VALUES (3230, 0, 'Police');

-- Referential Constraint #6
-- Violates foreign key constraint (mid) for Review. mid must exist in Movie
-- Cannot add or update a child row: a foreign key constraint fails (`CS143/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`) ON DELETE CASCADE ON UPDATE CASCADE)
INSERT INTO Review VALUES ('Albert', '2008-10-19 16:24:00', 0, NULL, 'Good Movie');

-- Check constraint #1 violation
-- Violates the check constraint (id) for Movie. ID must be > 0
INSERT INTO Movie VALUES (-1, 'My movie', 2000, 'R', 'Company');

-- Check constraint #2 violation
-- Violates the check constraint (id) for Actor. ID must be > 0
INSERT INTO Actor VALUES (-1, 'Hsieh', 'Albert', 'M', 1987-04-23, NULL);

-- Check constraint #3 violation
-- Violates the check constraint (id) for Director. ID must be > 0
INSERT INTO Director VALUES (-1, 'Hsieh', 'Albert', 1987-04-23, NULL);
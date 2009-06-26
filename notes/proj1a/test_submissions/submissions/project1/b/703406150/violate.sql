USE CS143
-- primary key constraint 1
-- Each movie should have a unique ID
insert into Movie values 
(123456789, 'Batman Begins', 2005, 'PG-13', 'Warner Bros.'),
(123456789, 'The Dark Knight', 2008, 'PG-13', 'Warner Bros.');
-- This violates the Primary key constraint from Movie because
-- no two movies can have the same ID.
-- ERROR 1062 (23000): Duplicate entry '123456789' for key 1

-- primary key constraint 2
-- no two actors can play the same role in the same movie
insert into MovieActor values
(2345678, 54321, 'Bruce Wayne'),
(2345678, 98765, 'Bruce Wayne');
-- This violates the primary key constraint from MovieActor because
-- no two actors can play the same role in the same movie.
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key 
-- constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` 
-- FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- primary key constraint 3
-- no two actors can have the same id
insert into Actor values
(31415926, 'Freeman', 'Morgan', 'Male', '1950-01-01', NULL),
(31415926, 'Oldman', 'Gary', 'Male', '1960-05-05', NULL);
-- This violates the primary key clause from Actor becuase
-- it attempts to add two actors with the same ID.
-- ERROR 1062 (23000): Duplicate entry '31415926' for key 1

-- referential integrity constraint 1
-- Movies should not have a genre if they don't have a title
insert into MovieGenre values
(-123, 'Musical');
-- This violates the referencial constraint from MovieGenre because
-- there is no movie with id = -123.
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key
-- constraint fails (`CS143/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1`
-- FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- referential integrity constraint 2
-- Movies not in the database shouldn't have reviews for them
insert into Review values
('Mike', '2008-10-18 12:00:00', -1, 0, 'This movie sucked!');
-- This violates the referencial constraint from Review because
-- there is no movie with id = -1.
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key
-- constraint fails (`CS143/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN
-- KEY (`mid`) REFERENCES `Movie` (`id`))

-- referential integrity constraint 3
-- MovieActor entries should be only for movies in the database
insert into MovieActor values
(-50, 12345, 'Captain America');
-- This violates the referencial constraint from MovieActor because
-- there is no movie in the database with id = -50.
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key
-- constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_1`
-- FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- referential integrity constraint 4
-- There should only be MovieActor entries for actors in the database
insert into MovieActor values
(12345, -25, 'Iron Man');
-- This violates the referencial constraint from MovieActor because
-- there is no actor in the database with id = -25.
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key
-- constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_2`
-- FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))

-- referential integrity constraint 5
-- There should only be MovieDirector entries for directors in the database
insert into MovieDirector values
(12345, -100);
-- This violates the referencial constraint from MovieDirector because
-- there is no director in the database with id = -100.
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key
-- constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_2`
-- FOREIGN KEY (`did`) REFERENCES `Director` (`id`))

-- referential integrity constraint 6
-- There should only be MovieDirector entries for directors in the database
insert into MovieDirector values
(-9000, 12345);
-- This violates the referencial constraint from MovieDirector because
-- there is no movie in the database with id = -9000.
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key
-- constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1`
-- FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- check constraint 1
-- Movies should have one of several standardized ratings
insert into Movie values
(12345, 'Final Fantasy VII', 2010, 'Awesome', 'Square');
-- This violates the check constraint from Movie because
-- 'Awesome' is not a standard rating.

-- check constraint 2
-- Actors should be either Male or Female
insert into Actor values
(13579, 'Sapien', 'Abe', 'Merman', '1965-04-04');
-- This violates the check constraint from Actor because
-- 'Merman' is neither Male nor Female.

-- check constraint 3
-- Review ratings should only be 0,1,2,3,4 or 5
insert into Review values
('Mike', '2008-10-18 12:00:00', '184723', -1, 'This movie REALLY sucked!');
-- This violates the check constraint from review because
-- it is not possible to give a movie a negative rating.

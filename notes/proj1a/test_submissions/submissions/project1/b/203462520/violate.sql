-- violate.sql
-- db modifications that should cause violations
-- I've added more than 12 constraints, so I will violate 3 of each kind:

-- PRIMARY KEYS

-- 1. violate Movie PRIMARY KEY by adding movie with duplicate id
-- oddly, there is no id=1...
INSERT INTO Movie VALUES(2, "No Rules", 1993, "NR", "Wilberforce Productions");
-- OUTPUT: ERROR 1062 (23000): Duplicate entry '2' for key 1

-- 2. violate Actor PRIMARY KEY by adding an actor with duplicate id
INSERT INTO Actor VALUES(1, "Peterson", "Peter", "Male", "1976-07-19", NULL);
-- OUTPUT: ERROR 1062 (23000): Duplicate entry '1' for key 1

-- 3. violate Director PRIMARY KEY by adding a director with duplicate id
INSERT INTO Director VALUES(16, "Peterson", "Peter", "1976-07-19", NULL);
-- OUTPUT: INSERT INTO Director VALUES(16, "Peterson", "Peter", "1976-07-19", NULL);

-- FOREIGN KEY CONSTRAINTS

-- 1. add a MovieActor for bogus actor -- requires real id
INSERT INTO MovieActor VALUES(100, 212313, "Boogerface");
-- OUTPUT: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))

-- 2. add a MovieActor for bogus movie -- requires real movie id
INSERT INTO MovieActor VALUES(100111, 180, "Boogerface");
-- OUTPUT: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_2` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- 3. add a review for a movie that doesn't exist -- requires real movie id
INSERT INTO Review VALUES("Peter Peterson", NOW(), 123141, 5, "It sucked.");
-- OUTPUT: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`CS143/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- 4. update a movie genre for a nonexistent movie -- requires real movie id
INSERT INTO MovieGenre VALUES(1231231, "RomanticHorror");
-- OUTPUT: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`CS143/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- 5. add a MovieDirector for bogus director -- requires real director
INSERT INTO MovieDirector VALUES(100, 212313);
-- OUTPUT: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`did`) REFERENCES `Director` (`id`))

-- 6. add a MovieDirector for bogus movie -- requires real movie id
INSERT INTO MovieDirector VALUES(100111, 16);
-- OUTPUT: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_2` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- CHECK CONSTRAINTS

-- 1. dod must be later than dob for Actor table
UPDATE Actor SET dod="1066-10-01" WHERE first='Kevin' AND last='Bacon';

-- 2. dod must be later than dob for Director table
UPDATE Director SET dod="1066-10-01" WHERE first='Clint' AND last='Eastwood';

-- 3. MaxPersonID must be > 0
UPDATE MaxPersonID SET id=-1;

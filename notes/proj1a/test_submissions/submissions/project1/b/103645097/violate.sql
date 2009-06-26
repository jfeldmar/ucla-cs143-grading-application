-- Violate primary key constraint of Movie (a movie already exists with id=2)
-- MySQL returns: ERROR 1062 (23000) at line 2: Duplicate entry '2' for key 1
INSERT INTO Movie VALUES
(2,
'Movie With ID=2, 2: The Revenge of Movie With ID=2',
2008,
'Broken',
'Primary Key Violation Production Company, Inc, Ltd'
);

-- Violate primary key constraint of Actor (56720 is the id of William Shatner)
-- MySQL returns: ERROR 1062 (23000) at line 12: Duplicate entry '56720' for key 1
INSERT INTO Actor VALUES
(56720,
'Singh',
'KHAAAAAAAAAN!',
'Male',
NULL,
NULL
);

-- Violate primary key constraint of Director (42284 is the id of Nicholas Meyer, director of Star Trek II: The Wrath of Khan)
-- MySQL returns: ERROR 1062 (23000): Duplicate entry '42284' for key 1
INSERT INTO Director VALUES
(42284,
'McKeyviolation',
'Primary',
NULL,
NULL
);

-- Violate foreign key constraint of MovieGenre (must reference a valid movie, and no movie has id 12345)
-- MySQL returns: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
INSERT INTO MovieGenre VALUES
(12345,
'Integrity Fail!'
);

-- Violate foreign key constraint of MovieDirector(must reference a valid movie, and no movie has id 12345)
-- MySQL returns: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
INSERT INTO MovieDirector VALUES
(12345,
42284
);

-- Violate foreign key constraint of MovieDirector(must reference a valid director, and no director has id  12345)
-- MySQL returns: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_2` FOREIGN KEY (`did`) REFERENCES `Director` (`id`))
INSERT INTO MovieDirector VALUES
(2,
12345
);

-- Violate foreign key constraint of MovieActor(must reference a valid movie, and no movie has id 12345)
-- MySQL returns: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
INSERT INTO MovieActor VALUES
(12345,
56720,
'Movie foreign key viola-KHAAAAN!'
);

-- Violate foreign key constraint of MovieActor(must reference a valid actor, and no actor has id 12345)
-- MySQL returns: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieActor`, CONSTRAINT `MovieActor_ibfk_2` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))
INSERT INTO MovieActor VALUES
(2,
12345,
'Non-existent actor!'
);

-- Violate foreign key constraint of Review(must reference a valid movie, and no movie has id 12345)
-- MySQL returns: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails (`TEST/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
INSERT INTO Review VALUES
(
'Frank McCrittick',
NOW(),
12345,
3,
'This movie does not exist. However, I give it a 3 out of 5 for causing a foreign key violation.'
);

-- Violate the CHECK constraint of Actor (must have died AFTER being born)
INSERT INTO Actor VALUES
(
12345,
'Traveler',
'Tim',
'Male',
DATE('1984-3-12'),
DATE('1923-8-7')
)

-- Violate the CHECK constraint of Director (must have died AFTER being born)
INSERT INTO DIRECTOR VALUES
(
12345,
'Traveler',
'Tim',
DATE('1984-3-12'),
DATE('1923-8-7')
)

-- Violate the CHECK constraint of Review (the rating must be between 0 and 5)
INSERT INTO Review VALUES
(
'Vegeta',
NOW(),
2,
9001,
'I rate this movie OVER NINE THOUSAAAAAAND!'
);
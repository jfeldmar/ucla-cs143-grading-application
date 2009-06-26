---------------------PRIMARY KEY CONSTRAINTS
--#1: every movie has a unique id number
--violates because there is already a movie in the Movie table with id=2, and id is the primary key for Movie, so you cannot insert another movie with id=2
--gives error: ERROR 1062 (23000): Duplicate entry '2' for key 1
INSERT INTO Movie VALUES (2, 'Hello CS143', 2008, 'R', 'UCLA Entertainment');

--#2: every actor has a unique id number
--you are trying to change the id of the actor who currently has id=11 to id=10, but there is already an actor with id=10. since id is the primary key for Actor, this update violates the primary key constraint
--gives error: ERROR 1062 (23000): Duplicate entry '10' for key 1
UPDATE Actor
SET id=10
WHERE id=11;

--#3: every director has a unique id number
--violates because there is already a director in the Director table with id=16, and id is the primary key for Director, so you cannot insert another director with id=16
--gives error: ERROR 1062 (23000): Duplicate entry '16' for key 1
INSERT INTO Actor VALUES (16, 'Alivia', 'Ronald', 'Male', '1987-12-08', NULL);


---------------------FOREIGN KEY CONSTRAINTS
--#1: every movie id in the moviegenre table must be in the movie table
--you cannot delete tuples that are referenced in another table. violates because now the MovieGenre table's mid attribute is referencing an id that does not appear in the Movie table's id column
--gives error: ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint fails ('CS143/MovieGenre', CONSTRAINT 'MovieGenre_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))
DELETE FROM Movie
WHERE id=2;

--#2: every director id in the moviedirector table must be in the director table
--you cannot insert something that is not in the referenced table into a referencing table. violates because there is no tuple in the Director table with id=70000. therefore, by inserting this tuple into the MovieDirector table, there is a director id in the MovieDirector table that is not in the referenced Director table
-- gives error: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/MovieDirector', CONSTRAINT 'MovieDirector_ibfk_1' FOREIGN KEY ('did') REFERENCES 'Director' ('id'))
INSERT INTO MovieDirector VALUES (253,70000);

--#3: every movie id in the moviedirector table must be in the movie table
--since the movie id 9999999 does not appear in the Movie table, you cannot update the MovieDirector table to have a tuple containing mid=999999. this is because every id in a referencing table must be in the referenced table.
--gives error: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/MovieDirector', CONSTRAINT 'MovieDirector_ibfk_2' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))
UPDATE MovieDirector
SET mid=9999999
WHERE mid=3;

--#4: every actor id in the movieactor table must be in the actor table
--you cannot delete tuples that are referenced in another table. violates because now the MovieActor table will try to reference the actor with id=162 from the Actor table, but it will not be there.
--gives error: ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint fails ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_1' FOREIGN KEY ('aid') REFERENCES 'Actor' ('id'))
DELETE FROM Actor
WHERE id=162;

--#5: every movie id in the movieactor table must be in the movie table
--you cannot insert something that is not in the referenced table into a referencing table.  since movie id 343434 does not appear in the Movie table, you cannot insert a tuple with movie id 343434 into the MovieActor table because MovieActor references Movie.
--gives error: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_2' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))
INSERT INTO MovieActor VALUES (343434,1,'Janitor');

--#6: every movie id in the review table must be in the movie table
--since the foreign key states that every movie id in the Review table must be in the Movie table, you cannot insert a tuple into review with the movie id 343434 because the id 343434 does not exist in the Movie table.
--gives error: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/Review', CONSTRAINT 'Review_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))
INSERT INTO Review VALUES ('cs143', '11:11:11', 343434, 1, 'sucks');

-----------------------CHECK CONSTRAINTS
--#1: every movie must be made by a company
--violates because the inserted tuple has company=NULL and every movie must be made by a company
INSERT INTO Movie VALUES (5000, 'Ocean Avenue', 2008, 'R', NULL);

--#2: every actor must have a sex of either Male or Female
--violates because you are trying to update the sex of the actor with id=1 from 'Female' to 'Paper'. since the check constraint says that sex must be 'Male' or 'Female', this update will fail
UPDATE Actor
SET sex='Paper'
WHERE id=1;

--#3: every director must have a date of birth
--violates because the inserted tuple has NULL as his date of birth, but every director must have a date of birth
INSERT INTO Actor VALUES (70001, 'Alivia', 'Ronald', 'Male', NULL, NULL);
--Primary Key violations

--There is already a movie w/ id 2
INSERT INTO Movie VALUES(2, 'Blah', 2003, 'G', 'Fox');
--ERROR 1062 (23000): Duplicate entry '2' for key 1

--There is an actor already w/ id 1
INSERT INTO Actor VALUES(1, 'Hwang', 'David', 'Male', 19870821, '\N');
--ERROR 1062 (23000): Duplicate entry '1' for key 1

--There is a director already w/ id 16
INSERT INTO Director VALUES(16, 'Hwang', 'David', 19870821, '\N');
--ERROR 1062 (23000): Duplicate entry '16' for key 1

----REFERENTIAL VIOLATIONS
--There is no Movie ID that is 10.
INSERT INTO MovieGenre VALUES(10, 'Action');
--ERROR 1452 (23000): Cannot add or update child row: a foregin key constraint fails ('TEST/MovieGenre',
--CONSTRAINT 'MovieGenre_ibfk_1' FOREIGN KEY ('mid' REFERENCES 'Movie' ('id'))

--There is no Movie ID that is 10.
INSERT INTO MovieDirector VALUES(10, 112);
--ERROR 1452 (23000): Cannot add or update child row: a foregin key constraint fails ('TEST/MovieDirector',
--CONSTRAINT 'MovieDirector_ibfk_1' FOREIGN KEY ('mid' REFERENCES 'Movie' ('id'))

--There is no Director Id of 1
INSERT INTO MovieDirector VALUES(9, 1);
--ERROR 1452 (23000): Cannot add or update child row: a foregin key constraint fails ('TEST/MovieDirector',
--CONSTRAINT 'MovieDirector_ibfk_1' FOREIGN KEY ('did' REFERENCES 'Director' ('id'))

--There is no Movie Id that is 10
INSERT INTO MovieActor VALUES(10,1,'Banker');
--ERROR 1452 (23000): Cannot add or update child row: a foregin key constraint fails ('TEST/MovieActor',
--CONSTRAINT 'MovieActor_ibfk_1' FOREIGN KEY ('mid' REFERENCES 'Movie' ('id'))

--There is no Actor ID that is 2
INSERT INTO MovieActor VALUES(9, 2, 'Actor');
--ERROR 1452 (23000): Cannot add or update child row: a foregin key constraint fails ('TEST/MovieActor',
--CONSTRAINT 'MovieActor_ibfk_1' FOREIGN KEY ('aid' REFERENCES 'Actor' ('id'))

--There is no Movie ID that is 10.
INSERT INTO Review VALUES('John', (SELECT CURRENT_TIMESTAMP), 10, 3, 'This movie iss alright');
--ERROR 1452 (23000): Cannot add or update child row: a foregin key constraint fails ('TEST/Review',
--CONSTRAINT 'Review_ibfk_1' FOREIGN KEY ('mid' REFERENCES 'Movie' ('id'))

--Check violations
--Movie ID of 99999999 is not allowed
INSERT INTO Movie VALUES(99999999, 'Blah', 2003, 'G', 'Fox');
--Actor ID of 99999999 is not allowed
INSERT INTO Actor VALUES(99999999, 'Hwang', 'David', 'Male', 19870821, '\N');
--Director ID of 99999999 is not allowed
INSERT INTO Director VALUES(99999999, 'Hwang', 'David', 19870821, '\N');
--Rating of 10 not allowed
INSERT INTO Review VALUES('Yikes', (SELECT CURRENT_TIMESTAMP), 10, 10, 'This movie iss alright');
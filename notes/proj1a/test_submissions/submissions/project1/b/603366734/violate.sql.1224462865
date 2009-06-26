-- 1)  this violates the primary key of Movie because it has the same id as an existing entry
INSERT INTO Movie VALUES(5, 'CS143 goes to Hollywood', 2008, '5/5', 'Paramount');
-- Error msg: ERROR 1062 (23000): Duplicate entry '5' for key 1

-- 2)  this violates the primary key of Actor because it has the same id as an existing entry
INSERT INTO Actor VALUES(1, 'Chang', 'Edward', 'male', 8/12/1987, NULL);
-- Error msg: ERROR 1062 (23000): Duplicate entry '1' for key 1

-- 3) this violates the primary key of Director because it has the same id as an existing entry
INSERT INTO Director VALUES(12414, 'Chang', 'Edward', 8/12/1987, NULL);
-- Error msg: ERROR 1062 (23000): Duplicate entry '12414' for key 1

-- 4) this violates the foreign key of Movie Genre because it tries to insert a value that is not represented in Movie(id)
INSERT INTO MovieGenre VALUES(12345, 'Too Funny');
-- Error msg: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/MovieGenre', CONSTRAINT 'MovieGenre_ibfk_1' FOREIGN KEY('mid') REFERENCES 'Movie' ('id'))

-- 5) this violates the did foreign key of Movie Director because it tries to insert a value that is not represented in Director(id)
INSERT INTO MovieDirector VALUES(1, 123456789);
-- Error msg: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/MovieDirector', CONSTRAINT 'MovieDirector_ibfk_1' FOREIGN KEY('did') REFERENCES 'Director' ('id'))

-- 6) this violates the mid foreign key of Movie Director because it tries to insert a value that is not represented in Movie(id)
INSERT INTO MovieDirector VALUES(4723, 68626);
-- Error msg: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/MovieDirector', CONSTRAINT 'MovieDirector_ibfk_1' FOREIGN KEY('mid') REFERENCES 'Movie' ('id'))

-- 7) this violates the mid foreign key of Movie Actor because it tries to insert a value that is not represented in Movie(id)
INSERT INTO MovieActor VALUES(123456789, 1, 'Hero');
-- Error msg: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_1' FOREIGN KEY('mid') REFERENCES 'Movie' ('id'))

-- 8) this violates the aid foreign key of Movie Actor because it tries to insert a value that is not represented in Actor(id)
INSERT INTO MovieActor VALUES(1, 123456789, 'Hero');
-- Error msg: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_1' FOREIGN KEY('aid') REFERENCES 'Actor' ('id'))

-- 8) this violates the mid foreign key of Review because it tries to insert a value that is not represented in Movie(id)
INSERT INTO Review VALUES('Tough', NULL, 123456789, '5/5', 'Awesome!!!!!');
-- Error msg: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/Review', CONSTRAINT 'Review_ibfk_1' FOREIGN KEY('mid') REFERENCES 'Movie' ('id'))

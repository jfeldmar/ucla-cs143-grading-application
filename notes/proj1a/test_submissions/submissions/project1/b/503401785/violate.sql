INSERT INTO Movie VALUES (2, "ABC", 1900, NULL, NULL);
-- Movie ID must be unique - violates the primary key.
-- Output:
-- ERROR 1062 (23000): Duplicate entry '2' for key 1

INSERT INTO Movie VALUES (NULL, NULL, 1900, NULL, NULL);
-- Movie ID and title cannot be NULL - violates the primary key and field 
-- constraint.
-- Output:
-- ERROR 1048 (23000): Column 'id' cannot be null

UPDATE Movie
SET id = -1 * id
WHERE id < 10;
-- Movie id cannot be negative numbers - violates the CHECK.  It is also a
-- referred key so it cannot be altered - violates the referential integrity.
-- Output:
-- ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint fails ('CS143/MovieGenre', CONSTRAINT 'MovieGenre_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id')) 

INSERT INTO Actor VALUES (1, "ABC", "DEF", "Female", NULL, NULL);
-- Actor ID must be unique - violates the primary key.
-- Output:
-- ERROR 1062 (23000): Duplicate entry '1' for key 1

INSERT INTO Actor VALUES (NULL, NULL, NULL, NULL, 1980-03-12, NULL);
-- Actor ID cannot be NULL because it is a primary key. Others also violate
-- the field constraint because they cannot be NULL - violates the primary key
-- and field constraints.
-- Output:
-- ERROR 1048 (23000): Column 'id' cannot be null

UPDATE Actor
SET id = -1, sex = 'neutral'
WHERE id = 1;
-- Actor ID cannot be negative and sex cannot be anything other than 'Male'
-- or 'Female' - violates the primary key and CHECK.

INSERT INTO Director VALUES (16, "ABC", "DEF", NULL, NULL);
-- Director ID must be unique - violates the primary key.
-- Output:
-- ERROR 1062 (23000): Duplicate entry '16' for key 1

INSERT INTO Director VALUES (NULL, NULL, NULL, 1980-03-12, NULL);
-- Director ID cannot be NULL because it is a primary key. Others also violate
-- the field constraint because they cannot be NULL - violates the primary key
-- and field constraints.
-- Output:
-- ERROR 1048 (23000): Column 'id' cannot be null

UPDATE Director
SET id = -1 * id
WHERE id < 100;
-- Director ID cannot be negative - violates the CHECK.

INSERT INTO MovieGenre VALUES (NULL, "Horror");
-- Movie ID must be unique and can't be NULL - violates the primary key as 
-- well as the referential integerity constraint because it references another
-- table.
-- Output:
-- ERROR 1048 (23000): Column 'mid' cannot be null

INSERT INTO MovieGenre VALUES (1, "Fantasy");
-- Cannot insert a movie ID into MovieGenre that doesn't already exist with
-- a matching id in Movie because mid references id - violates the referential
-- integrity constraint.
-- Output:
-- ERROR 1452 (23000): Cannot delete or update a child row: a foreign key constraint fails ('CS143/MovieGenre', CONSTRAINT 'MovieGenre_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id')) 

DELETE FROM Movie WHERE id = 2;
-- MovieGenre, MovieDirector, MovieActor, Review references this tuple's movie id, 
-- so it needs to be removed in the refering tables before the movie can be 
-- removed from Movie - violates the referential integrity constraint.
-- Output:
-- ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint fails ('CS143/MovieGenre', CONSTRAINT 'MovieGenre_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id')) 

INSERT INTO MovieDirector VALUES (NULL, 63);
-- Movie ID must be unique and can't be NULL - violates the primary key as 
-- well as the referential integerity constraint because it references another
-- table.
-- Output:
-- ERROR 1048 (23000): Column 'mid' cannot be null

INSERT INTO MovieDirector VALUES (1, 16);
-- Cannot insert a movie ID into MovieDirector that doesn't already exist with
-- a matching id in Movie because mid references id - violates the referential
-- integrity constraint.
-- Output:
-- ERROR 1452 (23000): Cannot delete or update a child row: a foreign key constraint fails ('CS143/MovieDirector', CONSTRAINT 'MovieDirector_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id')) 

INSERT INTO MovieActor VALUES (NULL, 10, "POI");
-- Movie ID must be unique and can't be NULL - violates the primary key as 
-- well as the referential integerity constraint because it references another
-- table.
-- Output:
-- ERROR 1048 (23000): Column 'mid' cannot be null

INSERT INTO MovieActor VALUES (1, 12, "BCD");
-- Cannot insert a movie ID into MovieActor that doesn't already exist with
-- a matching id in Movie because mid references id - violates the referential
-- integrity constraint.
-- Output:
-- ERROR 1452 (23000): Cannot delete or update a child row: a foreign key constraint fails ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id')) 

UPDATE Review
SET name = NULL, mid = NULL
WHERE mid < 100;
-- Name and movie id being reviewed cannot be NULL - violates the field
-- constraints.

UPDATE MaxPersonID
SET id = -1;
-- Cannot be a negative number, violates the CHECK.

UPDATE MaxMovieID
SET id = -1;
-- Cannot be a negative number, violates the CHECK.
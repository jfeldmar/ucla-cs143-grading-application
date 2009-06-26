--Violation 1: Insertion of duplicate ID in Movie table
--Returns: ERROR 1062 (23000): Duplicate entry '2' for key 1
insert into Movie value (2, 'test', 1988, 'R', 'test');

--violation 2: Insertion of duplicate ID in Actor table
--Returns: ERROR 1062 (23000): Duplicate entry '5605' for key 1
insert into Actor value (5605, 'Berry', 'Brooke', 'Female', \N, \N);

--Violation 3: Insertion of duplicate ID in Director table
--Returns: ERROR 1062 (23000): Duplicate entry '16' for key 1
insert into Director value (16, 'Berry', 'Brooke', 'Female', \N, \N);

--Violation 4: Insertion of a movie ID in Movie Genre table that does not exist in Movie Table
--Returns: Error 1452 (23000): Cannot add or update a child row: a foreign key constraint fails
insert into MovieGenre value (0, 'Horror');

--Violation 5: Insertion of a Movie ID in MovieDirector table that does not exist in Movie Table
--Returns: Error 1452 (2300): Cannot add or update a child row: a foreign key constraint fails
insert into MovieDirector value (0, 0);

--Violation 6: Insertion of a Director ID in MovieDirector table that does not exist in Director table
--Returns: Error 1452 (2300): Cannot add or update a child row: a foreign key constraint fails
insert into MovieDirector value (0, 0);

--violation 7: Insertion of a Movie ID in MovieActor table that does not exist in Movie table
--Returns: Error 1452 (2300): Cannot add or update a child row: a foreign key constraint fails
insert into MovieActor value (0, 0, 'Singer');

--Violation 8: Insertion of a Actor ID in MovieActor table that does not exist in Actor table
--Returns: Error 1452 (2300): Cannot add or update a child row: a foreign key constraint fails
insert into MovieActor value (0, 0, 'Singer');

--Violation 9: Insertion of a Movie ID in Review table taht does not exist in Movie table
--Returns: Error 1452 (2300): Cannot add or update a child row: a foreign key constraint fails
insert into Review value ('Reviewer', \N, 0, 0, 'Test Review');

--Violation 10: Creation of an ID < 1 in Movie table
insert into Movie value (0, 'test', 1988, 'R', 'test');

--Violation 11: Creation of an ID < 1 in Actor table
insert into Actor value (0, 'Berry', 'Brooke', 'Female', \N, \N);

--Violation 12: creation of an ID < 1 in Director table
insert into Director value (0, 'Berry', 'Brooke', 'Female', \N, \N);
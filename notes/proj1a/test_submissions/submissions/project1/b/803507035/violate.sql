-- violate.sql
-- Modifications that violate contraints in Database:

-- Contraint 1: Can't add Movie with id 4666, it already exists
INSERT INTO Movie VALUES (4666, 'X-MEN2', 2008, 'PG-13', '20th Century Fox');
-- Output from mySQL: ERROR 1062 (23000): Duplicate entry '4666' for key 1


-- Constraint 2: Should fail because year is less than 1978 (Check constraint, won't show up error 
INSERT INTO Movie VALUES (12, '14th Warrior', 98, 'PG-13', '20th Century Fox');
-- Output from mySQL: ERROR 1062 (23000): Duplicate entry '12' for key 1

-- Constraint 3: Can't add entry because it already exists
INSERT INTO Actor VALUES(5610, 'Berry', 'Halle', 'Female', '1968-10-10', NULL);
-- Output from mySQL: ERROR 1062 (23000): Duplicate entry '5610' for key 1


-- Constraint 4: Can't add entry because sex is something other than 'Female' or 'Male'
INSERT INTO Actor VALUES(4, 'Romero', 'Erick', 'Man', '1985-04-05', NULL);
-- Output from mySQL: ERROR 1062 (23000): Duplicate entry '4' for key 1

-- Constraint 5: Can't add because entry already exists
INSERT INTO Director VALUES (58777, 'Spielberg', 'Steven', '1946-12-18', NULL);
-- Output from mySQL: ERROR 1062 (23000): Duplicate entry '58777' for key 1

-- Constraint 6: Can't add because mid not in table Movie
INSERT INTO MovieGenre VALUES (7, 'Comedy');
-- Output from mySQL: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/......

-- Constraint 7, 8, and 9: Can't add because mid not in table Movie, and did not in Director table
INSERT INTO MovieDirector VALUES (7, 12);
-- Output from mySQL: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/......

-- Constraint 10, 11, 12, 13: Can't add because there is no Actor with id 18 or Movie with id 19
INSERT INTO MovieActor VALUES (19, 18, 'Police Man');
-- Output from mySQL: ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/......

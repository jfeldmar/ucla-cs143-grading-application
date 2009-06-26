------------------------Primary Key Constraints------------------------
INSERT INTO Actor VALUES(1,'Ho','Richard','Male',1988-10-30,NULL); 
--Every Actor must have a UNIQUE id
--ERROR 1062 (23000): Duplicate entry '1' for key 1

INSERT INTO Director VALUES(16,'Ho','Richard',1988-10-30,NULL); 
--Every Director must have a UNIQUE id
--ERROR 1062 (23000): Duplicate entry '16' for key 1

INSERT INTO Movie VALUES(2,'Richards Movie',2008,'PG','UCLA'); 
--EVERY Movie must have a UNIQUE id
--ERROR 1062 (23000): Duplicate entry '2' for key 1


------------------------FOREIGN KEY Constraints------------------------
DELETE FROM Movie where id=2;
--MovieGenre has a foreign key that points to this Movie
--ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint fails ('CS143/MovieGenre', CONSTRAINT 'MovieGenre_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))

DELETE FROM Movie where id=4710;
--MovieDirector has a foreign key that points to this Movie
--ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint fails ('CS143/MovieDirector', CONSTRAINT 'MovieDirector_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))

DELETE FROM Director where id=112
--MovieDirector has a foreign key that points to this Director
--ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint fails ('CS143/MovieDirector', CONSTRAINT 'MovieDirector_ibfk_2' FOREIGN KEY ('did') REFERENCES 'Director' ('id'))

INSERT INTO MovieActor VALUES(10, 2, 'Richard');
--MovieActor has to point to an existing Movie in the database
--ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))

DELETE FROM Actor where id=19;
--MovieActor has a foreign key that points to this Actor
--ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint fails ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_2' FOREIGN KEY ('aid') REFERENCES 'Actor' ('id'))

INSERT INTO Review VALUES('Richard',NULL,10,9,'MOVIE OF THE YEAR')
--Reviews must be made for an existing Movie in the database
--ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143/Review', CONSTRAINT 'Review_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))



------------------------CHECK Constraints------------------------------
INSERT INTO Movie VALUES(1,NULL,2008,'PG','UCLA'); 
--EVERY Movie must have a TITLE

INSERT INTO Actor VALUES(2,'Ho','Richard','Male',NULL,NULL);
--Every Actor must have a date of birth

INSERT INTO Director VALUES(1,'Ho','Richard',NULL,NULL);
--Every Director must have a date of birth
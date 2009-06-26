-------------------------------------------------------------------
--Primary Key constraints

--1) id in Actor is a primary key.
--Query)  INSERT INTO Actor VALUES(180, 'Heu', 'Alfred', 'Male', 1978-10-01, NULL);
--Error)  ERROR 1062 (23000) at line 1: Duplicate entry '180' for key 1
--Resaon) Existing primary key(180) cannot be inserted.

--2) id in Movie is a primary key.
--Query)  INSERT INTO Director VALUES(NULL, 'UCLA', 2008, 'PG-13', 'UC');
--Error)  ERROR 1048 (23000) at line 7: Column 'id' cannot be null
--Resaon) Primary key cannot be null.

--3) id in Director is a primary key.
--Query)  UPDATE Director SET id = 76 where id = 63;
--Error)  ERROR 1062 (23000) at line 12: Duplicate entry '76' for key 1
--Resaon) Primary key cannot be updated to an existing key value(76).

-------------------------------------------------------------------
--Foreigin Key constraints

--1) mid in MovieGenre is referenced from id in Movie(id).
--Query)  INSERT INTO MovieGenre VALUES (2030000, 'Comedy');
--Error)  ERROR 1452 (23000) at line 16: Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
--Resaon) Referenced key value(2030000) must exist in the referencing table(Movie).

--2) mid in MovieDirector is referenced from id in Movie(id).
--Query)  UPDATE MovieDirector SET mid = 10 WHERE mid = 3;
--Error)  ERROR 1452 (23000) at line 23: Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
--Resaon) Referenced key value(10) must exist in the referencing table(Movie).

--3) mid in MovieDirector is referenced from id in Director(id).
--Query)  INSERT INTO MovieDirector VALUES(17,3)
--Error)  ERROR 1452 (23000) at line 28: Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_2` FOREIGN KEY (`did`) REFERENCES `Director` (`id`))
--Resaon) Referenced key value(17) must exist in the referencing table(Director).

--4) mid in MovieActor is referenced from Movie(id).
--Query)  UPDATE Movie SET id = 10000000 WHERE id = 3;
--Error)  ERROR 1451 (23000) at line 34: Cannot delete or update a parent row: a foreign key constraint fails (`CS143/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
--Resaon) Movie id(3) in Movie cannot be updated because Movie id(3) exists in MovieDirector.

--5) aid in MovieActor is referenced from Actor(id).
--Query)  DELETE FROM Actor WHERE id = 19;
--Error)  ERROR 1451 (23000) at line 39: Cannot delete or update a parent row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_2` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))
--Resaon) Actor id(19) in Actor cannot be deleted because Actor id(19) exists in MovieDirector.

--6) mid in Review is referenced from Movie(id)
--Query)  INSERT INTO Review VALUES('SeunHoon', 2008-10-18, 100000, 5, 'Good Movie');
--Error)  ERROR 1452 (23000) at line 44: Cannot add or update a child row: a foreign key constraint fails (`CS143/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
--Resaon) Referenced key value(100000) must exist in the referencing table(Movie).

-------------------------------------------------------------------
--CHECK constraints

--1) DOD should be later than DOB in Actor/Director table as long as DOD is not null
--Query) INSERT INTO Director VALUES (600000, 'Park', 'Jay', 1970-10-01, 1950-01-01);
--Error) DOB(1970-10-01) is later than DOD(1950-01-01).

--2) The value of SEX in Actor should be either 'Female' or 'Male'
--Query) INSERT INTO Actor VALUES (600001, 'Park', 'Jay', 'XXX', 1970-10-01, NULL);
--Error) The value 'XXX' in attribute SEX is not valid.

--3) The range of  rating attribute in Review is from 0 to 5.
--Query) INSERT INTO Review VALUES('SeunHoon', 2008-10-18, 100000, -1, 'Good Movie');
--Error) The value -1 in attribute rating is out of the range (0 to 5).

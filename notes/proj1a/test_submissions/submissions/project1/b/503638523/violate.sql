-- Movie
INSERT INTO Movie VALUES(2, "Independence Day", 1997,"R","Federal Films");
-- causes ERROR 1062 (23000): Duplicate entry '2' for key 1
-- This is because that the 'id' value "2" already exists in the table.

INSERT INTO Movie VALUES(1, NULL, 2007,"R","Once Filems");
-- it should cause an error because title does not accept NULL.

UPDATE Movie SET year=3010 where id=2;
-- it should cause an error because year is not acceptable more than 2009.

INSERT INTO Movie VALUES(5000, "Google", 2004,"R","Netrwork Filems");
-- it should cause an error because id is not acceptable more than 4750.



-- Actor 
INSERT INTO Actor VALUES(10, "Abatantuono", "George", "Male",19420913, NULL);
-- causes ERROR 1062 (23000): Duplicate entry '10' for key 1
-- This is because that the 'id' value "10" already exists in the table.

INSERT INTO Movie VALUES(80000,  "Riza", "Evelyn", "Female",19890401, NULL);
-- it should cause an error because id is not acceptable more than 69000.



--Director
INSERT INTO Director VALUES(63, "Hanois", "Steve",19781107, NULL);
-- causes ERROR 1062 (23000): Duplicate entry '63' for key 1
-- This is because that the 'id' value "63" already exists in the table.

INSERT INTO Movie VALUES(80000,  "Riza", "Evelyn", "Female",19890401, NULL);
-- it should cause an error because id is not acceptable more than 69000.



--MovieGenre
UPDATE MovieGenre SET mid=7 where mid=4;
-- causes ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails
-- (`CS143/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`)
-- ON DELETE CASCADE ON UPDATE CASCADE)
-- This is because MovieGenre mid is a child of Movie id.



--MovieDirector
UPDATE MovieGenre SET mid=3 where mid=684;
-- causes ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails
-- (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`)
-- ON DELETE CASCADE ON UPDATE CASCADE)
-- This is because MovieDirector mid is a child of Movie id.

UPDATE MovieGenre SET did=5 where did=222;
-- causes ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails
-- (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`did`) REFERENCES `Director` (`id`)
-- ON DELETE CASCADE ON UPDATE CASCADE)
-- This is because MovieDirector did is a child of Director id.



--MovieActor
UPDATE MovieActor SET mid=3 where mid=5;
-- causes ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails
-- (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`aid`) REFERENCES `Movie` (`id`)
-- ON DELETE CASCADE ON UPDATE CASCADE)
-- This is because MovieActor mid is a child of Movie id.

UPDATE MovieActor SET aid=200 where aid=10208;
-- causes ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails
-- (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`)
-- ON DELETE CASCADE ON UPDATE CASCADE)
-- This is because MovieActor aid is a child of Actor id.



--Review
Insert INTO Review VALUES("TETSU",NULL,5,5,"GOOD!");
UPDATE MovieActor SET aid=106 where aid=5;
-- causes ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails
-- (`CS143/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`)
-- ON DELETE CASCADE ON UPDATE CASCADE)
-- This is because Review mid is a child of Movie id.



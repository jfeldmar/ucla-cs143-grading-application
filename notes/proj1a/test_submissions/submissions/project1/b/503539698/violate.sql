-- primary key constraint <1>: each movie should have a unique id
INSERT INTO Movie VALUES (539, "The Sixth Day", 1998, "2", "SONY"),
                         (539, "Terminator III", 2000, "4", "Walter");
-- Query OK, 2 rows affected (0.00 sec)
-- Records: 2  Duplicates: 0  Warnings: 0



-- primary key constraint<2>: each actor should have a unique id
INSERT INTO Actor VALUES (1000, "Leon", "Charlie", "Male", 1930-12-12, 2008-10-10),
                         (1000, "Quak", "Fred", "Male", 1929-11-12, 1998-11-19);
-- Query OK, 2 rows affected, 4 warnings (0.00 sec)
-- Records: 2  Duplicates: 0  Warnings: 4



-- primary key constraint<3>: each director should have a unique id
INSERT INTO Director VALUES (11, "Stewt", "Lee", 1930-12-12, 2008-10-10),
                            (11, "Liszt", "Franz", 1929-11-12, 1998-11-19);
-- Query OK, 2 rows affected, 4 warnings (0.00 sec)
-- Records: 2  Duplicates: 0  Warnings: 4



-- referential constraints<1>: movie id in movie genre can not be empty
INSERT INTO MovieGenre VALUES("", "Comedy");
-- Query OK, 1 row affected, 1 warning (0.04 sec)



-- referential constraints<2>: director id in movie director can not be empty
INSERT INTO MovieDirector VALUES("999", "");
-- Query OK, 1 row affected, 1 warning (0.00 sec)



-- referential constraints<3>: role in movie actor table can not be empty
INSERT INTO MovieActor VALUES("", "", "Stupid");
-- Query OK, 1 row affected, 2 warning (0.01 sec)


-- referential constraints<4>: movie id in review table can not be empty
INSERT INTO Review VALUES ("", 2008-10-10, "", "2", "Bad Movie");
-- Query OK, 1 row affected, 2 warnings (0.00 sec)



-- referential constraints<5>: reviewer name in review table can not be empty
INSERT INTO Review VALUES ("Charlie Charbie", 2008-10-10, "", "2", "Bad Movie");
-- Query OK, 1 row affected, 2 warnings (0.01 sec)



-- referential constraints<6>: date of birth in director table can not be empty
INSERT INTO Director VALUES (11, "Liszt", "Franz", "", 1998-11-19);
-- Query OK, 1 row affected, 2 warnings (0.00 sec)



-- check constraint<1>: date of birth of any people should not be later than current date
INSERT INTO Director VALUES (11, "Liszt", "Franz", "1930-06-13", 2100-11-19);
-- Query OK, 1 row affected, 1 warnings (0.00 sec)



-- check constraint<2>: MaxPersonID should not be negative number or zero
INSERT INTO MaxPersonID VALUES (-5);
-- Query OK, 1 row affected (0.00 sec)



-- check constraint<3>: MaxMovieId should not be negative number or zero
INSERT INTO MaxMovieID Values (-1000);
-- Query OK, 1 row affected (0.00 sec)

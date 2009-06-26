--<description: following blocks follow this format>
--<mySQL message>
--<SQL Statement>

--movie id is null, primary key cannot be null.
--Column 'id' cannot be null
INSERT INTO Movie VALUES( NULL, "Dawn of the Dead", 2995, "R", "Sony");

--movie name is null, doesn't make sense.
--Column 'title' cannot be null
INSERT INTO Movie VALUES( 4343, NULL, 2995, "PG-13", "RIAA");

--movie id exists already. primary key is unique.
--Duplicate entry '4333' for key 1
INSERT INTO Movie VALUES( 4333, "ueueo", 2995, "R", "ueou");

--rating is not one of the known values
--<should not work but mySQL doesn't support CHECK constraints>
UPDATE Movie SET rating="RR" WHERE id=4333;

--actor id is null, primary key cannot be null.
--Column 'id' cannot be null
INSERT INTO Actor VALUES( NULL, "ueou", "ueueu", "Female", 2007-05-21, 2008-05-21);

--director id is null, primary key cannot be null.
--Column 'id' cannot be null
INSERT INTO Director VALUES( NULL, "ueou", "ueueu", 2007-05-21, 2008-05-21);

--movie id 8765 Does not exist. Cannot reference.
--Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
INSERT INTO MovieActor VALUES(8765, 24, "main");

--actor id 4378 does not exist. cannot reference.
--Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieActor`, CONSTRAINT `MovieActor_ibfk_2` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))
INSERT INTO MovieActor VALUES(2495, 4378, "lead");

--movie year has to be less than 4 digits
--<mySQL doesn't support CHECK constraints>
INSERT INTO Movie VALUES(4324324, "foooooobar", 20008, "R", "mycomp");

--movie rating has to be a known value
--<mySQL doesn't support CHECK constraints>
INSERT INTO Movie VALUES(4324324, "foooooobar", 2008, "good", "mycomp");

--actor sex has to be either 'Male' or 'Female'
--<mySQL doesn't support CHECK constraints>
INSERT INTO Actor VALUES(4322123, "Darling", "Jen", "Alien", 2007-05-21, 2008-05-21);

--actor dod has to be after dob
--<mySQL doesn't support CHECK constraints>
INSERT INTO Actor VALUES(4322123, "Darling", "Jen", "Female", 2009-05-21, 2008-05-21);

--direct dod has to be after dob
--<mySQL doesn't support CHECK constraints>
INSERT INTO Director VALUES(4322123, "Dng", "Jn", "Male", 2030-05-21, 2008-05-21);

--entry is being referenced
--Cannot delete or update a parent row: a foreign key constraint fails (`TEST/MovieActor`, CONSTRAINT `MovieActor_ibfk_2` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))
DELETE FROM Actor WHERE id=68628;

--entry is being referenced
--Cannot delete or update a parent row: a foreign key constraint fails (`TEST/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
DELETE FROM Movie WHERE id=4734;

--entry is being referenced
--Cannot delete or update a parent row: a foreign key constraint fails (`TEST/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_2` FOREIGN KEY (`did`) REFERENCES `Director` (`id`))
DELETE FROM Director WHERE id=68623;

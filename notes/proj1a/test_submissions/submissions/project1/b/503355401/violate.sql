-- Every movie has a unique identification number.
-- Conflicts with id of ``Til There Was You''.
INSERT INTO Movie VALUES (2, "Alien", 1986, "PG-13", "Fox");
-- ERROR 1062 (23000) at line 3: Duplicate entry '2' for key 1

-- Every actor has a unique identification number.
-- Conflicts with id of ``Caroline Aaron''.
INSERT INTO Actor VALUES (19, "Weaver", "Sigourney", "Female", 19651012, \N);
-- ERROR 1062 (23000) at line 8: Duplicate entry '19' for key 1

-- Every director has a unique identification number.
-- Conflicts with id of ``Milie Aames''.
INSERT INTO Director VALUES (16, "Scott", "Ridley", 19530409, \N);
-- ERROR 1062 (23000) at line 13: Duplicate entry '16' for key 1

-- Every genre references a valid movie.
-- Does not reference a valid movie.
INSERT INTO MovieGenre VALUES (1, "Thriller");
-- ERROR 1452 (23000) at line 18: Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieGenre`, CONSTRAINT `movgen_mov` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- Every movie/director references a valid movie.
-- Does not reference a valid movie.
INSERT INTO MovieDirector VALUES (1, 16);
-- ERROR 1452 (23000) at line 23: Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieDirector`, CONSTRAINT `movdir_mov` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- Every movie/director references a valid director.
-- Does not reference a valid director.
INSERT INTO MovieDirector VALUES (2, 15);
-- ERROR 1452 (23000) at line 28: Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieDirector`, CONSTRAINT `movdir_dir` FOREIGN KEY (`did`) REFERENCES `Director` (`id`))

-- Every movie/actor references a valid movie.
-- Does not reference a valid movie.
INSERT INTO MovieActor VALUES (1, 1, "Lead actor");
-- ERROR 1452 (23000) at line 33: Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieActor`, CONSTRAINT `movact_mov` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- Every movie/actor references a valid actor.
-- Does not reference a valid actor.
INSERT INTO MovieActor VALUES (2, 2, "Lead actor");
-- ERROR 1452 (23000) at line 38: Cannot add or update a child row: a foreign key constraint fails (`TEST/MovieActor`, CONSTRAINT `movact_act` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))

-- Every review references a valid movie.
-- Does not reference a valid movie.
INSERT INTO Review VALUES ("Justin Meza", 20080508220353, 1, 5, "Great!");
-- ERROR 1452 (23000) at line 43: Cannot add or update a child row: a foreign key constraint fails (`TEST/Review`, CONSTRAINT `rev_mov` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))

-- Every actor's dod comes after their dob or is NULL.
-- Actor dod comes before dob.
INSERT INTO Actor VALUES (69001, "Weaver", "Sigourney", "Female", 19651012, 19641012);

-- Every director's dod comes after their dob or is NULL.
-- Director dod comes before dob.
INSERT INTO Director VALUES (69002, "Scott", "Ridley", 19530409, 19520409);

-- Every rating must be >= 0.
-- Rating is < 0.
INSERT INTO Review VALUES ("Justin Meza", 20080508220353, 2, -1, "Great!");

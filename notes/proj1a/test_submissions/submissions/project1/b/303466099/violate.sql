-- Movie's primary Key id cannot be NULL
INSERT INTO Movie VALUES (
	NULL, "High School Musical 3", 2008, "PG-13", "Disney"
	);

-- Actor's primary Key id cannot be NULL
INSERT INTO Actor VALUES (
	NULL, "Ferrell", "Will", "Male", 19670716, NULL
	);

-- Director's primary key id cannot be NULL
INSERT INTO Director VALUES (
	NULL, "Scorsese", "Martin", 19421117, NULL
	);

-- Violates foreign key constraint that references Movie
INSERT INTO MovieGenre VALUES (
	9999, "Horror"
	);

-- Violates foreign key constraint that references Movie
INSERT INTO MovieDirector VALUES (
	9999, 100
	);

-- Violates foreign key constraint that references Director
INSERT INTO MovieDirector VALUES (	
	100, 69001
	);

-- Violates foreign key constraint that references Movie
INSERT INTO MovieActor VALUES (
	9999, 100
	);

-- Violates foreign key constraint that references Actor
INSERT INTO MovieActor VALUES (
	100, 69001
	);

-- Violates foreign key constraint that references Movie
INSERT INTO Review VALUES (
	"Alex", '2008-10-19 09:57:30', 9999, 5, "Great Movie!"
	);

-- Violates check constraint on Review rating
INSERT INTO Review VALUES (
	"Alex", '2008-10-19 09:57:30', 100, 999, "Great Movie!"
	);

-- Violates check constraint on MaxPersonID
INSERT INTO MaxPersonID VALUES (
	-10
	);

-- Violates check constraint on MaxMovieID
INSERT INTO MaxMovieID VALUES (
	-10
	);
	


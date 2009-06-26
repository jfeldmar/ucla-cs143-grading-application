
-- Violates the Check statement ensuring every movie must have a title.  It attempts to add a movie without a title.
INSERT INTO Movie (id, year, rating, company) VALUES (4750, 2005, 'R', 'Universal');

--Violates the Check statement ensuring every movie must have a year.  It attempts to add a movie without a release year.
INSERT INTO Movie (id, title, rating, company) VALUES (4750, 'Crash', 'R', 'Universal');

-- Violates the Check statement ensuring every movie must have a company.  It attempts to add a movie without  a production company.
INSERT INTO Movie (id, title, year, rating) VALUES (4750, 'Crash', 2005, 'R');

-- Violates the Check statement ensuring every Actor must have  a date of birth.  It attempts to add an actor without a DOB.
INSERT INTO Actor (id, last, first, sex, dod) VALUES (69000, 'Bruce', 'Willis', 'M', NULL);

-- Violates the Check statement ensuring every Actor must have a gender.  It attempts to add an actor without a gender.
INSERT INTO Actor (id, last, first, dob, dod) VALUES (69000, 'Bruce', 'Willis', 12-11-1960, NULL);

-- Violates the Check statement ensuring that every Director must have a date of birth.  It attempts to add a director without a DOB.
INSERT INTO Director(id, last, first, dod) VALUES (69000, 'Stephen', 'Spielberg', NULL);

-- Violates every movie must have a unique ID number. It attempts to add a movie without an id Number.
INSERT INTO Movie (title, year, rating, company) VALUES ('crash', 2005, 'R', 'Universal');

-- Violates every actor must have a unque ID number.  It attempts to add an actor without an ID Number.
INSERT INTO Actor (last, first, sex, dob, dod) VALUES ('Willis', 'Bruce', 'M', 12-11-1960, NULL);

-- Violates constraint that every director must have a unique ID Number.  It attempts to add a ddirector without an ID Number.
INSERT INTO Director (last, first, dob, dod) VALUES ('Spielberg', 'Stephen', 12-11-1960, NULL);

-- Violates foreign key restraint on movie ID by attempting to change the movie id.
UPDATE Movie SET id = 2;
-- ERROR 1451 (23000): Cannot delete or update a parent row : a foreign key constraint  fails ('TEST/MovieGenre', Constrain 'MovieGenre_ibfk_1' FOREIGN KEY ('mid')
-- REFERENCES 'Movie' (id))

-- Violates Primary key restraint on mid by attempting to change the movie id so it refers to something else.
UPDATE Director SET mid = 3;
-- ERROR 1062 (23000): Duplicate entry  '3' for key 1.

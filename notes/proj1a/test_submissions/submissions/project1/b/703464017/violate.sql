--Movie
    /* id is a primary key, cannot be duplicated */
    INSERT INTO Movie VALUE(4734, 'abc', 1997, 1, 'dreamwork');
    /* ERROR 1062 (23000): Duplicate entry '4734' for key 1 */

    /* title cannot be NULL */
    INSERT INTO Movie VALUE(47344, NULL, 1997, 1, 'dreamwork');
    /* year cannot be negative */
    INSERT INTO Movie VALUE(47344, 'abc', -1, 1, 'dreamwork');
    /* rating must be in range [1,5] */
    INSERT INTO Movie VALUE(47344, 'abc', 1997, 0, 'dreamwork');

--Actor
    /* id is a primary key, cannot be duplicated */
    INSERT INTO Actor VALUE(68635, 'last', 'first', 'male', 1997-07-01, NULL);
    /* ERROR 1062 (23000): Duplicate entry '68635' for key 1 */

    /* last and first cannot be NULL */
    INSERT INTO Actor VALUE(686355, NULL, NULL, 'male', 1997-07-01, NULL);
    /* dob cannot be NULL */
    INSERT INTO Actor VALUE(686355, 'last', 'first', 'male', NULL, NULL);

--Director
    /* id is a primary key, cannot be duplicated */
    INSERT INTO Actor VALUE(68626, 'last', 'first', 1997-07-01, NULL);
    /* ERROR 1062 (23000): Duplicate entry '68626' for key 1 */

    /* last and first cannot be NULL */
    INSERT INTO Actor VALUE(686266, NULL, NULL, 1997-07-01, NULL);
    /* dob cannot be NULL */
    INSERT INTO Actor VALUE(686266, 'last', 'first', NULL, NULL);

--MovieGenre
    /* Foreign key constrain, new mid does not exist in Movie */
    UPDATE MovieGenre SET mid = 4701 where mid = 4700;
    /*
       ERROR 1452 (23000): Cannot add or update a child row: a foreign key
       constraint fails ('CS143/MovieGenre', CONSTRAINT 'MovieGenre_ibfk_1'
       FOREIGN KEY ('mid') REFERENCES 'Movie' ('id') ON DELETE CASCADE)
    */


--MovieDirector
    /* Foreign key constrain, new mid does not exist in Movie */
    UPDATE MovieDirector SET mid = 123 WHERE mid = 3743;
    /*
       ERROR 1452 (23000): Cannot add or update a child row: a foreign key
       constraint fails ('CS143/MovieDirector', CONSTRAINT 'MovieDirector_ibfk_1'
       FOREIGN KEY ('mid') REFERENCES 'Movie' ('id') ON DELETE CASCADE)
    */

    /* Foreign key constrain, new did does not exist in Director */
    UPDATE MovieDirector SET did = 123 WHERE did = 68622;
    /*
       ERROR 1452 (23000): Cannot add or update a child row: a foreign key
       constraint fails ('CS143/MovieDirector', CONSTRAINT 'MovieDirector_ibfk_2'
       FOREIGN KEY ('did') REFERENCES 'Director' ('id') ON DELETE CASCADE)
    */

--MovieActor
    /* role cannot be NULL */
    INSERT INTO MovieActor VALUE(998, 58668, NULL);
    /* Foreign key constrain, new mid does not exist in Movie */
    UPDATE MovieActor SET mid = 123 WHERE mid = 998;
    /*
       ERROR 1452 (23000): Cannot add or update a child row: a foreign key
       constraint fails ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_1'
       FOREIGN KEY ('mid') REFERENCES 'Movie' ('id') ON DELETE CASCADE)
    */

    /* Foreign key constrain, new aid does not exist in Actor */
    UPDATE MovieActor SET aid = 111 WHERE aid = 58668;
    /*
       ERROR 1452 (23000): Cannot add or update a child row: a foreign key
       constraint fails ('CS143/MovieActor', CONSTRAINT 'MovieActor_ibfk_2'
       FOREIGN KEY ('aid') REFERENCES 'Actor' ('id') ON DELETE CASCADE)
    */

--Review
    /* name cannot be NULL */
    INSERT INTO Review VALUE(NULL, 1997-07-01, 998, 1, 'the movie is sucks');
    /* rating cannot be NULL */
    INSERT INTO Review VALUE('Ian', 1997-07-01, 998, NULL, 'the movie is sucks');
    /* Foreign key constrain, new mid does not exist in Movie */
    UPDATE Review SET mid = 123 WHERE mid = 998;
    /*
       ERROR 1452 (23000): Cannot add or update a child row: a foreign key
       constraint fails ('CS143/Review', CONSTRAINT 'Reviewr_ibfk_1'
       FOREIGN KEY ('mid') REFERENCES 'Movie' ('id') ON DELETE CASCADE)
    */

    /* rating must be in range [1,5] */
    INSERT INTO Review VALUE('Ian', 1997-07-01, 998, 0, 'the movie is sucks');

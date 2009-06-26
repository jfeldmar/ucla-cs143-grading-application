--Attempts to insert a new movie with an already used id 
INSERT INTO TABLE movie 
Values (
1578, 'My New Car', 1988, 'R', 'Hollywood Studios')

--Attempts to add a movie with no title into the movie table
INSERT INTO TABLE movie 
Values (
1578, '', 1988, 'R', 'Hollywood Studios')

--Attempts to add a movie with a production year prior to 1900 
INSERT INTO TABLE movie 
Values (
1578, 'My New Car', 1827, 'R', 'Hollywood Studios')

--Attempts to add a new actor with an id that is already used
INSERT INTO TABLE Actor 
Values (
174, 'Smith', 'John', 'Male', '19880918', \N)

--Attempts to add a Director with an id that is already used
INSERT INTO TABLE Director
Values (23414, 'Smith', 'John', '19880918', \N)

--Attempts to add a new tuple into MovieGenre that with an invalid mid
INSERT INTO TABLE MovieGenre
Values (-1, 'Drama')

--Attempts to add a new tuple into MovieDirector that with an invalid mid
INSERT INTO TABLE MovieDirector
Values (-1, 47992)


--Attempts to add a new tuple into MovieDirector that with an invalid did
INSERT INTO TABLE MovieDirector
Values (
1578, 47992)


--Attempts to add a new tuple into MovieActor that with an invalid mid
INSERT INTO TABLE MovieActor
Values (
-1, 174, 'Lead')

--Attempts to add a new tuple into MovieActor that with an invalid aid
INSERT INTO TABLE MovieActor
Values (

1578, -1, 'Lead')

--Attempts to add a new tuple into Review that with an invalid mid
INSERT INTO TABLE Review
Values (

'John', , -1, 3, 'i liked it')

--Attempts to add a new tuple into Review that with an invalid rating
INSERT INTO TABLE Review
VALUES (

'John', NOW() , 

1578, 132, 'i liked it')






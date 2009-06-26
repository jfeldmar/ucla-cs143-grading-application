-- these queries are written based on the assumption that 
-- the movie, director and actor table do not have negative keys.
-- if so, please write keys that do not appear in the table

-- fails because movie has to have a unique id and title cannot be null
-- (for this i assume key value with id = 2 already exists
insert into Movie
	values (2,'i2',NULL,NULL,NULL);
insert into Movie
	values (2,NULL,NULL,NULL,NULL);


-- fails because MovieGenre gets an id which is not in Movie

insert into MovieGenre
	values (  -1 , NULL);
	
-- fails because MovieDirector gets an id which is not in Director
insert into MovieDirector
	values (  -1 , -123);

-- fails because MovieActor gets an id which is not in Actor
insert into MovieActor
	values (  -1 , -123, NULL);

-- fails because Review  gets an id which is not in Movie
insert into Review (name, mid, rating, comment)
	values (  'hah', -123, NULL, NULL);

-- fail beacuse value of id cannot be null
insert into MovieGenre
	values (  NULL , NULL);
insert into MovieDirector
	values ( -1 , NULL);
insert into MovieActor
	values ( -1 , NULL, NULL);

-- CHECK CONSTRAINTS--
-- PLEASE NOTE: the sql code is commented because check condition does
-- not check the values.

-- check if the person is both actor and director, the he has the same id
insert into Director values(-1, 'j', 'k', 19520022, NULL);
insert into Actor values(-2, 'j', 'k','male', 19520022, NULL);

-- check if values only from following set. if not fail.(we give it something
-- that fails) (G, PG, PG-13, R, NC-17

insert into Movie values(-1, 'a', 1988, 'X', null);

-- check to see if sex between male and female only

insert into Actor values(-1, 't', 'y', 'adad', 19520022, NULL);


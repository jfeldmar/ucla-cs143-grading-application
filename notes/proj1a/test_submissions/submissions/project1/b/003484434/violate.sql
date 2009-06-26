-- PRIMARY KEY VIOLATIONS
-- same primary key, different people
insert into Director values('2','lee','lee','19521112', NULL);
insert into Director values('2','choi','kang','19529212', NULL);

-- same Movie id, different Movie titles
insert into Movie values('567', 'Hello world!', '1992', 'R', 'Fox');
insert into Movie values('567', 'When Databases Attack', '1992', 'PG-13', 'Paramount');

-- same Actor id, different attributes
insert into Actor values('9', 'thai', 'tarun', 'F', '19431543', NULL);
insert into Actor values('9', 'sendie', 'cho', 'M', '19437743', NULL);


-- REFERENTIAL/FOREIGN KEY VIOLATIONS
-- inserts a value into MovieDirector, but 766 is not a 
-- valid Director id
insert into MovieDirector values('4734', '766');

-- tries to delete Movie 'jack Frost' but this causes 
-- a foreign key constraint violation
delete from Movie where title='Jack Frost'; --2094
delete from Movie where year='1998' and company='Castle Rock Entertainment';
delete from Actor where last='DiCaprio';
-- message: ERROR 1451 (23000): Cannot delete or update a parent row: 
--          a foreign key constraint fails

insert into MovieDirector values('789', '68606');
insert into MovieGenre values('123', 'Horror');
insert into MovieActor values('10', '50', 'Lead');
-- message: ERROR 1452 (23000): Cannot add or update a child row: 
--          a foreign key constraint fails


-- CHECK CONDITION VIOLATIONS 
-- if the Actor and Director have the
-- same name, they should have the same ID
insert into Director values('2','lee','lee','19521112', NULL);
insert into Actor values('3','lee','lee','F','19521112', NULL);

-- this Movie tuple has the rating as X, which isn't an 
-- actual MPAA movie rating
insert into Movie values('567', 'Hello world!', '1992', 'X', 'Fox');

-- this tuple has gender not as M or F
insert into Actor values('9', 'thai', 'tarun', 'X', '19431543', NULL);



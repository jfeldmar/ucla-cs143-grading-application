-- Primary Keys Violations
insert into Movie value(4734, 'test movie', 2008, 'R', 'test corp');
	-- violate unique constraint of primary key
	-- ERROR 1062 (23000): Duplicate Entry '4734' for key 1

insert into Actor value(68635, 'l', 'f', 'Male', '2008-01-01', null);
	-- violate unique constraint of primary key
	-- ERROR 1062 (23000): Duplicate Entry '68635' for key 1

insert into Director value(68626, 'l', 'f', '2008-01-01', null);
	-- violate unique constraint of primary key
	-- ERROR 1062 (23000): Duplicate Entry '68626' for key 1

-- Foreign Keys Violations
insert into MovieGenre value(5000, 'test');
	-- violate foreing key constraint
	-- ERROR 1452 (23000): Cannot add or updatea child row

insert into MovieActor value(5000, 49514, 'test');
	-- violate foreing key constraint
	-- ERROR 1452 (23000): Cannot add or updatea child row

insert into MovieActor value(4729, 70000, 'test');
	-- violate foreing key constraint
	-- ERROR 1452 (23000): Cannot add or updatea child row

insert into MovieDirector value(5000, 58777 );
	-- violate foreing key constraint
	-- ERROR 1452 (23000): Cannot add or updatea child row

insert into MovieDirector value(4729, 70000 );
	-- violate foreing key constraint
	-- ERROR 1452 (23000): Cannot add or updatea child row

insert into Review value( 'test', '2008-01-01 00:00:00.00', 5000, 10, 'test')
	-- violate foreing key constraint
	-- ERROR 1452 (23000): Cannot add or updatea child row

-- Check Violations
insert into Movie value(5000, null, 2008, 'R', 'test corp');
	-- violate null constraint
	-- ERROR 1048 (23000): Column 'title' cannot be null

insert into Actor value(68700, 'l', 'f', 'Male', null, null);
	-- violate null constraint
	-- ERROR 1048 (23000): Column 'title' cannot be null

insert into Director value(68701, 'l', 'f', null, null);
	-- violate null constraint
	-- ERROR 1048 (23000): Column 'title' cannot be null

-- Primary key constraints --
-- for Actor, Director, and Movie tables
-- Each table have 'id' as a primary key meaning no two tuples may 
-- share the same 'id'.

INSERT INTO Movie('10','Movie',1980,'M','MGM');
INSERT INTO Movie('10','DoD','2000,'PG13','FOX');


-- Referential integrity constraints -- 
-- If an id with foreign key constraint is inserted without having
-- a corresponding id in the referenced table,  
-- then a referential integrity constraint is violated. 

INSERT INTO MovieActor values('10000','51000','lead');

-- Check constraints --
-- If a person is both an Actor and Director, both should have the same ID

INSERT INTO Director values('10','John','Smith','19520022', NULL);
INSERT INTO Actor    values('12','John','Smith','M','19520022', NULL);

-- 'rating' entry for movies is limited to five values
-- 'G', 'PG', 'PG-13', 'R', 'NC-17'

INSERT INTO Movie values('212','Home',1988,'X','WB'); 

-- 'sex' for Actor table must be either 'F' or 'M'

INSERT INTO Actor values('09','Peter','Solace','T','19425522',NULL);
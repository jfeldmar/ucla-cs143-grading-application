INSERT INTO Movie VALUES(12,"1776",1972,"PG","Columbia Pictures Corporation"); -- This violates the primary key constraint in which the id must be unique
UPDATE Movie SET year=-5 WHERE id=12; -- This violates the check constraint in which the year cannot be negative
INSERT INTO Actor VALUES(1,"A","Isabelle","Female",19750525,\N); -- This violates the primary key constraint in which the id must be unique
UPDATE Actor SET sex="123456" WHERE id=1; -- This violates the check constraint in which the sex must be either Male or Female
INSERT INTO Director VALUES(16,"Aames","Willie",19600715,\N); -- This constraint violates the primary key constraint in which the id must be unique
UPDATE Director SET id=-3 WHERE id=16; -- This violates the check constraint in which the primary key must be unique
INSERT INTO MovieGenre VALUES(0,"Test"); -- This violates the foreign key constraint in which a referenced movie id does not exist in the referenced table Movie
INSERT INTO MovieDirector VALUES(0,16); -- This violates the foreign key constraint in which a referenced movie id does not exist in the referenced table Movie
UPDATE MovieDirector SET did=15 WHERE did=16; -- This violates the foreign key constraint in which a referenced director id does not exist in the referenced table Director
INSERT INTO MovieActor VALUE(1,1,"Test"); -- This violates the foreign key constraint in which a referenced movie id does not exist in the referenced table Movie
UPDATE MovieActor SET aid=-3 WHERE role="Christmas Radio Preacher"; -- This violates the foreign key constraint in which a referenced actor id does not exist in the referenced table Actor
INSERT INTO Review VALUES("Test",CURRENT_TIMESTAMP,1,1,"Test"); -- This violates the foreign key constraint in which a referenced movie id does not exist in the referenced table Movie
INSERT INTO Review VALUES("Test",CURRENT_TIMESTAMP,2,1,""); -- This violates the check constraint in which the comment must not be null
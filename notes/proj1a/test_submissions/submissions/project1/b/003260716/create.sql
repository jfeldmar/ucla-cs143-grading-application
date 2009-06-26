CREATE TABLE Movie(id INT,title VARCHAR(100),year INT, rating VARCHAR(10),company VARCHAR(50),CHECK(id >= 1)) ENGINE = INNODB;
ALTER TABLE Movie ADD PRIMARY KEY (id);
/*all movies need to have an id. years, ratings, and company are not as important as an id and old movies maynot even have this information.  
Also, the movie does not necessarily need a title. An example of which would be an unfinished movie that was bever released and never had one.
Movies can also not have negative id values*/ 

CREATE TABLE Actor(id INT,last VARCHAR(20),first VARCHAR(20),sex VARCHAR(6),dob DATE,dod DATE, CHECK(id >= 1)) ENGINE = INNODB;
ALTER TABLE Actor ADD PRIMARY KEY (id);
/*all actors need to have an id. names and dob are not as important as an id as this information might not even be available for obscure actors
and extras. Actors can also not have negative id values*/

CREATE TABLE Director(id INT,last VARCHAR(20),first VARCHAR(20),dob DATE,dod DATE,CHECK(id >= 1)) ENGINE = INNODB;
ALTER TABLE Director ADD PRIMARY KEY (id);
/*all directors need to have an id. names and dob are not as important as an id as this information might not even be available for obscure/unknown
directors. Also, the movie does not necessarily need a title. Actors can also not have negative id values*/

CREATE TABLE MovieGenre(mid INT, genre VARCHAR(20)) ENGINE = INNODB;
ALTER TABLE MovieGenre ADD FOREIGN KEY (mid) REFERENCES Movie(id);
/*mid's need to link to actual movies so a foreign key restraint is required here.*/ 

CREATE TABLE MovieDirector(mid INT, did INT) ENGINE = INNODB;
ALTER TABLE MovieDirector ADD FOREIGN KEY (mid) REFERENCES Movie(id);
ALTER TABLE MovieDirector ADD FOREIGN KEY (did) REFERENCES Director(id);
/*mid's and did's need to link to actual movies and directors so 2 foreign key restraints are required here.*/ 

CREATE TABLE MovieActor(mid INT, aid INT, role VARCHAR(50)) ENGINE = INNODB;
ALTER TABLE MovieActor ADD FOREIGN KEY (mid) REFERENCES Movie(id);
ALTER TABLE MovieActor ADD FOREIGN KEY (aid) REFERENCES Actor(id);
/*mid's and aid's need to link to actual movies and Actors so 2 foreign key restraints are required here.*/ 

CREATE TABLE Review(name VARCHAR(50),time TIMESTAMP,mid INT,rating INT,comment VARCHAR(500)) ENGINE = INNODB;
ALTER TABLE Review ADD FOREIGN KEY (mid) REFERENCES Movie(id);
/*Reviews need to be associated with an actual movie so a oreign key restraint is required here.*/

CREATE TABLE MaxPersonID(id INT) ENGINE = INNODB;
CREATE TABLE MaxMovieID(id INT) ENGINE = INNODB;

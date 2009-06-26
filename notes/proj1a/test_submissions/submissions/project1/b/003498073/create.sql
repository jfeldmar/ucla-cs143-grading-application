/*Create.sql
Creates the following tables
	Movie
	Actor
	Director
	MovieGenre
	MovieDirector
	MovieActor
	Review
	MaxPersonID
	MaxMovieID

The format of the tables are as as follows
	Name	Type			Description
-------------------------------------------
*/

/* Movie
	-Every Movie must a unique id, a title, the year it was made and a production company
*/
CREATE TABLE Movie(
	id		INT NOT NULL,				-- Movie ID	
	title	VARCHAR(100) NOT NULL,		-- Movie Title
	year	INT NOT NULL,				-- Release year
	rating	VARCHAR(10),				-- MPAA rating
	company	VARCHAR(50)	NOT NULL,		-- Production company
	
	PRIMARY KEY(id)
	
	)ENGINE = INNODB;

/* Actor
	-Every actor must have unique id, a last name, a first name, gender, and date of birth
*/
CREATE TABLE Actor(
	id 		INT NOT NULL,				-- Actor ID
	last 	VARCHAR(20) NOT NULL,		-- Last Name
	first 	VARCHAR(20) NOT NULL,		-- First Name
	sex		VARCHAR(6) NOT NULL,		-- Sex of the actor
	dob		DATE NOT NULL,				-- Date of Birth
	dod		DATE,						-- Date of Death
	
	PRIMARY KEY(id)
	)ENGINE = INNODB;
	
/* Director
	-Every director must have an unique id, a last name, first name, and date of birth
*/
CREATE TABLE Director(
	id		INT NOT NULL,				-- Director ID
	last 	VARCHAR(20) NOT NULL,		-- Last Name
	first 	VARCHAR(20) NOT NULL,		-- First Name
	dob		DATE NOT NULL,				-- Date of Birth
	dod		DATE,						-- Date of Death
	
	PRIMARY KEY(id)
	)ENGINE = INNODB;
	
/* MovieGenre
	-There must be a mid that actually exist in the Movie table and a genre must be specified
*/
CREATE TABLE MovieGenre(
	mid		INT NOT NULL,				-- Movie ID
	genre	VARCHAR(20) NOT NULL,		-- Movie Genre
	
	FOREIGN KEY(mid) REFERENCES Movie(id)
	ON DELETE CASCADE
	----  When a Movie is deleted we no longer care about the genres associated so each Movie Genre is deleted
	
	)ENGINE = INNODB;

/* MovieActor
	-There must be a mid and aid that exist in the movie and actor table, respectively
*/
CREATE TABLE MovieActor(
	mid		INT NOT NULL,				-- Movie ID
	aid		INT NOT NULL,				-- Actor ID
	role 	VARCHAR(50),				-- Actor role in Movie
	
	FOREIGN KEY(mid) REFERENCES Movie(id)
	ON DELETE CASCADE,
	----  When a Movie is deleted we no longer care about the actors associated with the movie	
	FOREIGN KEY(aid) REFERENCES Actor(id)
	----  We don't want to permit deletion of actors while there is a reference to them from Movies
	)ENGINE= INNODB;
	
/* MovieDirector
	-There must be a mid and did that exist in the movie and director table, respecitively
*/
CREATE TABLE MovieDirector(
	mid		INT NOT NULL,				-- Movie ID
	did		INT	NOT NULL,				-- Director ID
	
	FOREIGN KEY(mid) REFERENCES Movie(id)
	ON DELETE CASCADE,
	----  When a Movie is deleted we no longer care about the Director who filmed it
	FOREIGN KEY(did) REFERENCES Director(id)
	----  We don't want to permit deletion of a director while there are references to him/her from movies
	)ENGINE = INNODB;
	
/* Review
	-There must a be a mid that exist in the movie table
*/
CREATE TABLE Review(
	name	VARCHAR(20),				-- Reviewer Name
	time	TIMESTAMP,					-- Review Time
	mid		INT NOT NULL,				-- Movie ID
	rating	INT,						-- Review Rating
	comment	VARCHAR(500),				-- Reviewer Comment
	
	FOREIGN KEY(mid) REFERENCES Movie(id)
	ON DELETE CASCADE
	----  When a movie is deleted we don't care about the reviews anymore so they are deleted too
	)ENGINE = INNODB;

/* MaxPersonID
	-We can't have a null value
*/
CREATE TABLE MaxPersonID(
	id		INT	NOT NULL				-- Max ID assigned to all persons
	);
	
/* MaxMovieID
	-We can't have a null value
*/
CREATE TABLE MaxMovieID(
	id		INT	NOT NULL				-- Max ID assigned to all Movies
	);
	
	
----  Triggers that I will write are that Review ratings are between 1-5 and
--  MPAA ratings are like real world MPAA Ratings
	
----  A Trigger I thought of was that all ID types must positive, this could easily be accomplished 
--  Make id types unsigned as well, but I didn't wnat to change the Schema

DELIMITER //             -- change the end of statement delimiter from ; to //
	CREATE TRIGGER insert_rating_check
	BEFORE INSERT ON Review
	FOR EACH ROW             -- note that "REFERECING NEW ROW AS NEW" is missing
	BEGIN
	   IF NEW.rating > 5 OR NEW.rating < 1 THEN 
	      DELETE FROM Review WHERE rating = new.rating;
	   END IF;
	END; 
	//                   -- mark the end of trigger with //
DELIMITER ;              -- change the delimiter back to ;

DELIMITER //             -- change the end of statement delimiter from ; to //
	CREATE TRIGGER update_rating_check
	BEFORE UPDATE ON Review
	FOR EACH ROW             -- note that "REFERECING NEW ROW AS NEW" is missing
	BEGIN
	   IF NEW.rating > 5 OR NEW.rating < 1 THEN 
	      UPDATE Review SET 
	      	name = OLD.name, 
	      	time = OLD.time,
	      	mid = OLD.mid,
	      	rating = OLD.rating,
	      	comment = OLD.comment;
	   END IF;
	END; 
	//                   -- mark the end of trigger with //
DELIMITER ;              -- change the delimiter back to ;

/*   I COULDN't GET THIS TO WORK
DELIMITER //             -- change the end of statement delimiter from ; to //
	CREATE TRIGGER insert_mpaa_check
	BEFORE INSERT ON Movie
	FOR EACH ROW             -- note that "REFERECING NEW ROW AS NEW" is missing
	BEGIN
	   IF NEW.rating = "G" 
			OR NEW.rating = "NR" 
			OR NEW.rating = "PG"
			OR NEW.rating = "PG-13"
			OR NEW.rating = "R"
			OR NEW.rating = "NC-17")
		THEN 
			INSERT INTO Movie VALUES(new.id, new.title, new.year, new.rating, new.company);
		ELSE
			DELETE FROM Movie WHERE rating = new.rating;
	   END IF;
	END; 
	//                   -- mark the end of trigger with //
DELIMITER ;              -- change the delimiter back to ;
*/

/*		
DELIMITER //             -- change the end of statement delimiter from ; to //
	CREATE TRIGGER update_mpaa_check
	BEFORE UPDATE ON Movie
	FOR EACH ROW             -- note that "REFERECING NEW ROW AS NEW" is missing
	BEGIN
		IF NEW.rating = "G" 
			OR NEW.rating = "NR" 
			OR NEW.rating = "PG"
			OR NEW.rating = "PG-13"
			OR NEW.rating = "R"
			OR NEW.rating = "NC-17"
		then
	      UPDATE Review SET 
	      	id = NEW.id, 
	      	title = NEW.title,
	      	year = NEW.year,
	      	rating = NEW.rating,
	      	company = NEW.company;
		ELSE
	      UPDATE Review SET 
	      	id = OLD.id, 
	      	title = OLD.title,
	      	year = OLD.year,
	      	rating = OLD.rating,
	      	company = OLD.company;
	   END IF;
	END; 
	//                   -- mark the end of trigger with //
DELIMITER ;              -- change the delimiter back to ;
*/
----  Triggers that can't be implemenented in MySQL because of its limitations are
--  	-We don't add too many tuples into a certain table
--  	-The Varchar Buffers aren't overflowed


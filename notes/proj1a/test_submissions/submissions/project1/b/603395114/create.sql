-- Constrainst:
-- Primary Key
-- 1. The movie id should be unique and therefore a primary key.
-- 2. The reviewer name, tile and movie id should uniquely identify  a review.
-- 3. The id in the actor table should be a primary key.
-- 4. The id in the director table should be a primary key.
-- 
-- Referential Integrity
-- 1.  In the MovieGenre table mid should reference Movie(id) as a foreign key.
-- 2. In the MovieDirector table mid should reference Movie(id) as a foreign key.
-- 3. In the MovieActor table mid should reference Movie(id) as a foreign key.
-- 4. In the Review table mid should reference Movie(id) as a foreign key.
-- 5. In the MovieDirector table did should reference Director(id) as a foreign key.
-- 6. In the MovieActor table aid should reference Actor(id) as a foreign key.
-- 
-- Check Constraints
-- 1.  Check that dob is logically less than dod in the Director table.
-- 2. Check that dob is logically less than dod in the Actor table.
-- 3. Check that the year in the Movie table is positive.
 
 
 
 CREATE TABLE `CS143`.`Movie` (
	`id` INT NOT NULL COMMENT 'Movie ID',
	`title` VARCHAR( 100 ) NOT NULL COMMENT 'Movie title',
	`year` INT NOT NULL COMMENT 'Movie year',
	`rating` VARCHAR( 10 ) NOT NULL COMMENT 'MPAA rating',
	`company` VARCHAR( 50 ) NOT NULL COMMENT 'Production Company',
-- The movie id should be unique and therefore a primary key.
	PRIMARY KEY ( `id` ),
	CHECK (year > 0)
	) ENGINE = InnoDB;

CREATE TABLE `CS143`.`MovieGenre` (
	`mid` INT NOT NULL COMMENT 'Movie ID',
	`genre` VARCHAR( 20 ) NOT NULL COMMENT 'Movie genre',
-- In the MovieGenre table mid should reference Movie(id) as a foreign key
	FOREIGN KEY mid REFERENCES Movie(id)
	) ENGINE = InnoDB;
	
CREATE TABLE `CS143`.`Actor` (
	`id` INT NOT NULL COMMENT 'Actor ID',
	`last` VARCHAR( 20 ) NOT NULL COMMENT 'Last name', 
	`first` VARCHAR( 20 ) NOT NULL COMMENT 'First name',
	`sex` VARCHAR( 6 ) NOT NULL COMMENT 'Sex of the actor',
	`dob` DATE NOT NULL COMMENT 'Date of birth',
	`dod` DATE COMMENT 'Date of death',
-- The id in the actor table should be a primary key.
	PRIMARY KEY(`id`),
	CHECK (dob <= dod)
	) ENGINE = InnoDB;
	
CREATE TABLE `CS143`.`Director` (
	`id` INT NOT NULL COMMENT 'Director ID',
	`last` VARCHAR( 20 ) NOT NULL COMMENT 'Last name', 
	`first` VARCHAR( 20 ) NOT NULL COMMENT 'First name',
	`dob` DATE NOT NULL COMMENT 'Date of birth',
	`dod` DATE COMMENT 'Date of death',
-- -- The id in the director table should be a primary key.
	PRIMARY KEY(`id`),
	CHECK (dob <= dod)
	) ENGINE = InnoDB;
	
CREATE TABLE `CS143`.`MovieDirector` (
	`mid` INT NOT NULL COMMENT 'Movie ID',
	`did` INT NOT NULL COMMENT 'Director ID',
-- In the MovieDirector table mid should reference Movie(id) as a foreign key
	FOREIGN KEY mid REFERENCES Movie(id),
-- In the MovieDirector table did should reference Director(id) as a foreign key
	FOREIGN KEY did REFERENCES Director(id),
	PRIMARY KEY(`mid`, `did`)
	) ENGINE = InnoDB;
	
CREATE TABLE `CS143`.`MovieActor` (
	`mid` INT NOT NULL COMMENT 'Movie ID',
	`aid` INT NOT NULL COMMENT 'Actor ID',
	`role` VARCHAR(50) NOT NULL COMMENT 'Actor role in movie',
-- In the MovieActor table mid should reference Movie(id) as a foreign key
	FOREIGN KEY mid REFERENCES Movie(id),
-- In the MovieActor table aid should reference Actor(id) as a foreign key
	FOREIGN KEY aid REFERENCES Actor(id),
	PRIMARY KEY(`mid`, `aid`)
	) ENGINE = InnoDB;
	
CREATE TABLE `CS143`.`Review` (
	`name` VARCHAR( 20 ) NOT NULL COMMENT 'Reveiwer name',
	`time` TIMESTAMP NOT NULL COMMENT 'Review time',
	`mid` INT NOT NULL COMMENT 'Movie ID',
	`rating` INT NOT NULL COMMENT 'Review rating',
	`comment` VARCHAR( 500 ) NOT NULL COMMENT 'Reviewer comment',
-- In the Review table mid should reference Movie(id) as a foreign key
	FOREIGN KEY mid REFERENCES Movie(id),
-- The reviewer name, tile and movie id should uniquely identify  a review.
	PRIMARY KEY(`name`,`time`,`mid`)
	) ENGINE = InnoDB;
	
CREATE TABLE `CS143`.`MaxPersonID` (
	`id` INT NOT NULL COMMENT 'MAX ID assigned to all persons'
	) ENGINE = InnoDB;

CREATE TABLE `CS143`.`MaxMovieID` (
	`id` INT NOT NULL COMMENT 'MAX ID assigned to all movies'
	) ENGINE = InnoDB;
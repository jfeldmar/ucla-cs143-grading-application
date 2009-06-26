CREATE TABLE Movie(
		id int NOT NULL, 
		title varchar(100) NOT NULL, 
		year int, 
		rating varchar(10), 
		company varchar(50),
		PRIMARY KEY(id))
		ENGINE = InnoDB;
		
CREATE TABLE Actor(
		id int NOT NULL,
		last varchar(20) NOT NULL,
		first varchar(20),
		sex varchar(6) DEFAULT NULL CHECK (sex = 'Male' OR sex = 'Female' OR sex = NULL),
		dob date DEFAULT NULL,
		dod date DEFAULT NULL CHECK (dod > dob),
		PRIMARY KEY(id),
		CHECK (NOT EXISTS (SELECT *
						   FROM Actor, Director D
						   WHERE Actor.id = D.id AND (Actor.last <> D.last OR Actor.first <> D.first OR Actor.dob <> D.dob OR Actor.dod <> D.dod))))
		ENGINE = InnoDB;
		
CREATE TABLE Director(
		id int NOT NULL,
		last varchar(20) NOT NULL,
		first varchar(20),
		dob date DEFAULT NULL,
		dod date DEFAULT NULL CHECK (dod > dob),
		PRIMARY KEY(id),
		CHECK (NOT EXISTS (SELECT *
						   FROM Director, Actor A
						   WHERE Director.id = A.id AND (Director.last <> A.last OR Director.first <> A.first OR Director.dob <> A.dob OR Director.dod <> A.dod))))
		ENGINE = InnoDB;
		
CREATE TABLE MovieGenre(
		mid int NOT NULL,
		genre varchar(20) NOT NULL,
		PRIMARY KEY(mid, genre),
		FOREIGN KEY (mid) REFERENCES Movie(id))
		ENGINE = InnoDB;
		
CREATE TABLE MovieDirector(
		mid int NOT NULL,
		did int NOT NULL,
		FOREIGN KEY (mid) REFERENCES Movie(id),
		FOREIGN KEY (did) REFERENCES Director(id))
		ENGINE = InnoDB;
		
CREATE TABLE MovieActor(
		mid int NOT NULL,
		aid int NOT NULL,
		role varchar(50),
		FOREIGN KEY (mid) REFERENCES Movie(id),
		FOREIGN KEY (aid) REFERENCES Actor(id))
		ENGINE = InnoDB;
		
CREATE TABLE Review(
		name varchar(20),
		time timestamp,
		mid int NOT NULL,
		rating int CHECK (0 <= rating AND rating <= 5),
		comment varchar(500) DEFAULT NULL,
		FOREIGN KEY (mid) REFERENCES Movie(id))
		ENGINE = InnoDB;
		
CREATE TABLE MaxPersonID(
		id int)
		ENGINE = InnoDB;
		
CREATE TABLE MaxMovieID(
		id int)
		ENGINE = InnoDB;
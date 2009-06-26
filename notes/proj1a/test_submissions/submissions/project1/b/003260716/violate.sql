 INSERT INTO Movie VALUES(NULL,'Bad id',2005, 'PG','Paramount');
 /*Movie id Primary keys cannot be null.
 Column 'id' cannot be null
 */ 
 
 INSERT INTO Actor VALUES(NULL,'ID','Bad','Male','1994-04-03',NULL);
 /*Actor id Primary keys cannot be null.
 Column 'id' cannot be null
 */ 
 
 INSERT INTO Director VALUES(NULL,'ID','Bad','1994-04-03',NULL);
  /*Director id Primary keys cannot be null.
 Column 'id' cannot be null
 */ 
 
 INSERT INTO MovieGenre VALUES(-85,'Bad Genre');
 /*mid has to link to a correct Movie.id. There is no Movie with id=-85
 Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
 */ 
 
 INSERT INTO MovieDirector VALUES(2,12987946554654651321);
  /*Did has to link to a correct Director.id. There is no Director with id=12987946554654651321
 Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`did`) REFERENCES `Director` (`id`))
 */ 
 
 INSERT INTO MovieDirector VALUES(654654654654654654654,12);
  /*mid has to link to a correct Movie.id. There is no Movie with id=654654654654654654654
 Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieDirector`, CONSTRAINT `MovieDirector_ibfk_2` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
 */ 
 
 INSERT INTO MovieActor VALUES(50000000000000000000000,12,'Role');
  /*mid has to link to a correct Movie.id. There is no Movie with id=50000000000000000000000
 Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
 */ 
 
 INSERT INTO MovieActor VALUES(11,123213215165,'Bad');
  /*aid has to link to a correct Actor.id. There is no Actor with id=-85
 Cannot add or update a child row: a foreign key constraint fails (`CS143/MovieActor`, CONSTRAINT `MovieActor_ibfk_2` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))
 */ 
 
 INSERT INTO Review VALUES('Review Dude',NULL,46546546546,5,'Review');
  /*mid has to link to a correct Movie.id. There is no Movie with id=46546546546
 Cannot add or update a child row: a foreign key constraint fails (`CS143/Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
 */ 
 
 INSERT INTO Movie VALUES(-5,'Bad id',2005, 'PG','Paramount');
 /*id values cannot be less than 1*/  
 
 INSERT INTO Actor VALUES(-6,'ID','Bad','Male','1994-04-03',NULL);
 /*id values cannot be less than 1*/
   
 INSERT INTO Director VALUES(-7,'ID','Bad','1994-04-03',NULL);
  /*id values cannot be less than 1*/ 
 
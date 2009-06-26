insert into Actor values (1,"b","Isabelle","Female",19750525,\N)
--violates Actor's primary key restraint
--ERROR DUPLICATE ENTRY '1' for key 1

insert into Director values (37146,"Lip","Aaron",19521112,\N
)
--violates Director's primary key restraint
--ERROR DUPLICATE ENTRY '37146' for key 37146

insert into Movie values (272,"Baw",1934,"PG","Fox Film )
--violates Movie's primary key constraint
--ERROR DUPLICATE ENTRY '272' for key 272

insert into Director values (12343,"Lip","Aaron",20201112,\N
)
--birthday is in the future

insert into Actor values (68999,"b","Isabelle","Mefale",19750525,\N)
--sex is not male or female or Male or Female

insert into Actor values (68998,"b","Isabelle","Female",19800525,19750525)
--date of birth is after date of death

insert into MovieActor values (12345, 10208,"Doorman"
)
--trying to insert a movie into MovieActor before inserted into Movie is not allowed

insert into MovieActor values (100, 68992,"Doorman"
)
--trying to insert a MovieActor before the actor is inserted

delete from Movie WHERE id = 3
--trying to delete a movie which has a listing in MovieDirector
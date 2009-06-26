-- Name: Angel Darquea
-- ID: 003512936
-- Date: 10/19/2008
-- Fall 2008
-- Project 1B - README

- Since actors and directors have unique IDs, it would be convinient I think
to have a Person table tha holds IDs (primary keys), last and first names of 
all actors and directors and then reference those IDs from the Actor and
Director table. Otherwise, we can run into a problem when adding an actor
with an ID that belongs to a director. Currently, this constrain is not checked
since it required, I believe, the creation of that additional Person table OR
the implemenation of the proper management of circular constrains (which seems
more complicated and less flexible).

-- insert into Movie Values(3366, 'Ueda', 'Hanako', 'animal', 20080101, 0);
-- supposedly this will show the error due to the check under sex.

insert into Movie Values(3391, 'Ueda Hanako', 2008, 'PG', 'Hanako Entertainment');
-- ERROR 1062 (23000): Duplicate entry '3391' for key 1
-- the error occured because id 3391 already exist and it is the primekey that no duplicates allowed

insert into MovieGenre Values(1999999, 'Horror');
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('TEST/MovieGenre', CONSTRAINT 'MovieGenre_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))
-- the error occured because id 1999999 does not exist in the Movie.id and it is referenced to the Movie table.

delete from Movie where id=300;
-- ERROR 1451 (23000): Cannot delete or update a parent row: a foreign key constraint fails ('TEST/MovieGenre', CONSTRAINT 'MovieGenre_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))
-- the error occured because Movie is the parent (referenced key for MovieGenre) that it cannot be deleted.

update MovieGenre set mid = mid*10;
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('TEST/MovieGenre', CONSTRAINT 'MovieGenre_ibfk_1' FOREIGN KEY ('mid') REFERENCES 'Movie' ('id'))
-- the error occured because MovieGenre.mid is the foreign key that it cannot be updated without modifying the referenced key (Movie.id) first.

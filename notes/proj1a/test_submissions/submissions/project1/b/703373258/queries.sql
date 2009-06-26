SELECT first, last FROM Actor WHERE (id) in (SELECT
id FROM Movie WHERE title = 'Die Another Day')

--Select the Actors in Die Another Day

select( count(select Actor from id)- count(select distinct aid from Movie))

--count the total actors and subtract the number of
--distinct actors

SELECT * FROM Actor WHERE dob < 19500101 & dod <>'/N'
--count the number of actors born before 1950 who
--are still alive


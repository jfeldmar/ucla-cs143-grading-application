SELECT first, last
FROM Movie M, MovieActor MA, Actor A
WHERE M.id=MA.mid AND A.id=MA.aid AND title='Die Another Day'

SELECT COUNT(DISTINCT MA1.aid)
FROM MovieActor MA1, MovieActor MA2
WHERE MA1.aid=MA2.aid AND MA1.mid<>MA2.mid

--Names of dead directors who directed more than one movie
SELECT D.first, D.last
FROM Director D, MovieDirector MD
WHERE D.id=MD.did AND D.dod < '2008-10-18'
GROUP BY did
HAVING COUNT(mid)>1
select count(*) from (select aid from MovieActor group by aid having count(mid) > 1) as t;

select first, last from Actor where id in (select aid from MovieActor where mid = (select id from
Movie where title = 'Die Another Day'));

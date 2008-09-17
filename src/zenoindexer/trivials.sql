insert into trivialwords (word)
  select word
    from (select distinct aid, word from words) words
    group by word
    having count(*) > (select count(distinct aid) * 0.7 from words);

delete from words where word in (
  select word
    from (select distinct aid, word from words) words
    group by word
    having count(*) > (select count(distinct aid) * 0.7 from words));


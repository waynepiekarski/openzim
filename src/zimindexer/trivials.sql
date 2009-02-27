  select word, count(*)
    from (select distinct aid, word from words) words
    group by word
  order by 2, 1

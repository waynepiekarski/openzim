create table words
(
  word     text not null primary key,
  aid      integer not null,
  pos      integer not null,

  foreign key (aid) references article
);

create table trivialwords
(
  word     text not null primary key
);

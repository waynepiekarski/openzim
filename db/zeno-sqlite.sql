create table article
(
  aid          integer primary key autoincrement,
  namespace    text    not null,
  title        text    not null,
  url          text    not null,
  redirect     text,     -- title of redirect target
  mimetype     integer,
  data         bytea,
  compression  integer   -- 0: unknown/not specified, 1: none, 2: zip
);

create index article_ix1 on article(namespace, title);

create table zenofile
(
  zid          integer primary key autoincrement,
  filename     text    not null,
  count        integer
);

create table zenoarticles
(
  zid          integer not null,
  aid          integer not null,
  direntpos    bigint,
  datapos      bigint,

  primary key (zid, aid),
  foreign key (zid) references zenofile,
  foreign key (aid) references article
);

create index zenoarticles_ix1 on zenoarticles(zid, direntpos);

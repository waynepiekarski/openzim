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

create table zenodata
(
  zid          integer not null,
  did          integer not null,
  data         bytea not null,
  primary key (zid, did)
);

create table zenoarticles
(
  zid          integer not null,
  aid          integer not null,
  sort         integer,
  direntlen    bigint,
  datapos      bigint,
  dataoffset   bigint,
  datasize     bigint,
  did          bigint,

  primary key (zid, aid),
  foreign key (zid) references zenofile,
  foreign key (aid) references article,
  foreign key (zid, did) references zenodata
);

create index zenoarticles_ix1 on zenoarticles(zid, direntlen);
create index zenoarticles_ix2 on zenoarticles(zid, sort);

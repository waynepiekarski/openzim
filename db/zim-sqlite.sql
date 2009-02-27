create table article
(
  aid          integer primary key autoincrement,
  namespace    text    not null,
  title        text    not null,
  url          text    not null,
  redirect     text,     -- title of redirect target
  mimetype     integer,
  data         bytea
);

create unique index article_ix1 on article(namespace, title);

create table zimfile
(
  zid          integer primary key autoincrement,
  filename     text    not null
);

create table zimdata
(
  zid          integer not null,
  did          integer not null,
  data         bytea not null,
  primary key (zid, did)
);

create table zimarticles
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
  foreign key (zid) references zimfile,
  foreign key (aid) references article
);

create index zimarticles_ix1 on zimarticles(zid, direntlen);
create index zimarticles_ix2 on zimarticles(zid, sort);

create table words
(
  word     text not null primary key,
  aid      integer not null,
  pos      integer not null,

  foreign key (aid) references article
);

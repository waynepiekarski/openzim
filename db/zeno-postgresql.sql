create table article
(
  aid          serial  not null primary key,
  namespace    text    not null,
  title        text    not null,
  redirect     text,     -- title of redirect target
  mimetype     integer,
  data         bytea
);

create unique index article_ix1 on article(namespace, title);

create table zenofile
(
  zid          serial  not null primary key,
  filename     text    not null
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
  foreign key (aid) references article
);

create index zenoarticles_ix1 on zenoarticles(zid, direntlen);
create index zenoarticles_ix2 on zenoarticles(zid, sort);

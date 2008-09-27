create table article
(
  aid          serial  not null primary key,
  namespace    text    not null,
  title        text    not null,
  url          text    not null,
  redirect     text,     -- title of redirect target
  mimetype     integer,
  data         bytea
);

create unique index article_ix1 on article(namespace, title);

create table category
(
  cid          serial  not null primary key,
  title        text    not null,
  description  bytea   not null
);

create table categoryarticles
(
  cid          integer not null,
  title        text  not null,
  article      text  not null,
  primary key (cid, title),
  foreign key (cid) references category
);

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

create table indexarticle
(
  zid          integer not null,
  xid          serial  not null,
  namespace    text    not null,
  title        text    not null,
  sort         integer,
  direntlen    bigint,
  datapos      bigint,
  dataoffset   bigint,
  datasize     bigint,
  did          bigint,

  primary key (zid, namespace, title),
  foreign key (zid) references zenofile
);

create index zenoarticles_ix1 on zenoarticles(zid, direntlen);
create index zenoarticles_ix2 on zenoarticles(zid, sort);

create table words
(
  word     text not null,
  aid      integer not null,
  pos      integer not null,
  weight   integer not null, -- 0: title/header, 1: subheader, 3: paragraph

  primary key (word, aid, pos),
  foreign key (aid) references article
);

create index words_ix1 on words(aid);

create table trivialwords
(
  word     text not null primary key
);

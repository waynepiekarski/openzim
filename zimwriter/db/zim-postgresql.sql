create table article
(
  aid          serial  not null primary key,
  namespace    text    not null,
  title        text    not null,
  url          text,
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
  aid          integer not null,
  primary key (cid, aid),
  foreign key (cid) references category,
  foreign key (aid) references article
);

create table zimfile
(
  zid          serial  not null primary key,
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
  parameter    bytea,

  primary key (zid, aid),
  foreign key (zid) references zimfile,
  foreign key (aid) references article
);

create index zimarticles_ix1 on zimarticles(zid, direntlen);
create index zimarticles_ix2 on zimarticles(zid, sort);

create table indexarticle
(
  zid          integer not null,
  xid          serial  not null,
  namespace    text    not null,
  title        text    not null,
  data         bytea,
  sort         integer,
  direntlen    bigint,
  datapos      bigint,
  dataoffset   bigint,
  datasize     bigint,
  did          bigint,
  parameter    bytea,

  primary key (zid, namespace, title),
  foreign key (zid) references zimfile
);

create index indexarticle_ix1 on indexarticle(zid, xid);
create index indexarticle_ix2 on indexarticle(zid, sort);

create table words
(
  word     text not null,
  pos      integer not null,
  aid      integer not null,
  weight   integer not null, -- 0: title/header, 1: subheader, 3: paragraph

  primary key (word, aid, pos),
  foreign key (aid) references article
);

create index words_ix1 on words(aid);

create table trivialwords
(
  word     text not null primary key
);

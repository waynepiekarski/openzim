/*
 * Copyright (C) 2008 Tommi Maekitalo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include "zenowriter.h"
#include <iostream>
#include <fstream>
#include <limits>
#include <cxxtools/log.h>
#include <cxxtools/loginit.h>
#include <cxxtools/arg.h>
#include <tntdb/connect.h>
#include <tntdb/statement.h>
#include <tntdb/transaction.h>
#include <zeno/fileheader.h>
#include "zenocompressor.h"

log_define("zeno.writer")

Zenowriter::Zenowriter(const char* basename_)
  : basename(basename_),
    minChunkSize(65536),
    compression(zeno::Dirent::zenocompNone),
    numThreads(4),
    createIndex(false)
{ 
}

tntdb::Connection& Zenowriter::getConnection()
{
  if (!conn)
  {
    conn = tntdb::connect(dburl);
    zid = getConnection()
              .prepare("select zid from zenofile where filename = :filename")
              .set("filename", basename)
              .selectValue()
              .getUnsigned();
  }

  return conn;
}

namespace
{
  struct KeyType
  {
    char ns;
    zeno::QUnicodeString title;
    KeyType() { }
    KeyType(char ns_, const zeno::QUnicodeString& title_)
      : ns(ns_),
        title(title_)
        { }
    bool operator< (const KeyType& k) const
    { return ns < k.ns || ns == k.ns && title < k.title; }
  };
}

void Zenowriter::prepareWordIndex()
{
  // create article entries in prepareWordIndex
  // and add index data in createWordIndex

  log_info("prepare word index");
  std::cout << "prepare word index   " << std::flush;

  // create indexarticle with namespace 'X' from words
  //
  // format of indexarticle:
  //   title contains the word
  //   parameter in dirent must be empty to distinquish from old format,
  //     which used the parameter field
  // the data:
  //   4 x integer: number of entries for each category (0 first)
  //   each entry:
  //     article index (integer)
  //     position of word in article (integer)
  //
  // In contrast the the previous format (directmedia format) we don't use
  // integer compression since the article data is compressed, which leads to
  // much better compression at the cost of slower access.
  //
  tntdb::Statement stmt = getConnection().prepare(
    "select distinct w.word"
    "  from words w"
    "  join zenoarticles z"
    "    on w.aid = z.aid"
    " where z.zid = :zid"
    " order by 1");
  stmt.set("zid", zid);

  tntdb::Statement insIndexArticle = getConnection().prepare(
    "insert into indexarticle"
    "  (zid, namespace, title, data)"
    " values (:zid, 'X', :title, :data)");
  insIndexArticle.set("zid", zid);

  tntdb::Transaction transaction(getConnection());
  unsigned count = 0;
  char currentChar = '\0';

  for (tntdb::Statement::const_iterator cur = stmt.begin(); cur != stmt.end(); ++cur)
  {
    tntdb::Row row = *cur;
    std::string word = row[0].getString();

    log_debug("word: \"" << word << '"');
    insIndexArticle.set("title", word)
        .execute();

    ++count;
    if (commitRate && count % commitRate == 0)
    {
      transaction.commit();
      transaction.begin();
    }

    if (!word.empty() && currentChar < word.at(0))
    {
      currentChar = word.at(0);
      log_info(currentChar);
      std::cout << currentChar << std::flush;
    }
  }

  transaction.commit();

  countArticles += count;
  std::cout << std::endl;

  log_info(countArticles << " articles");
  std::cout << countArticles << " articles" << std::endl;
}

void Zenowriter::createWordIndex()
{
  log_info("create word index");
  std::cout << "create word index    " << std::flush;

  // create indexarticle with namespace 'X' from words
  //
  // format of indexarticle:
  //   title contains the word
  //   parameter in dirent must be empty to distinquish from old format,
  //     which used the parameter field
  // the data:
  //   4 x integer: number of entries for each category (0 first)
  //   each entry:
  //     article index (integer)
  //     position of word in article (integer)
  //
  // In contrast the the previous format (directmedia format) we don't use
  // integer compression since the article data is compressed, which leads to
  // much better compression at the cost of slower access.
  //
  tntdb::Statement stmt = getConnection().prepare(
    "select w.word, w.weight, z.sort, w.pos"
    "  from words w"
    "  join zenoarticles z"
    "    on w.aid = z.aid"
    " where z.zid = :zid"
    " order by 1, 2, 3, 4");
  stmt.set("zid", zid);

  tntdb::Statement updIndexArticle = getConnection().prepare(
    "update indexarticle"
    "  set data = :data"
    "  where zid = :zid"
    "    and namespace = 'X'"
    "    and title = :title");
  updIndexArticle.set("zid", zid);

  std::string currentWord;
  char currentChar = '\0';

  // for each weight one vector of entries
  std::vector<ZenoIndexEntry> data[4];

  tntdb::Transaction transaction(getConnection());
  unsigned count = 0;

  for (tntdb::Statement::const_iterator cur = stmt.begin(); cur != stmt.end(); ++cur)
  {
    tntdb::Row row = *cur;
    std::string word = row[0].getString();
    unsigned weight = row[1].getUnsigned();
    unsigned idx = row[2].getUnsigned();
    unsigned pos = row[3].getUnsigned();

    log_debug("word \"" << word << "\" idx=" << idx << " pos=" << pos << " weight=" << weight);

    if (word != currentWord)
    {
      if (!currentWord.empty())
      {
        // create document data
        std::ostringstream document;

        log_debug("create document data for word \"" << currentWord << '"');

        zeno::size_type header[4];
        for (unsigned c = 0; c < 4; ++c)
          header[c] = fromLittleEndian<zeno::size_type>(data[c].size());

        document.write(reinterpret_cast<char*>(&header[0]), sizeof(zeno::size_type) * 4);

        for (unsigned c = 0; c < 4; ++c)
        {
          for (std::vector<ZenoIndexEntry>::const_iterator it = data[c].begin();
            it != data[c].end(); ++it)
          {
            document << *it;
          }
          data[c].clear();
        }

        // update indexarticle
        log_debug("document size=" << document.str().size());

        tntdb::Blob bdata(document.str().data(), document.str().size());
        updIndexArticle.set("title", currentWord)
                       .set("data", bdata)
                       .execute();

        ++count;
        if (commitRate && count % commitRate == 0)
        {
          transaction.commit();
          transaction.begin();
        }

      }

      if (!word.empty() && currentChar < word.at(0))
      {
        currentChar = word.at(0);
        log_info(currentChar);
        std::cout << currentChar << std::flush;
      }

      currentWord = word;
    }

    data[weight].push_back(ZenoIndexEntry(idx, pos));
  }

  transaction.commit();

  std::cout << std::endl;
}

void Zenowriter::prepareSort()
{
  log_info("sort articles");
  std::cout << "sort articles       " << std::flush;

  typedef std::map<KeyType, int> ArticlesType;  // positive => aid, negative => -(1+xid)

  ArticlesType articles;
  tntdb::Statement stmt = getConnection().prepare(
    "select a.aid, a.namespace, a.title"
    "  from zenoarticles z"
    "  join article a"
    "    on a.aid = z.aid"
    "  left outer join article r"
    "    on r.namespace = a.namespace"
    "   and r.title = a.redirect"
    "  left outer join zenoarticles rz"
    "    on rz.zid = z.zid"
    "   and rz.aid = r.aid"
    " where z.zid = :zid"
    "   and (a.redirect is null or rz.aid is not null)"
    " union "
    "select -(1 + xid), namespace, title"
    "  from indexarticle"
    " where zid = :zid");
  stmt.set("zid", zid);

  unsigned count = 0;
  unsigned process = 0;

  for (tntdb::Statement::const_iterator cur = stmt.begin();
       cur != stmt.end(); ++cur)
  {
    tntdb::Row row = *cur;
    int aid = row[0].getInt();
    char ns = row[1].getChar();
    std::string title = row[2].getString();
    articles.insert(ArticlesType::value_type(KeyType(ns, zeno::QUnicodeString(title)), aid));

    ++count;
    while (process < count * 50 / countArticles + 1)
    {
      log_info("sort articles " << process << '%');
      std::cout << ' ' << process << '%' << std::flush;
      process += 10;
    }
  }

  tntdb::Statement upd = getConnection().prepare(
    "update zenoarticles"
    "   set sort = :sort"
    " where zid = :zid"
    "   and aid = :aid");
  upd.set("zid", zid);

  tntdb::Statement updx = getConnection().prepare(
    "update indexarticle"
    "   set sort = :sort"
    " where zid = :zid"
    "   and xid = :xid");
  updx.set("zid", zid);

  log_info("write data to database");

  tntdb::Transaction transaction(getConnection());

  unsigned sort = 0;

  for (ArticlesType::const_iterator it = articles.begin(); it != articles.end(); ++it)
  {
    int id = it->second;
    if (id >= 0)
    {
      log_debug("update article " << id << " with sort " << sort);
      upd.set("sort", sort++)
         .set("aid", id)
         .execute();
    }
    else
    {
      log_debug("update indexarticle " << -(1+id) << " (" << id << ") with sort " << sort);
      updx.set("sort", sort++)
          .set("xid", -(1+id))
          .execute();
    }

    if (commitRate && sort % commitRate == 0)
    {
      transaction.commit();
      transaction.begin();
    }

    ++count;
    while (process < count * 50 / countArticles + 1)
    {
      log_info("sort articles " << process << '%');
      std::cout << ' ' << process << '%' << std::flush;
      process += 10;
    }
  }

  transaction.commit();

  std::cout << ' ' << count/2 << std::endl;
}

void Zenowriter::init()
{
  tntdb::Statement stmt = getConnection().prepare(
    "select count(*)"
    "  from zenoarticles z"
    "  join article a"
    "    on a.aid = z.aid"
    "  left outer join article r"
    "    on r.namespace = a.namespace"
    "   and r.title = a.redirect"
    "  left outer join zenoarticles rz"
    "    on rz.zid = z.zid"
    "   and rz.aid = r.aid"
    " where z.zid = :zid"
    "   and (a.redirect is null or rz.aid is not null)");

  countArticles = stmt.set("zid", zid)
                      .selectValue()
                      .getUnsigned();

  log_info(countArticles << " articles");
  std::cout << countArticles << " articles" << std::endl;
}

void Zenowriter::cleanup()
{
  log_info("cleanup database");
  std::cout << "cleanup database    " << std::flush;

  log_debug("delete zenodata");
  tntdb::Statement stmt = getConnection().prepare(
    "delete from zenodata"
    " where zid = :zid");
  stmt.set("zid", zid)
      .execute();

  log_debug("delete indexarticle");
  stmt = getConnection().prepare(
    "delete from indexarticle"
    " where zid = :zid");
  stmt.set("zid", zid)
      .execute();

  log_debug("update zenoarticles");
  stmt = getConnection().prepare(
    "update zenoarticles"
    "   set sort = null,"
    "       direntlen = null,"
    "       datapos = null,"
    "       dataoffset = null,"
    "       datasize = null,"
    "       did = null"
    " where zid = :zid");
  stmt.set("zid", zid)
      .execute();

  std::cout << std::endl;
}

void Zenowriter::prepareFile()
{
  cleanup();

  if (createIndex)
    prepareWordIndex();

  prepareSort();

  if (createIndex)
    createWordIndex();

  log_info("prepare file");
  std::cout << "prepare file        " << std::flush;

  {
     // write redirect index into dataoffset since it is a shared field in dirent-structure
    tntdb::Statement updRedirect = getConnection().prepare(
      "update zenoarticles"
      "   set direntlen  = :direntlen,"
      "       dataoffset = :redirect"
      " where zid = :zid"
      "   and aid = :aid");
    updRedirect.set("zid", zid);

    tntdb::Statement stmt = getConnection().prepare(
      "select a.aid, a.title, a.mimetype, a.redirect, rz.sort, 'A'"
      "  from zenoarticles z"
      "  join article a"
      "    on a.aid = z.aid"
      "  left outer join article r"
      "    on r.namespace = a.namespace"
      "   and r.title = a.redirect"
      "  left outer join zenoarticles rz"
      "    on rz.zid = z.zid"
      "   and rz.aid = r.aid"
      " where z.zid = :zid"
      "   and (a.redirect is null or rz.aid is not null)"
      " union "
      "select -(1 + xid), title, 7, null, sort, 'X'"
      "  from indexarticle"
      " where zid = :zid"
      " order by 4, 1");
    stmt.set("zid", zid);

    tntdb::Statement stmtSelData = getConnection().prepare(
      "select data"
      "  from article"
      " where aid = :aid");
    tntdb::Statement stmtSelDataX = getConnection().prepare(
      "select data"
      "  from indexarticle"
      " where zid = :zid"
      "   and xid = :xid");
    stmtSelDataX.set("zid", zid);

    unsigned count = 0;
    unsigned process = 0;

    zeno::ZenoCompressor compressor(dburl, zid, numThreads);
    zeno::CompressJob job;
    for (tntdb::Statement::const_iterator cur = stmt.begin();
         cur != stmt.end(); ++cur)
    {
      tntdb::Row row = *cur;
      int aid = row[0].getInt();
      std::string title = row[1].getString();
      zeno::Dirent::MimeType mimetype(static_cast<zeno::Dirent::MimeType>(row[2].getUnsigned()));

      log_debug("process article \"" << title << '"');

      unsigned direntlen = zeno::Dirent::headerSize + title.size();
      tntdb::Blob articledata;
      unsigned redirect = std::numeric_limits<unsigned>::max();
      unsigned dataoffset = job.data.size();

      if (row[3].isNull())
      {
        // article
        tntdb::Value v;
        if (aid >= 0)
          v = stmtSelData.set("aid", aid)
                         .selectValue();
        else
          v = stmtSelDataX.set("xid", -(1 + aid))
                          .selectValue();
        v.getBlob(articledata);

        if (!job.data.empty() && (compression == zeno::Dirent::zenocompNone
                           || !mimeDoCompress(mimetype)
                           || job.data.size() + articledata.size() / 2 >= minChunkSize))
        {
          log_debug("insert data chunk");
          job.compression = compression;
          compressor.put(job);
          job.articles.clear();
          dataoffset = 0;
          job.data.clear();
        }

        log_debug("add article " << aid << ": \"" << title << '"');

        job.articles.push_back(
            zeno::CompressJob::Article(
                aid, direntlen, dataoffset, articledata.size()));

        job.data.append(articledata.data(), articledata.size());

        if (!mimeDoCompress(mimetype) && !job.data.empty())
        {
          log_debug("insert non compression data");
          job.compression = zeno::Dirent::zenocompNone;
          compressor.put(job);
          job.articles.clear();
          dataoffset = 0;
          job.data.clear();
        }
      }
      else
      {
        // redirect
        redirect = row[4].getUnsigned();

        log_debug("update redirect " << aid << ": \"" << title << '"');
        updRedirect.set("aid", aid)
                   .set("direntlen", direntlen)
                   .set("redirect", redirect)
                   .execute();
      }

      if (!compressor.getErrorMessage().empty())
      {
        std::cout << std::endl;
        throw std::runtime_error(compressor.getErrorMessage());
      }

      ++count;
      while (process < count * 100 / countArticles + 1)
      {
        log_info("prepare file " << process << '%');
        std::cout << ' ' << process << '%' << std::flush;
        process += 10;
      }
    }

    if (!job.data.empty())
    {
      job.compression = compression;
      compressor.put(job);
    }

    std::cout << ' ' << count << std::endl;
  }

  log_info("file prepared");
}

void Zenowriter::writeHeader(std::ostream& ofile)
{
  log_info("write header");
  std::cout << "write header        " << std::flush;

  unsigned indexPtrPos = zeno::Fileheader::headerSize + zeno::Fileheader::headerFill;
  unsigned indexPos = indexPtrPos + countArticles * 4;

  tntdb::Statement stmtIndexLen = getConnection().prepare(
    "select sum(direntlen)"
    "  from (select direntlen"
    "          from zenoarticles"
    "         where zid = :zid"
    "       union all"
    "        select direntlen"
    "          from indexarticle"
    "         where zid = :zid) as zenoarticles");
  indexLen = stmtIndexLen.set("zid", zid)
                         .selectValue()
                         .getUnsigned();

  header.setCount(countArticles);
  header.setIndexPos(indexPos);
  header.setIndexLen(indexLen);
  header.setIndexPtrPos(indexPtrPos);
  header.setIndexPtrLen(countArticles * 4);

  ofile << header;

  std::cout << std::endl;
}

void Zenowriter::writeIndexPtr(std::ostream& ofile)
{
  log_info("write index ptr");
  std::cout << "write index ptr     " << std::flush;

  tntdb::Statement stmt = getConnection().prepare(
    "select sort, direntlen"
    "  from zenoarticles"
    " where zid = :zid"
    "   and direntlen is not null"
    " union "
    "select sort, direntlen"
    "  from indexarticle"
    " where zid = :zid"
    " order by sort");
  stmt.set("zid", zid);

  zeno::size_type dirpos = 0;

  unsigned count = 0;
  unsigned process = 0;
  for (tntdb::Statement::const_iterator cur = stmt.begin();
       cur != stmt.end(); ++cur)
  {
    zeno::size_type ptr0 = fromLittleEndian<zeno::size_type>(&dirpos);
    ofile.write(reinterpret_cast<const char*>(&ptr0), sizeof(ptr0));
    unsigned direntlen = cur->getUnsigned(1);
    dirpos += direntlen;
    log_debug("direntlen=" << direntlen << " dirpos=" << dirpos);

    ++count;
    while (process < count * 100 / countArticles + 1)
    {
      log_info("write index ptr " << process << '%');
      std::cout << ' ' << process << '%' << std::flush;
      process += 10;
    }
  }

  std::cout << ' ' << count << std::endl;
}

void Zenowriter::writeDirectory(std::ostream& ofile)
{
  log_info("write directory");
  std::cout << "write directory     " << std::flush;

  tntdb::Statement stmt = getConnection().prepare(
    "select a.namespace, a.title, a.redirect, a.mimetype, z.datapos,"
    "       z.dataoffset, z.datasize, length(d.data), z.sort"
    "  from article a"
    "  join zenoarticles z"
    "    on z.aid = a.aid"
    "  left outer join article r"
    "    on r.namespace = a.namespace"
    "   and r.title = a.redirect"
    "  left outer join zenoarticles rz"
    "    on rz.zid = z.zid"
    "   and rz.aid = r.aid"
    "  left outer join zenodata d"
    "    on d.zid = z.zid"
    "   and d.did = z.did"
    " where z.zid = :zid"
    "   and (a.redirect is null or rz.zid is not null)"
    " union "
    "select namespace, title, null, 7, datapos,"
    "       dataoffset, datasize, length(d.data), sort"
    "  from indexarticle a"
    "  join zenodata d"
    "    on d.zid = a.zid"
    "   and d.did = a.did"
    " where a.zid = :zid"
    " order by 9");
  stmt.set("zid", zid);

  unsigned count = 0;
  unsigned process = 0;

  zeno::offset_type database = header.getIndexPos() + indexLen;
  for (tntdb::Statement::const_iterator cur = stmt.begin();
       cur != stmt.end(); ++cur)
  {
    tntdb::Row row = *cur;

    char ns = row[0].getChar();
    std::string title = row[1].getString();
    std::string redirect;
    if (!row[2].isNull())
      redirect = row[2].getString();
    zeno::Dirent::MimeType mimetype = static_cast<zeno::Dirent::MimeType>(row[3].isNull() ? 0 : row[3].getUnsigned());
    zeno::offset_type datapos = row[4].getUnsigned64();
    unsigned dataoffset = row[5].getUnsigned();
    unsigned datasize = row[6].getUnsigned();
    unsigned datalen = row[7].isNull() ? 0 : row[7].getUnsigned();

    zeno::Dirent dirent;
    dirent.setOffset(database + datapos);
    dirent.setArticleOffset(dataoffset);
    dirent.setArticleSize(datasize);
    dirent.setSize(datalen);
    dirent.setCompression(mimeDoCompress(mimetype) ? compression : zeno::Dirent::zenocompDefault);
    dirent.setMimeType(mimetype);
    dirent.setRedirectFlag(!redirect.empty());
    dirent.setNamespace(ns);
    dirent.setTitle(title);
    log_debug("write dirent for \"" << title << "\" redirect is " << dirent.getRedirectFlag());

    ofile << dirent;

    ++count;
    while (process < count * 100 / countArticles + 1)
    {
      log_info("write directory " << process << '%');
      std::cout << ' ' << process << '%' << std::flush;
      process += 10;
    }
  }

  std::cout << ' ' << count << std::endl;
}

void Zenowriter::writeData(std::ostream& ofile)
{
  tntdb::Statement stmt = getConnection().prepare(
    "select count(*) from zenodata where zid = :zid");
  unsigned countDatachunks = stmt.set("zid", zid).selectValue().getUnsigned();

  log_info("write data");
  std::cout << "write data          " << std::flush;

  stmt = getConnection().prepare(
    "select data"
    "  from zenodata"
    " where zid = :zid"
    " order by did");
  stmt.set("zid", zid);

  unsigned count = 0;
  unsigned process = 0;

  for (tntdb::Statement::const_iterator cur = stmt.begin();
       cur != stmt.end(); ++cur)
  {
    tntdb::Row row = *cur;
    tntdb::Blob data;
    row[0].getBlob(data);
    ofile.write(data.data(), data.size());

    ++count;
    while (process < count * 100 / countDatachunks + 1)
    {
      log_info("write data " << process << '%');
      std::cout << ' ' << process << '%' << std::flush;
      process += 10;
    }
  }

  std::cout << std::endl;
}

void Zenowriter::outputFile()
{
  std::string filename;
  if (outdir.empty())
    filename = basename;
  else
    filename = outdir + '/' + basename;

  std::ofstream zenoFile(filename.c_str());
  if (!zenoFile)
    throw std::runtime_error("cannot open output file " + filename);

  writeHeader(zenoFile);
  writeIndexPtr(zenoFile);
  writeDirectory(zenoFile);
  writeData(zenoFile);

  log_info(filename << " created");
}

int main(int argc, char* argv[])
{
  try
  {
    log_init();

    cxxtools::Arg<std::string> dburl(argc, argv, "--db", "postgresql:dbname=zeno");
    cxxtools::Arg<unsigned> minChunkSize(argc, argv, 's', 512);
    cxxtools::Arg<bool> compressZlib(argc, argv, 'z');
    cxxtools::Arg<bool> compressLzma(argc, argv, 'l');
    cxxtools::Arg<bool> noCompress(argc, argv, 'n');
    cxxtools::Arg<unsigned> commitRate(argc, argv, 'C', 10000);
    cxxtools::Arg<bool> prepareOnly(argc, argv, 'p');
    cxxtools::Arg<bool> generateOnly(argc, argv, 'g');
    cxxtools::Arg<unsigned> numThreads(argc, argv, 'j', 4);
    cxxtools::Arg<bool> createIndex(argc, argv, 'x');

    if (argc != 2)
    {
      std::cerr << "usage: " << argv[0] << " [options] zenofile\n"
                   "\t--db dburl     tntdb-dburl (default: postgresql:dbname=zeno)\n"
                   "\t-s number      chunk size in kBytes (default: 512)\n"
                   "\t-z             use zlib\n"
                   "\t-l             use lzma (experimental)\n"
                   "\t-n             disable compression (default: bzip2)\n"
                   "\t-C number      commit rate (default: 10000)\n"
                   "\t-p             prepare only\n"
                   "\t-g             generate only\n"
                   "\t-j number      number of parallel compressor threads (default: 4)\n";
      return -1;
    }

    Zenowriter app(argv[1]);
    app.setDburl(dburl);
    app.setCommitRate(commitRate);
    app.setMinChunkSize(minChunkSize * 1024);
    if (compressZlib)
      app.setCompression(zeno::Dirent::zenocompZip);
    else if (compressLzma)
      app.setCompression(zeno::Dirent::zenocompLzma);
    else if (noCompress)
      app.setCompression(zeno::Dirent::zenocompNone);
    else
      app.setCompression(zeno::Dirent::zenocompBzip2);

    app.setNumThreads(numThreads);
    app.setCreateIndex(createIndex);

    app.init();

    if (!generateOnly)
      app.prepareFile();

    if (!prepareOnly)
      app.outputFile();
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}


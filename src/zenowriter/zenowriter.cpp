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
    numThreads(4)
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

void Zenowriter::prepareSort()
{
  log_info("sort articles");
  std::cout << "sort articles       " << std::flush;

  typedef std::map<KeyType, unsigned> ArticlesType;

  ArticlesType articles;
  tntdb::Statement stmt = getConnection().prepare(
    "select a.aid, a.namespace, a.title"
    "  from zenoarticles z"
    "  join article a"
    "    on a.aid = z.aid"
    " where z.zid = :zid");
  stmt.set("zid", zid);

  unsigned count = 0;
  unsigned process = 0;

  for (tntdb::Statement::const_iterator cur = stmt.begin();
       cur != stmt.end(); ++cur)
  {
    tntdb::Row row = *cur;
    unsigned aid = row[0].getUnsigned();
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

  log_info("write data to database");

  tntdb::Transaction transaction(getConnection());

  unsigned sort = 0;

  for (ArticlesType::const_iterator it = articles.begin(); it != articles.end(); ++it)
  {
    upd.set("sort", sort++)
       .set("aid", it->second)
       .execute();

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

  std::cout << std::endl;
}

void Zenowriter::init()
{
  tntdb::Statement stmt = getConnection().prepare(
    "select count(*) from zenoarticles"
    " where zid = :zid");

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

  tntdb::Statement stmt = getConnection().prepare(
    "delete from zenodata"
    " where zid = :zid");

  stmt.set("zid", zid)
      .execute();

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
  prepareSort();

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
      "select a.aid, a.title, a.mimetype, a.redirect, rz.sort"
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
      " order by z.sort, a.aid");
    stmt.set("zid", zid);

    tntdb::Statement stmtSelData = getConnection().prepare(
      "select data"
      "  from article"
      " where aid = :aid");

    unsigned dataoffset = 0;
    unsigned count = 0;
    unsigned process = 0;

    zeno::ZenoCompressor compressor(dburl, zid, numThreads);
    zeno::CompressJob job;
    for (tntdb::Statement::const_iterator cur = stmt.begin();
         cur != stmt.end(); ++cur)
    {
      tntdb::Row row = *cur;
      unsigned aid = row[0].getUnsigned();
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
        tntdb::Value v = stmtSelData.set("aid", aid)
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

      while (process < ++count * 100 / countArticles + 1)
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

    std::cout << std::endl;
  }

  log_info("file prepared");
}

void Zenowriter::writeHeader(std::ostream& ofile)
{
  log_info("write header");
  std::cout << "write header        " << std::flush;

  unsigned indexPtrPos = zeno::Fileheader::headerSize + zeno::Fileheader::headerFill;

  tntdb::Statement stmtArticleCount = getConnection().prepare(
    "select count(*)"
    "  from zenoarticles"
    " where zid = :zid");
  unsigned articleCount = stmtArticleCount.set("zid", zid)
                                          .selectValue()
                                          .getUnsigned();

  unsigned indexPos = indexPtrPos + articleCount * 4;

  tntdb::Statement stmtIndexLen = getConnection().prepare(
    "select sum(direntlen)"
    "  from zenoarticles"
    " where zid = :zid");
  indexLen = stmtIndexLen.set("zid", zid)
                         .selectValue()
                         .getUnsigned();

  header.setCount(articleCount);
  header.setIndexPos(indexPos);
  header.setIndexLen(indexLen);
  header.setIndexPtrPos(indexPtrPos);
  header.setIndexPtrLen(articleCount * 4);

  ofile << header;

  std::cout << std::endl;
}

void Zenowriter::writeIndexPtr(std::ostream& ofile)
{
  log_info("write index ptr");
  std::cout << "write index ptr     " << std::flush;

  tntdb::Statement stmt = getConnection().prepare(
    "select direntlen"
    "  from zenoarticles"
    " where zid = :zid"
    " order by sort, aid");
  stmt.set("zid", zid);

  zeno::size_type dirpos = 0;

  unsigned count = 0;
  unsigned process = 0;
  for (tntdb::Statement::const_iterator cur = stmt.begin();
       cur != stmt.end(); ++cur)
  {
    zeno::size_type ptr0 = fromLittleEndian<zeno::size_type>(&dirpos);
    ofile.write(reinterpret_cast<const char*>(&ptr0), sizeof(ptr0));
    unsigned direntlen = cur->getUnsigned(0);
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

  std::cout << std::endl;
}

void Zenowriter::writeDirectory(std::ostream& ofile)
{
  log_info("write directory");
  std::cout << "write directory     " << std::flush;

  tntdb::Statement stmt = getConnection().prepare(
    "select a.namespace, a.title, a.redirect, a.mimetype, z.datapos,"
    "       z.dataoffset, z.datasize, length(d.data)"
    "  from article a"
    "  join zenoarticles z"
    "    on z.aid = a.aid"
    "  left outer join zenodata d"
    "    on d.zid = z.zid"
    "   and d.did = z.did"
    " where z.zid = :zid"
    " order by z.sort, a.aid");
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

    ofile << dirent;

    ++count;
    while (process < count * 100 / countArticles + 1)
    {
      log_info("write directory " << process << '%');
      std::cout << ' ' << process << '%' << std::flush;
      process += 10;
    }
  }

  std::cout << std::endl;
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
    cxxtools::Arg<unsigned> minChunkSize(argc, argv, 's', 256);
    cxxtools::Arg<bool> compressZlib(argc, argv, 'z');
    cxxtools::Arg<bool> noCompress(argc, argv, 'n');
    cxxtools::Arg<unsigned> commitRate(argc, argv, 'C', 10000);
    cxxtools::Arg<bool> prepareOnly(argc, argv, 'p');
    cxxtools::Arg<bool> generateOnly(argc, argv, 'g');
    cxxtools::Arg<unsigned> numThreads(argc, argv, 'j', 4);

    if (argc != 2)
    {
      std::cerr << "usage: " << argv[0] << " [options] zenofile\n"
                   "\t--db dburl     tntdb-dburl (default: postgresql:dbname=zeno)\n"
                   "\t-s number      chunk size in kBytes (default: 256)\n"
                   "\t-z             use zlib\n"
                   "\t-n             disable compression (default: bzip2)\n"
                   "\t-C number      commit rate (default: 10000)\n"
                   "\t-p             prepare only\n"
                   "\t-g             generate only\n";
                   "\t-j number      number of parallel compressor threads (default: 4)\n";
      return -1;
    }

    Zenowriter app(argv[1]);
    app.setDburl(dburl);
    app.setCommitRate(commitRate);
    app.setMinChunkSize(minChunkSize * 1024);
    if (compressZlib)
      app.setCompression(zeno::Dirent::zenocompZip);
    else if (noCompress)
      app.setCompression(zeno::Dirent::zenocompNone);
    else
      app.setCompression(zeno::Dirent::zenocompBzip2);
    app.setNumThreads(numThreads);

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


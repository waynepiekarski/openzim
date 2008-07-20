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
#include <cxxtools/log.h>
#include <cxxtools/loginit.h>
#include <cxxtools/arg.h>
#include <tntdb/connect.h>
#include <tntdb/statement.h>
#include <zeno/dirent.h>
#include <zeno/fileheader.h>
#include <zeno/deflatestream.h>

log_define("zeno.writer")

Zenowriter::Zenowriter(const char* basename_)
  : basename(basename_),
    minChunkSize(65536)
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

void Zenowriter::prepareSort()
{
  std::cout << "sort articles " << std::flush;

  typedef std::map<zeno::QUnicodeString, unsigned> ArticlesType;

  ArticlesType articles;
  tntdb::Statement stmt = getConnection().prepare(
    "select a.aid, a.title"
    "  from zenoarticles z"
    "  join article a"
    "    on a.aid = z.aid"
    " where z.zid = :zid");
  stmt.set("zid", zid);
  for (tntdb::Statement::const_iterator cur = stmt.begin();
       cur != stmt.end(); ++cur)
  {
    tntdb::Row row = *cur;
    unsigned aid = row[0].getUnsigned();
    std::string title = row[1].getString();
    articles.insert(ArticlesType::value_type(zeno::QUnicodeString(title), aid));
  }

  tntdb::Statement upd = getConnection().prepare(
    "update zenoarticles"
    "   set sort = :sort"
    " where zid = :zid"
    "   and aid = :aid");
  upd.set("zid", zid);

  unsigned sort = 0;
  for (ArticlesType::const_iterator it = articles.begin(); it != articles.end(); ++it)
    upd.set("sort", sort++)
       .set("aid", it->second)
       .execute();

  std::cout << std::endl;
}

void Zenowriter::cleanup()
{
  std::cout << "cleanup old data" << std::flush;

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
  prepareSort();

  std::cout << "prepare file " << std::flush;
  tntdb::Statement updArticle = getConnection().prepare(
    "update zenoarticles"
    "   set direntlen  = :direntlen,"
    "       datapos    = :datapos,"
    "       dataoffset = :dataoffset,"
    "       datasize   = :datasize,"
    "       did        = :did"
    " where zid = :zid"
    "   and aid = :aid");
  updArticle.set("zid", zid);

  tntdb::Statement insData = getConnection().prepare(
    "insert into zenodata"
    "  (zid, did, data)"
    " values (:zid, :did, :data)");
  insData.set("zid", zid);

  unsigned did = 0;
  std::string data;

  tntdb::Statement stmt = getConnection().prepare(
    "select a.aid, a.title, a.data, a.redirect"
    "  from zenoarticles z"
    "  join article a"
    "    on a.aid = z.aid"
    " where z.zid = :zid"
    " order by z.sort, a.aid");
  stmt.set("zid", zid);

  indexLen = 0;
  unsigned& direntpos(indexLen);
  unsigned datapos = 0;
  unsigned dataoffset = 0;
  for (tntdb::Statement::const_iterator cur = stmt.begin();
       cur != stmt.end(); ++cur)
  {
    tntdb::Row row = *cur;
    unsigned aid = row[0].getUnsigned();
    std::string title = row[1].getString();
    log_debug("process article \"" << title << '"');
    unsigned direntlen;
    std::string articledata;
    if (row[2].isNull())
    {
      // redirect
      std::string redirect = row[3].getString();
    }
    else
    {
      // article
      articledata = row[2].getString();
      if (!data.empty() && data.size() + articledata.size() / 2 >= minChunkSize)
      {
        datapos += insertDataChunk(data, did, insData);
        dataoffset = 0;
        data.clear();
        ++did;
      }
      data += articledata;
    }

    direntlen = zeno::Dirent::headerSize + title.size();

    updArticle.set("aid", aid)
              .set("direntlen", direntlen)
              .set("datapos", datapos)
              .set("dataoffset", dataoffset)
              .set("datasize", static_cast<unsigned>(articledata.size()))
              .set("did", did)
              .execute();

    dataoffset += articledata.size();
    direntpos += direntlen;
  }

  if (!data.empty())
    insertDataChunk(data, did, insData);

  std::cout << std::endl;
}

unsigned Zenowriter::insertDataChunk(const std::string& data, unsigned did, tntdb::Statement& insData)
{
  // insert into zenodata
  log_debug("compress data " << data.size());
  std::ostringstream u;
  zeno::DeflateStream ds(u);
  ds << data << std::flush;
  ds.end();
  log_debug("after compression " << u.str().size());

  log_debug("insert datachunk " << did << " with " << u.str().size() << " bytes");
  tntdb::Blob bdata(u.str().data(), u.str().size());
  insData.set("did", did)
         .set("data", bdata)
         .execute();
  return u.str().size();
}

void Zenowriter::writeHeader(std::ostream& ofile)
{
  std::cout << "write header " << std::flush;

  unsigned indexPtrPos = zeno::Fileheader::headerSize + zeno::Fileheader::headerFill;

  tntdb::Statement stmtArticleCount = getConnection().prepare(
    "select count(*)"
    "  from zenoarticles"
    " where zid = :zid");
  unsigned articleCount = stmtArticleCount.set("zid", zid)
                                          .selectValue()
                                          .getUnsigned();

  unsigned indexPos = indexPtrPos + articleCount * 4;

  if (indexLen == 0)
  {
    tntdb::Statement stmtIndexLen = getConnection().prepare(
      "select sum(direntpos)"
      "  from zenoarticles"
      " where zid = :zid");
    indexLen = stmtIndexLen.set("zid", zid)
                           .selectValue()
                           .getUnsigned();
  }

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
  std::cout << "write index ptr " << std::flush;

  tntdb::Statement stmt = getConnection().prepare(
    "select direntlen"
    "  from zenoarticles"
    " where zid = :zid"
    " order by sort, aid");
  stmt.set("zid", zid);

  zeno::size_type dataPos = 0;

  for (tntdb::Statement::const_iterator cur = stmt.begin();
       cur != stmt.end(); ++cur)
  {
    zeno::size_type ptr0 = fromLittleEndian<zeno::size_type>(&dataPos);
    ofile.write(reinterpret_cast<const char*>(&ptr0), sizeof(ptr0));
    unsigned direntlen = cur->getUnsigned(0);
    dataPos += direntlen;
    log_debug("direntlen=" << direntlen << " dataPos=" << dataPos);
  }

  std::cout << std::endl;
}

void Zenowriter::writeDirectory(std::ostream& ofile)
{
  std::cout << "write directory " << std::flush;

  tntdb::Statement stmt = getConnection().prepare(
    "select a.namespace, a.title, a.url, a.redirect, a.mimetype, z.datapos,"
    "       z.dataoffset, z.datasize, d.data"
    "  from article a"
    "  join zenoarticles z"
    "    on z.aid = a.aid"
    "  join zenodata d"
    "    on d.zid = z.zid"
    "   and d.did = z.did"
    " where z.zid = :zid"
    " order by z.sort, a.aid");
  stmt.set("zid", zid);

  zeno::size_type database = header.getIndexPos() + indexLen;
  for (tntdb::Statement::const_iterator cur = stmt.begin();
       cur != stmt.end(); ++cur)
  {
    tntdb::Row row = *cur;

    char ns = row[0].getChar();
    std::string title = row[1].getString();
    std::string url = row[2].getString();
    std::string redirect;
    if (!row[3].isNull())
      redirect = row[3].getString();
    unsigned mimetype = row[4].isNull() ? 0 : row[4].getUnsigned();
    unsigned datapos = row[5].getUnsigned();
    unsigned dataoffset = row[6].getUnsigned();
    unsigned datasize = row[7].getUnsigned();
    tntdb::Blob blob = row[8].getBlob();

    zeno::Dirent dirent;
    dirent.setOffset(database + datapos);
    dirent.setArticleOffset(dataoffset);
    dirent.setArticleSize(datasize);
    dirent.setSize(blob.size());
    dirent.setCompression(zeno::Dirent::zenocompZip);
    dirent.setMimeType(static_cast<zeno::Dirent::MimeType>(mimetype));
    dirent.setRedirectFlag(!redirect.empty());
    dirent.setNamespace(ns);
    dirent.setTitle(title);

    ofile << dirent;
  }

  std::cout << std::endl;
}

void Zenowriter::writeData(std::ostream& ofile)
{
  std::cout << "write data " << std::flush;

  tntdb::Statement stmt = getConnection().prepare(
    "select data"
    "  from zenodata"
    " where zid = :zid"
    " order by did");
  stmt.set("zid", zid);

  for (tntdb::Statement::const_iterator cur = stmt.begin();
       cur != stmt.end(); ++cur)
  {
    tntdb::Row row = *cur;
    tntdb::Blob blob = row[0].getBlob();
    ofile.write(blob.data(), blob.size());
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
}

int main(int argc, char* argv[])
{
  try
  {
    log_init();

    cxxtools::Arg<std::string> dburl(argc, argv, "--db", "postgresql:dbname=zeno");
    cxxtools::Arg<unsigned> minChunkSize(argc, argv, 's', 65536);

    if (argc <= 1)
    {
      std::cerr << "usage: " << argv[0] << " zenofile" << std::endl;
      return -1;
    }

    Zenowriter app(argv[1]);
    app.setDburl(dburl);
    app.setMinChunkSize(minChunkSize);
    app.cleanup();
    app.prepareFile();
    app.outputFile();
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}


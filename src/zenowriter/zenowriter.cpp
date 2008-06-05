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
#include <tntdb/connect.h>
#include <tntdb/statement.h>
#include <tntdb/row.h>
#include <zeno/dirent.h>
#include <cxxtools/arg.h>
#include <cxxtools/loginit.h>
#include <cxxtools/hirestime.h>

log_define("zeno.writer")

Zenowriter::Zenowriter(const char* filename_)
  : basename(filename_),
    commitRate(1)
{
}

tntdb::Connection& Zenowriter::getConnection()
{
  if (!conn)
  {
    conn = tntdb::connect(dburl);
    zid = conn.prepare("select zid from zenofile where filename = :filename")
              .set("filename", basename)
              .selectValue()
              .getUnsigned();
  }

  return conn;
}

void Zenowriter::readArticles()
{
  tntdb::Statement stmt = getConnection().prepare(
    "select a.aid, a.title, length(a.data), a.redirect"
    "  from zenoarticles z"
    "  join article a"
    "    on a.aid = z.aid"
    " where z.zid = :zid");

  stmt.set("zid", zid);

  unsigned count = 0;
  cxxtools::HiresTime t0 = cxxtools::HiresTime::gettimeofday();
  std::cout << "read articles     " << std::flush;

  for (tntdb::Statement::const_iterator cur = stmt.begin();
       cur != stmt.end(); ++cur)
  {
    tntdb::Row row = *cur;
    ArticleInfo a;
    a.aid = row[0].getUnsigned();
    std::string title = row[1].getString();
    a.titlesize = title.size();
    a.datasize = row[2].isNull() ? 0 : row[2].getUnsigned();
    a.redirectsize = row[3].isNull() ? 0 : row[3].getString().size();

    articleMap.insert(articleMap.end(),
                      ArticleMapType::value_type(zeno::QUnicodeString(title), a));

    if (++count % commitRate == 0)
      std::cout << '.' << std::flush;
  }

  cxxtools::HiresTime t1 = cxxtools::HiresTime::gettimeofday();
  std::cout << ' ' << count << "; " << (t1 - t0) << " seconds" << std::endl;
}

void Zenowriter::dump()
{
  for (ArticleMapType::const_iterator it = articleMap.begin();
       it != articleMap.end(); ++it)
  {
    std::cout << it->first << '\n';
  }
  std::cout.flush();
}

void Zenowriter::prepareFile()
{
  tntdb::Statement upd = getConnection().prepare(
    "update zenoarticles"
    "   set direntpos = :direntpos,"
    "       datapos = :datapos"
    " where zid = :zid"
    "   and aid = :aid");
  upd.set("zid", zid);

  indexLen = 0;
  dataLen = 0;
  unsigned count = 0;
  cxxtools::HiresTime t0 = cxxtools::HiresTime::gettimeofday();
  std::cout << "calculate offsets ";

  getConnection().beginTransaction();

  for (ArticleMapType::const_iterator it = articleMap.begin();
       it != articleMap.end(); ++it)
  {
    ArticleInfo a = it->second;
    unsigned aid = a.aid;
    unsigned tlen;
    unsigned dlen;
    if (a.redirectsize == 0)
    {
      // article
      tlen = a.titlesize;
      dlen = a.datasize;
    }
    else
    {
      // redirect
      tlen = a.redirectsize;
      dlen = 0;
    }

    upd.set("aid", aid)
       .set("direntpos", indexLen)
       .set("datapos", dataLen)
       .execute();

    indexLen += zeno::Dirent::headerSize + tlen;
    dataLen += dlen;

    if (++count % commitRate == 0)
    {
      getConnection().commitTransaction();
      getConnection().beginTransaction();
      std::cout << '.' << std::flush;
    }
  }

  getConnection().prepare("update zenofile set count = :count where zid = :zid")
      .set("zid", zid)
      .set("count", count)
      .execute();
      
  getConnection().commitTransaction();

  cxxtools::HiresTime t1 = cxxtools::HiresTime::gettimeofday();
  std::cout << ' ' << count << "; " << (t1 - t0) << " seconds" << std::endl;
}

void Zenowriter::writeHeader(std::ofstream& ofile)
{
  cxxtools::HiresTime t0 = cxxtools::HiresTime::gettimeofday();
  std::cout << "write header      " << std::flush;

  zeno::Fileheader header;

  indexPtrPos = zeno::Fileheader::headerSize + zeno::Fileheader::headerFill;
  indexPos = indexPtrPos + articleMap.size() * 4;

  header.setCount(articleMap.size());
  header.setIndexPos(indexPos);
  header.setIndexLen(indexLen);
  header.setIndexPtrPos(indexPtrPos);
  header.setIndexPtrLen(articleMap.size() * 4);

  ofile << header;
  cxxtools::HiresTime t1 = cxxtools::HiresTime::gettimeofday();
  std::cout << "; " << (t1 - t0) << " seconds" << std::endl;
}

void Zenowriter::writeIndexPtr(std::ofstream& ofile)
{
  cxxtools::HiresTime t0 = cxxtools::HiresTime::gettimeofday();
  std::cout << "write index       " << std::flush;
  unsigned count = 0;
  dataPos = 0;
  for (ArticleMapType::const_iterator it = articleMap.begin();
       it != articleMap.end(); ++it)
  {
    zeno::size_type ptr0 = fromLittleEndian<zeno::size_type>(&dataPos);
    ofile.write(reinterpret_cast<const char*>(&ptr0), sizeof(ptr0));
    dataPos += zeno::Dirent::headerSize + it->second.titlesize;

    if (++count % commitRate == 0)
      std::cout << '.' << std::flush;
  }

  cxxtools::HiresTime t1 = cxxtools::HiresTime::gettimeofday();
  std::cout << ' ' << count << "; " << (t1 - t0) << " seconds" << std::endl;
}

void Zenowriter::writeDirectory(std::ofstream& ofile)
{
  cxxtools::HiresTime t0 = cxxtools::HiresTime::gettimeofday();
  std::cout << "write directory   " << std::flush;

  tntdb::Statement stmt = getConnection().prepare(
    "select namespace, title, url, redirect, mimetype, compression"
    "  from article"
    " where aid = :aid");

  unsigned count = 0;
  zeno::size_type offset = indexPos + dataPos;
  for (ArticleMapType::const_iterator it = articleMap.begin();
       it != articleMap.end(); ++it)
  {
    const ArticleInfo& a = it->second;
    tntdb::Row row = stmt.set("aid", it->second.aid)
                         .selectRow();

    char ns = row[0].getChar();
    std::string title = row[1].getString();
    std::string url = row[2].getString();
    std::string redirect;
    if (!row[3].isNull())
      redirect = row[3].getString();
    unsigned mimetype = row[4].isNull() ? 0 : row[4].getUnsigned();
    unsigned compression = row[5].isNull() ? 0 : row[5].getUnsigned();

    zeno::Dirent dirent;
    dirent.setOffset(offset);
    dirent.setSize(a.datasize);
    dirent.setCompression(static_cast<zeno::Dirent::CompressionType>(compression));
    dirent.setMimeType(static_cast<zeno::Dirent::MimeType>(mimetype));
    dirent.setRedirectFlag(!redirect.empty());
    dirent.setNamespace(ns);
    dirent.setTitle(title);

    ofile << dirent;

    if (++count % commitRate == 0)
      std::cout << '.' << std::flush;

    offset += a.datasize;
  }

  cxxtools::HiresTime t1 = cxxtools::HiresTime::gettimeofday();
  std::cout << ' ' << count << "; " << (t1 - t0) << " seconds" << std::endl;
}

void Zenowriter::writeData(std::ofstream& ofile)
{
  cxxtools::HiresTime t0 = cxxtools::HiresTime::gettimeofday();
  std::cout << "write data        " << std::flush;

  tntdb::Statement stmt = getConnection().prepare(
    "select data"
    "  from article"
    " where aid = :aid");

  unsigned count = 0;
  for (ArticleMapType::const_iterator it = articleMap.begin();
       it != articleMap.end(); ++it)
  {
    const ArticleInfo& a = it->second;
    tntdb::Value value = stmt.set("aid", it->second.aid)
                             .selectValue();

    if (!value.isNull())
    {
      tntdb::Blob data;
      value.getBlob(data);
      ofile.write(data.data(), data.size());
    }

    if (++count % commitRate == 0)
      std::cout << '.' << std::flush;
  }

  cxxtools::HiresTime t1 = cxxtools::HiresTime::gettimeofday();
  std::cout << ' ' << count << "; " << (t1 - t0) << " seconds" << std::endl;
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

    cxxtools::Arg<bool> prepare(argc, argv, 'p');
    cxxtools::Arg<bool> dump(argc, argv, 'd');
    cxxtools::Arg<bool> output(argc, argv, 'c');
    cxxtools::Arg<unsigned> commitRate(argc, argv, 'C', 1000);
    cxxtools::Arg<std::string> outdir(argc, argv, 'O', ".");
    cxxtools::Arg<std::string> dburl(argc, argv, "--db", "postgresql:dbname=zeno");

    if (argc <= 1)
    {
      std::cerr << "usage: " << argv[0] << " zenofile" << std::endl;
      return -1;
    }

    Zenowriter app(argv[1]);

    app.setDburl(dburl);
    app.setCommitRate(commitRate);
    app.setOutdir(outdir);

    app.readArticles();

    if (dump)
      app.dump();
    else
    {
      if (prepare || !output)
        app.prepareFile();

      if (output || !prepare)
        app.outputFile();
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}


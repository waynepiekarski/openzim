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

#include <iostream>
#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/blob.h>
#include <tntdb/statement.h>
#include <zim/article.h>
#include <cxxtools/loginit.h>
#include <cxxtools/arg.h>

int main(int argc, char* argv[])
{
  try
  {
    log_init();

    cxxtools::Arg<std::string> dburl(argc, argv, "--db", "postgresql:dbname=zim");
    tntdb::Connection conn = tntdb::connect(dburl);

    tntdb::Statement selArticle = conn.prepare(
      "select aid, title, data"
      "  from article"
      " where compression is null"
      "   and data is not null"
      "   and redirect is null");

    tntdb::Statement updArticle = conn.prepare(
      "update article"
      "   set compression = :compression,"
      "       data = :data"
      " where aid = :aid");

    for (tntdb::Statement::const_iterator cur = selArticle.begin();
         cur != selArticle.end(); ++cur)
    {
      tntdb::Row row = *cur;
      unsigned aid      = row[0].getUnsigned();
      std::string title = row[1].getString();
      tntdb::Blob data  = row[2].getBlob();

      zim::Article article;
      article.setRawData(std::string(data.data(), data.size()));
      article.setCompression(zim::Dirent::zimcompNone);
      article.tryCompress();
      std::cout << "article " << aid << ": " << title << "\t=> compression " << article.getCompression() << std::endl;
      updArticle.set("aid", aid)
                .set("compression", static_cast<unsigned>(article.getCompression()))
                .set("data", tntdb::Blob(article.getRawData().data(), article.getRawData().size()))
                .execute();
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}


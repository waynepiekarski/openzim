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

int main(int argc, char* argv[])
{
  try
  {
    log_init();

    tntdb::Connection conn = tntdb::connect("postgresql:dbname=zim");

    tntdb::Statement selArticle = conn.prepare(
      "select aid, title, data"
      "  from article"
      " where compression = 2"
      "   and data is not null"
      "   and redirect is null");

    tntdb::Statement updArticle = conn.prepare(
      "update article"
      "   set compression = null,"
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
      article.setCompression(zim::Dirent::zimcompZip);
      std::cout << "article " << aid << ": " << title << std::endl;
      updArticle.set("aid", aid)
                .set("data", tntdb::Blob(article.getData().data(), article.getData().size()))
                .execute();
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}


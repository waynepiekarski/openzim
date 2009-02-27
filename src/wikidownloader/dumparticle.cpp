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
#include <sstream>
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
      "select title, compression, data, redirect"
      "  from article"
      " where aid = :aid");

    for (int a = 1; a < argc; ++a)
    {
      std::istringstream in(argv[a]);
      unsigned n;
      if (!in >> n)
        continue;

      tntdb::Row row = selArticle.set("aid", n)
                                 .selectRow();
      zim::Article article;
      article.setTitle(zim::QUnicodeString(row.getString(0)));
      article.setCompression(static_cast<zim::Dirent::CompressionType>(row.getInt(1)));
      tntdb::Blob data;
      row.getBlob(2, data);
      article.setRawData(std::string(data.data(), data.size()));
      article.setRedirectFlag(!row.isNull(3));

      std::cout << "aid=" << n << " title=" << article.getTitle() << " redirect=" << article.getRedirectFlag() << '\n'
        << article.getData() << std::endl;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}


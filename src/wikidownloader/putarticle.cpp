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
#include <fstream>
#include <tntdb/connect.h>
#include <tntdb/statement.h>
#include <cxxtools/arg.h>
#include <cxxtools/log.h>
#include <cxxtools/loginit.h>
#include <cxxtools/regex.h>
#include <zeno/dirent.h>

log_define("zeno.putarticle")

std::string mybasename(const std::string& fname)
{
  std::string::size_type b = fname.find_last_of('/');
  if (b == std::string::npos)
    b = 0;
  else
    ++b;

  std::string::size_type e = fname.find_last_of('.');

  return e == std::string::npos || e < b ? fname.substr(b) : fname.substr(b, e-b);
}

int main(int argc, char* argv[])
{
  try
  {
    log_init();

    cxxtools::Arg<std::string> dburl(argc, argv, "--db", "postgresql:dbname=zeno");
    cxxtools::Arg<char> ns(argc, argv, 'n', 'A');
    tntdb::Connection conn = tntdb::connect(dburl);

    tntdb::Statement insArticle = conn.prepare(
      "insert into article (namespace, mimetype, title, url, data)"
      " values (:namespace, :mimetype, :title, :url, :data)");
    tntdb::Statement insRedirect = conn.prepare(
      "insert into article (namespace, title, url, redirect)"
      " values (:namespace, :title, :url, :redirect)");

    for (int a = 1; a < argc; ++a)
    {
      std::ifstream in(argv[a]);
      if (!in)
      {
        std::cerr << "error reading " << argv[a] << std::endl;
        continue;
      }

      std::ostringstream data;
      data << in.rdbuf();

      static cxxtools::Regex regArticle("<h1( class=\"[^\"]*\")?>([^<]+)</h1>.*<!--\\s*start\\s+content\\s*-->(.*)<!--\\send\\s+content\\s*-->");
      static cxxtools::Regex regRedirect("<meta http-equiv=\"Refresh\".*<a href=\"[^\"]*\">([^<]*)");
      cxxtools::RegexSMatch match;
      if (regArticle.match(data.str(), match))
      {
        std::string title = match[2];
        std::string content = match[3];
        log_info(argv[a] << " is article; title=" << title << "; size=" << content.size());

        std::cout << "article: \"" << title << '"' << std::endl;

        // TODO fix links

        // insert article
        tntdb::Blob data(content.data(), content.size());
        insArticle.set("namespace", ns)
                  .set("mimetype", zeno::Dirent::zenoMimeTextHtml)
                  .set("title", title)
                  .set("url", title)
                  .set("data", data)
                  .execute();
      }
      else if (regRedirect.match(data.str(), match))
      {
        std::string title = mybasename(argv[a]);
        std::string::size_type pos;
        while ((pos = title.find('_')) != std::string::npos)
          title[pos] = ' ';

        // redirect identified
        std::string redirect = match[1];

        // TODO unhtml redirect

        log_info(argv[a] << " is redirect; target=" << redirect);

        std::cout << "redirect: \"" << title << "\" to \"" << redirect << '"' << std::endl;

        // insert redirect
        insRedirect.set("namespace", ns)
                   .set("title", title)
                   .set("url", title)
                   .set("redirect", redirect)
                   .execute();
      }
      else
      {
        std::cout << "unknown file " << argv[a] << std::endl;
        log_info(argv[a] << " not identified");
      }

    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}


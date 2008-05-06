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
#include <tntdb/transaction.h>
#include <tntdb/error.h>
#include <tntdb/statement.h>
#include <tntdb/blob.h>
#include <tnt/regex.h>
#include <cxxtools/arg.h>
#include <cxxtools/httprequest.h>
#include <cxxtools/httpreply.h>
#include <cxxtools/loginit.h>
#include <zeno/articlebase.h>

log_define("wikicrawler")

class Article : public zeno::ArticleBase
{
    std::string url;
    std::string heading;

  public:
    explicit Article(const std::string& url, const std::string& title);

    const std::string& getUrl() const      { return url; }
    const std::string& getHeading() const  { return heading; }
};

Article::Article(const std::string& url_, const std::string& title)
  : url(url_)
{
  setTitle(zeno::QUnicodeString(title));

  cxxtools::HttpRequest req("http://de.wikipedia.org/wiki/" + url);
  req.addHeader("User-Agent:", "TntReader/1.0.2");
  req.addHeader("Accept:", "*/*");
  cxxtools::HttpReply rep(req);

  std::ostringstream contentStream;
  contentStream << rep.rdbuf();
  std::string allContent = contentStream.str();
  //static tnt::Regex reg("<h1 class=\"firstHeading\">([^<]+)</h1>(.*)<!--\\s*NewPP limit report");
  static tnt::Regex reg("<h1 class=\"firstHeading\">([^<]+)</h1>.*<!--\\s*start\\s+content\\s*-->(.*)<!--\\send\\s+content\\s*-->");
  tnt::RegexSMatch match;
  if (reg.match(allContent, match))
  {
    heading = match.get(1);
    setData(match.get(2));
  }
  else
  {
    log_warn("article " << url << " not identified");
  }
}

class GetArticles
{
    std::string from;
    tntdb::Connection conn;
    tntdb::Statement selArticle;
    tntdb::Statement insArticle;
    tntdb::Statement insRedirect;

    unsigned count;

    void processArticle(const Article& article);
    void processRedirect(const Article& article);

  public:
    GetArticles(int& argc, char* argv[]);
    int run();
};

GetArticles::GetArticles(int& argc, char* argv[])
  : from(cxxtools::Arg<std::string>(argc, argv, 'f', "%20%")),
    count(0)
{
  conn = tntdb::connect("postgresql:dbname=zeno");

  selArticle = conn.prepare(
    "select 1 from article"
    " where namespace = :namespace"
    "   and title = :title");

  insArticle = conn.prepare(
    "insert into article (namespace, mimetype, title, url, data, compression)"
    " values (:namespace, :mimetype, :title, :url, :data, :compression)");
  insRedirect = conn.prepare(
    "insert into article (namespace, title, url, redirect)"
    " values (:namespace, :title, :url, :redirect)");
}

int GetArticles::run()
{
  while (true)
  {
    std::cout << "read index from " << from << std::endl;
    cxxtools::HttpRequest req("http://de.wikipedia.org/w/index.php?title=Spezial:Alle_Seiten&from=" + from);
    req.addHeader("User-Agent:", "TntReader/1.0.2");
    req.addHeader("Accept:", "*/*");
    cxxtools::HttpReply rep(req);
    if (rep.getReturnCode() != 200)
      break;

    std::ostringstream contentStream;
    contentStream << rep.rdbuf();
    std::string content = contentStream.str();

    static tnt::Regex reg("(<div class=\"allpagesredirect\">)?<a href=\"/wiki/([^\":]+)\" title=\"([^\":]+)\">\\2</a>(</div>)?");
    tnt::RegexSMatch match;
    while (reg.match(content, match))
    {
      bool redirect = match.offsetBegin(1) >= 0;
      std::string url = match.get(2);
      std::string title = match.get(3);
      content.erase(0, match.offsetEnd(3));
      ++count;

      try
      {
        selArticle.set("namespace", 'A')
                  .set("title", title)
                  .selectValue();
        std::cout << count << "\tskip " << title << std::endl;
      }
      catch (const tntdb::NotFound&)
      {
        Article article(url, title);
        std::cout << count
            << "\tredirect=" << redirect
            << "\turl=" << url
            << "\ttitle=" << title
            << "\tcontent size=" << article.getData().size() << std::endl;
        if (redirect)
          processRedirect(article);
        else
        {
          article.tryCompress(0.95);
          processArticle(article);
        }
      }

      from = title;
    }
  }
}

void GetArticles::processArticle(const Article& article)
{
  tntdb::Transaction trans(conn);
  insArticle.set("namespace", 'A')
            .set("mimetype", zeno::Dirent::zenoMimeTextHtml)
            .set("title", article.getTitle().toUtf8())
            .set("url", article.getUrl())
            .set("data", tntdb::Blob(article.getRawData().data(), article.getRawData().size()))
            .set("compression", article.getCompression())
            .execute();
  trans.commit();
}

void GetArticles::processRedirect(const Article& article)
{
  tntdb::Transaction trans(conn);
  insRedirect.set("namespace", 'A')
             .set("title", article.getTitle().toUtf8())
             .set("url", article.getUrl())
             .set("redirect", article.getHeading())
             .execute();
  trans.commit();
}

int main(int argc, char* argv[])
{
  try
  {
    log_init();
    GetArticles app(argc, argv);
    return app.run();
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}


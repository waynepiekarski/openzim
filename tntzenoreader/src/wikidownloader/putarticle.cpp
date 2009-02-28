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
#include <zim/dirent.h>

log_define("zim.putarticle")

class UrlRewriter
{
private:
	tntdb::Connection conn;
    tntdb::Statement selectUrl;
	std::map<std::string, std::string> cache;
	std::string zimname;

public:
	UrlRewriter(tntdb::Connection& c, const std::string& name = "Wikipedia")
        : conn(c),
          selectUrl(conn.prepare(
            "select title,redirect,namespace from article where url = :url")),
          zimname(name)
          { }

	std::string getUrl(const std::string& filename);
	void add(const char * filename, const std::string& url, char ns);

	static std::string encode(const char * in, char percent = '%');
	static std::string decode(const std::string& in);
};

std::string UrlRewriter::getUrl(const std::string& filename)
{
  std::ostringstream out;

  std::map<std::string, std::string>::const_iterator it = cache.find(filename);
  if (it != cache.end())
    return it->second;

  try
  {
    std::string encodedurl = decode(filename);

    tntdb::Row row = selectUrl.set("url", encodedurl).selectRow();
    if (row[1].isNull())
    {
      out << " href=\"/" << zimname << "/" << row[2].getString() << "/" << row[0].getString() << "\"";
    }
    else
    {
      out << " href=\"/" << zimname << "/" << row[2].getString() << "/" << row[1].getString() << "\"";
    }
    cache[filename] = out.str();
  }
  catch(const std::exception& e)
  {
    out << " href=\"../../../../" << filename << '"';
  }

  return out.str();
}

void UrlRewriter::add(const char * filename, const std::string& url, char ns)
{
  std::ostringstream out;
  std::string baseurl = encode(filename);

  out << " href=\"/" << zimname << "/" << ns << "/" << url << "\"";
  cache[baseurl] = out.str();

  std::ostringstream likeurlstream;
  likeurlstream << '%' << encode(filename, '_') << '%';
  std::ostringstream urlstream;
  urlstream << "../../../../" << baseurl;
  std::string likeurl = likeurlstream.str();
  std::string fullurl = urlstream.str();

  tntdb::Statement selectQuery = conn.prepare(
    "select aid,data from article where data like :likeurl");
  selectQuery.set("likeurl", likeurl);

  for (tntdb::Statement::const_iterator cur = selectQuery.begin();
       cur != selectQuery.end(); ++cur)
  {
    tntdb::Row row = *cur;

    tntdb::Blob contentBlob;
    row[1].getBlob(contentBlob);
    std::string content(contentBlob.data(), contentBlob.size());

    std::ostringstream zimurl;
    zimurl << "/" << zimname << "/A/" << url;

    int pos = content.find(fullurl);
    while(pos != -1)
    {
      content = content.replace(pos, fullurl.size(), zimurl.str());

      pos = content.find(fullurl, pos + fullurl.size());
    }

    contentBlob.assign(content.data(), content.size());

    tntdb::Statement updateUrl = conn.prepare(
      "update article set data = :data where aid = :aid");
    updateUrl.set("data", contentBlob);
    updateUrl.set("aid", row[0].getUnsigned());
    updateUrl.execute();
  }
}

std::string UrlRewriter::encode(const char * in, char percent)
{
  std::ostringstream decodedurl;

  for (const char* c = in; *c; ++c)
  {
    if (*c < 0)
    {
      static char hex[] = "0123456789ABCDEF";
      decodedurl << hex[(*c >> 4) & 0xf]
                 << hex[*c & 0xf];
    }
    else if (*c == '~')
    {
      decodedurl << percent << "7E";
    }
    else
    {
      decodedurl << *c;
    }
  }

  return decodedurl.str();
}

std::string UrlRewriter::decode(const std::string& in)
{
  std::ostringstream url;

  for(std::string::const_iterator c = in.begin();c != in.end();c++)
  {
    switch(*c)
    {
      case '%':
        std::string::value_type code[2];
        unsigned int decode;
        code[0] = *++c;
        code[1] = *++c;
        sscanf(code, "%2x", &decode);
        url.put(decode);
        break;
                  
      default:
        url << *c;
        break;
    }
  }

  return url.str();
}

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

    cxxtools::Arg<std::string> dburl(argc, argv, "--db", "postgresql:dbname=zim");
    cxxtools::Arg<char> ns(argc, argv, 'n', 'A');
    cxxtools::Arg<std::string> zimname(argc, argv, "--zimname", "Wikipedia");
    tntdb::Connection conn = tntdb::connect(dburl);

    tntdb::Statement insArticle = conn.prepare(
      "insert into article (namespace, mimetype, title, url, data)"
      " values (:namespace, :mimetype, :title, :url, :data)");
    tntdb::Statement insRedirect = conn.prepare(
      "insert into article (namespace, title, url, redirect)"
      " values (:namespace, :title, :url, :redirect)");

    UrlRewriter rewriter(conn, zimname);

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

      static cxxtools::Regex regHref("href=\"([^/]+/){4}([^\"]+\\.html)\"");

      static cxxtools::Regex regArticle("<h1( class=\"[^\"]*\")?>([^<]+)</h1>.*<!--\\s*start\\s+content\\s*-->(.*)<!--\\send\\s+content\\s*-->");
      static cxxtools::Regex regRedirect("<meta http-equiv=\"Refresh\".*<a href=\"[^\"]*\">([^<]*)");
      cxxtools::RegexSMatch match;
      if (regArticle.match(data.str(), match))
      {
        std::string title = match[2];
        std::string content = match[3];
        std::ostringstream cleancontent;

        log_info(argv[a] << " is article; title=" << title << "; size=" << content.size());

        std::cout << "article: \"" << title << '"' << std::endl;

        // TODO fix links
        
        cxxtools::RegexSMatch hrefmatch;
        while(regHref.match(content, hrefmatch))
        {
          cleancontent << content.substr(0, hrefmatch.offsetBegin(0));

          cleancontent << rewriter.getUrl(hrefmatch.get(2));

          content.erase(0, hrefmatch.offsetEnd(0));
        }

        cleancontent << content;
        content = cleancontent.str();

        // insert article
        tntdb::Blob data(content.data(), content.size());
        insArticle.set("namespace", ns)
                  .set("mimetype", zim::Dirent::zimMimeTextHtml)
                  .set("title", title)
                  .set("url", argv[a])
                  .set("data", data)
                  .execute();

        rewriter.add(argv[a], title, ns);
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
                   .set("url", argv[a])
                   .set("redirect", redirect)
                   .execute();

        rewriter.add(argv[a], redirect, ns);
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


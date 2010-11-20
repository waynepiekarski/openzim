/*
 * Copyright (C) 2010 Tommi Maekitalo
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
#include <cxxtools/arg.h>
#include <cxxtools/log.h>
#include <cxxtools/net/uri.h>
#include <cxxtools/xml/xmlreader.h>
#include <cxxtools/xml/startelement.h>
#include <cxxtools/xml/endelement.h>
#include <cxxtools/http/client.h>
#include <cxxtools/query_params.h>
#include <cxxtools/utf8codec.h>
#include <zim/writer/articlesource.h>
#include <zim/writer/zimcreator.h>
#include <zim/blob.h>

log_define("zim.writer.wiki")

class WikiArticle : public zim::writer::Article
{
    friend class WikiSource;

    char ns;
    std::string aid;
    std::string title;

  public:
    virtual std::string getAid() const;
    virtual char getNamespace() const;
    virtual std::string getUrl() const;
    virtual std::string getTitle() const;
    virtual zim::size_type getVersion() const;
    virtual bool isRedirect() const;
    virtual std::string getMimeType() const;
    virtual std::string getRedirectAid() const;
};

std::string WikiArticle::getAid() const
{
  return title;
}

char WikiArticle::getNamespace() const
{
  return ns;
}

std::string WikiArticle::getUrl() const
{
  return title;
}

std::string WikiArticle::getTitle() const
{
  return title;
}

zim::size_type WikiArticle::getVersion() const
{
  return 0;
}

bool WikiArticle::isRedirect() const
{
  return false;
}

std::string WikiArticle::getMimeType() const
{
  return "text/html";
}

std::string WikiArticle::getRedirectAid() const
{
  return std::string();
}

class WikiSource : public zim::writer::ArticleSource
{
    // parameters to fetch
    cxxtools::http::Client client;
    std::string url;

    // xml parser stuff
    std::istringstream xmlStream;
    cxxtools::xml::XmlReader xmlReader;
    cxxtools::xml::XmlReader::Iterator xmlIterator;

    // current article
    WikiArticle article;

    // body of the last article fetched
    std::string body;

    void queryPagesInfo(const std::string& apfrom);

  public:
    WikiSource(const std::string& host, unsigned short int port, const std::string& url_)
      : client(host, port),
        url(url_),
        xmlReader(xmlStream)
    {
      if (url.empty() || url[url.size()-1] != '/')
        url += '/';
    }

    virtual const zim::writer::Article* getNextArticle();
    virtual zim::Blob getData(const std::string& aid);
};

void WikiSource::queryPagesInfo(const std::string& apfrom)
{
  cxxtools::QueryParams q;
  q.add("action", "query")
   .add("list", "allpages")
   .add("format", "xml")
   .add("aplimit", "500");
  if (!apfrom.empty())
   q.add("apfrom", apfrom);

  std::string body = client.get(url + "api.php?" + q.getUrl());
  xmlStream.str(body);
  xmlStream.seekg(0);
  xmlReader.reset(xmlStream);
  xmlIterator = xmlReader.current();
}

const zim::writer::Article* WikiSource::getNextArticle()
{
  if (xmlIterator == xmlReader.end())
  {
    queryPagesInfo(std::string());
  }

  while ((++xmlIterator)->type() != cxxtools::xml::Node::EndDocument)
  {
    if (xmlIterator->type() == cxxtools::xml::Node::StartElement)
    {
      const cxxtools::xml::StartElement& startElement = dynamic_cast<const cxxtools::xml::StartElement&>(*xmlIterator);
      log_debug("element name=" << startElement.name().narrow());
      if (startElement.name() == L"p")
      {
        // page found - fill attributes and return article

        // get attribute pageid => article.aid
        // get attribute ns => article.ns
        // get attribute title => article.title
        article.aid   = cxxtools::Utf8Codec::encode(startElement.attribute(L"pageid"));
        article.ns    = cxxtools::Utf8Codec::encode(startElement.attribute(L"ns"))[0];
        article.title = cxxtools::Utf8Codec::encode(startElement.attribute(L"title"));

        log_debug("title=\"" << article.title << '"');

        return &article;
      }
      else if (startElement.name() == L"allpages")
      {
        // get attribute apfrom
        cxxtools::String s = startElement.attribute(L"apfrom");
        if (!s.empty())
        {
          cxxtools::String apfrom = startElement.attribute(L"apfrom");
          log_debug("apfrom=" << apfrom.narrow());
          queryPagesInfo(cxxtools::Utf8Codec::encode(apfrom));
        }
      }
    }
  }

  return 0;
}

zim::Blob WikiSource::getData(const std::string& aid)
{
  log_debug("fetch data for aid " << aid);
  cxxtools::QueryParams q;
  q.add("action", "render")
   .add("title", aid);
  body = client.get(url + "index.php?" + q.getUrl());
  return zim::Blob(body.data(), body.size());
}

int main(int argc, char* argv[])
{
  try
  {
    log_init();

    if (argc != 3)
    {
      std::cout << "usage: " << argv[0] << " [options] wiki-url output-filename\n"
                << "example: " << argv[0] << " http://openzim.org/ openzim-org\n"
                << "            generates openzim-org.zim with the content of the openzim.org wiki\n";
      return 1;
    }

    cxxtools::net::Uri uri = cxxtools::net::Uri(cxxtools::Arg<std::string>(argc, argv));

    std::cout << "host: " << uri.host() << "\n"
                 "port: " << uri.port() << "\n"
                 "url: " << uri.url() << std::endl;

    WikiSource source(uri.host(), uri.port(), uri.url());
    zim::writer::ZimCreator creator(argc, argv);

    std::string fname = cxxtools::Arg<std::string>(argc, argv);
    source.setFilename(fname);
    creator.create(fname, source);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}


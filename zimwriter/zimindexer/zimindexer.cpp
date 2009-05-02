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

#include "articleparser.h"
#include <cxxtools/loginit.h>
#include <cxxtools/arg.h>
#include <tntdb/connect.h>
#include <tntdb/statement.h>
#include <tntdb/transaction.h>
#include <set>

log_define("zim.indexer")

class Zimindexer : public zim::ArticleParseEventEx
{
    zim::MStream ostream;
    bool inTitle;
    zim::size_type aid

    typedef std::set<std::string> TrivialWordsType;
    TrivialWordsType trivialWords;

    void insertWord(const std::string& word, unsigned char weight, unsigned pos);

  public:
    Zimindexer();

    void process(unsigned aid_, const std::string& title, const tntdb::Blob& data)

    void onH1(const std::string& word, unsigned pos);
    void onH2(const std::string& word, unsigned pos);
    void onH3(const std::string& word, unsigned pos);
    void onB(const std::string& word, unsigned pos);
    void onP(const std::string& word, unsigned pos);
};

Zimindexer::Zimindexer()
{
}

void Zimindexer::insertWord(const std::string& word, unsigned char weight, unsigned pos)
{
  log_debug(word << '\t' << pos << '\t' << aid << '\t' << static_cast<unsinged>(weight));

  struct W
  {
    uint32_t aid;
    uint32_t pos;
    unsigned char weight;
  } w;
  w.aid = aid;
  w.pos = pos;
  w.weight = weight;
  ostream.write(word, reinterpret_cast<char*>(&w), sizeof(W));
}

void Zimindexer::process(zim::size_type aid_, const std::string& title, const tntdb::Blob& data)
{
  aid = aid_;

  zim::ArticleParser parser(*this);

  inTitle = true;
  parser.parse(title);
  parser.endparse();

  inTitle = false;
  parser.parse(data.data(), data.size());
  parser.endparse();
}

void Zimindexer::onH1(const std::string& word, unsigned pos)
{
  if (trivialWords.find(word) == trivialWords.end())
    insertWord(word, 0, pos);
}

void Zimindexer::onH2(const std::string& word, unsigned pos)
{
  if (trivialWords.find(word) == trivialWords.end())
    insertWord(word, 1, pos);
}

void Zimindexer::onH3(const std::string& word, unsigned pos)
{
  if (trivialWords.find(word) == trivialWords.end())
    insertWord(word, 2, pos);
}

void Zimindexer::onB(const std::string& word, unsigned pos)
{
  if (trivialWords.find(word) == trivialWords.end())
    insertWord(word, 2, pos);
}

void Zimindexer::onP(const std::string& word, unsigned pos)
{
  if (trivialWords.find(word) == trivialWords.end())
    insertWord(word, inTitle ? 0 : 3, word, pos, pos);
}

int main(int argc, char* argv[])
{
  try
  {
    log_init();

    cxxtools::Arg<std::string> dburl(argc, argv, "--db", "postgresql:dbname=zim");
    cxxtools::Arg<bool> printOnly(argc, argv, 'p');
    cxxtools::Arg<bool> all(argc, argv, 'a');
    tntdb::Connection conn = tntdb::connect(dburl);

    Zimindexer zimindexer(conn, printOnly);

    tntdb::Statement stmt;
    if (all)
    {
      log_info("select all articles");
      stmt = conn.prepare(
        "select aid, title, data"
        "  from article"
        " where mimetype = 0");
    }
    else
    {
      log_info("search for articles to process");
      stmt = conn.prepare(
        "select aid, title, data"
        "  from article"
        " where mimetype = 0"
        "   and aid not in (select distinct aid from words)"
        " order by namespace, title");
    }

    for (tntdb::Statement::const_iterator cur = stmt.begin(); cur != stmt.end(); ++cur)
    {
      unsigned aid = cur->getUnsigned(0);
      std::string title = cur->getString(1);
      tntdb::Blob data;
      cur->getBlob(2, data);
      log_info("process " << aid << ": " << title);
      zimindexer.process(aid, title, data);
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}

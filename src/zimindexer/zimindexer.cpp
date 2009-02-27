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

class Zenoindexer : public zim::ArticleParseEventEx
{
    tntdb::Connection conn;
    bool printOnly;
    unsigned aid;
    tntdb::Statement insWord;
    bool inTitle;

    typedef std::set<std::string> TrivialWordsType;
    TrivialWordsType trivialWords;

    void insertWord(unsigned weight, const std::string& word, unsigned pos);

  public:
    explicit Zenoindexer(tntdb::Connection& conn, bool printOnly);
    void process(unsigned aid, const std::string& title, const tntdb::Blob& data);
    void onH1(const std::string& word, unsigned pos);
    void onH2(const std::string& word, unsigned pos);
    void onH3(const std::string& word, unsigned pos);
    void onB(const std::string& word, unsigned pos);
    void onP(const std::string& word, unsigned pos);
};

Zenoindexer::Zenoindexer(tntdb::Connection& conn_, bool printOnly_)
  : conn(conn_),
    printOnly(printOnly_)
{
  if (printOnly)
  {
    std::cout << "COPY words (word, pos, aid, weight) FROM stdin;\n";
  }
  else
  {
    insWord = conn.prepare(
      "insert into words (word, pos, aid, weight)"
      " values (:word, :pos, :aid, :weight)");
  }

  tntdb::Statement selTrivialWords = conn.prepare(
    "select word from trivialwords");
  for (tntdb::Statement::const_iterator cur = selTrivialWords.begin(); cur != selTrivialWords.end(); ++cur)
    trivialWords.insert(cur->getString(0));
}

void Zenoindexer::insertWord(unsigned weight, const std::string& word, unsigned pos)
{
  log_debug(word << '\t' << pos << '\t' << aid << '\t' << weight);
  if (printOnly)
  {
    std::cout << word << '\t' << pos << '\t' << aid << '\t' << weight << '\n';
  }
  else
  {
    insWord.set("word", word)
           .set("pos", pos)
           .set("weight", weight)
           .execute();
  }
}

void Zenoindexer::process(unsigned aid_, const std::string& title, const tntdb::Blob& data)
{
  tntdb::Transaction trans(conn);

  aid = aid_;
  if (!printOnly)
    insWord.set("aid", aid);

  zim::ArticleParser parser(*this);

  inTitle = true;
  parser.parse(title);
  parser.endparse();

  inTitle = false;
  parser.parse(data.data(), data.size());
  parser.endparse();

  trans.commit();
}

void Zenoindexer::onH1(const std::string& word, unsigned pos)
{
  if (trivialWords.find(word) == trivialWords.end())
    insertWord(0, word, pos);
}

void Zenoindexer::onH2(const std::string& word, unsigned pos)
{
  if (trivialWords.find(word) == trivialWords.end())
    insertWord(1, word, pos);
}

void Zenoindexer::onH3(const std::string& word, unsigned pos)
{
  if (trivialWords.find(word) == trivialWords.end())
    insertWord(2, word, pos);
}

void Zenoindexer::onB(const std::string& word, unsigned pos)
{
  if (trivialWords.find(word) == trivialWords.end())
    insertWord(2, word, pos);
}

void Zenoindexer::onP(const std::string& word, unsigned pos)
{
  if (trivialWords.find(word) == trivialWords.end())
    insertWord(inTitle ? 0 : 3, word, pos);
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

    Zenoindexer zimindexer(conn, printOnly);

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

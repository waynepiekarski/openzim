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

log_define("zeno.indexer")

class Zenoindexer : public zeno::ArticleParseEventEx
{
    tntdb::Connection conn;
    tntdb::Statement insWord;
    bool inTitle;

    typedef std::set<std::string> TrivialWordsType;
    TrivialWordsType trivialWords;

    void insertWord(unsigned category, const std::string& word, unsigned pos);

  public:
    explicit Zenoindexer(tntdb::Connection& conn);
    void process(unsigned aid, const std::string& title, const tntdb::Blob& data);
    void onH1(const std::string& word, unsigned pos);
    void onH2(const std::string& word, unsigned pos);
    void onH3(const std::string& word, unsigned pos);
    void onP(const std::string& word, unsigned pos);
};

Zenoindexer::Zenoindexer(tntdb::Connection& conn_)
  : conn(conn_)
{
  insWord = conn.prepare(
    "insert into words (word, aid, pos, category)"
    " values (:word, :aid, :pos, :category)");

  tntdb::Statement selTrivialWords = conn.prepare(
    "select word from trivialwords");
  for (tntdb::Statement::const_iterator cur = selTrivialWords.begin(); cur != selTrivialWords.end(); ++cur)
    trivialWords.insert(cur->getString(0));
}

void Zenoindexer::insertWord(unsigned category, const std::string& word, unsigned pos)
{
  insWord.set("word", word)
         .set("pos", pos)
         .set("category", category)
         .execute();
}

void Zenoindexer::process(unsigned aid, const std::string& title, const tntdb::Blob& data)
{
  tntdb::Transaction trans(conn);

  insWord.set("aid", aid);

  zeno::ArticleParser parser(*this);

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

    cxxtools::Arg<std::string> dburl(argc, argv, "--db", "postgresql:dbname=zeno");
    tntdb::Connection conn = tntdb::connect(dburl);

    Zenoindexer zenoindexer(conn);

    tntdb::Statement stmt = conn.prepare(
      "select aid, title, data"
      "  from article"
      " where mimetype = 0"
      "   and aid not in (select distinct aid from words)");

    for (tntdb::Statement::const_iterator cur = stmt.begin(); cur != stmt.end(); ++cur)
    {
      unsigned aid = cur->getUnsigned(0);
      std::string title = cur->getString(1);
      tntdb::Blob data;
      cur->getBlob(2, data);
      std::cout << "process " << title << std::endl;
      zenoindexer.process(aid, title, data);
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}

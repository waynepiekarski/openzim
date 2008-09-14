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
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <cxxtools/loginit.h>

class EventPrinter : public zeno::ArticleParseEventEx
{
    typedef std::vector<unsigned> PositionsType;
    struct WordType
    {
      PositionsType h1;
      PositionsType h2;
      PositionsType h3;
      PositionsType p;
    };

    typedef std::map<std::string, WordType> WordsMapType;
    WordsMapType words;

  public:
    void onH1(const std::string& word, unsigned pos);
    void onH2(const std::string& word, unsigned pos);
    void onH3(const std::string& word, unsigned pos);
    void onP(const std::string& word, unsigned pos);
    void print() const;
    void print(const std::string& cat, const PositionsType& p) const;
};

void EventPrinter::onH1(const std::string& word, unsigned pos)
{
  words[word].h1.push_back(pos);
}

void EventPrinter::onH2(const std::string& word, unsigned pos)
{
  words[word].h2.push_back(pos);
}

void EventPrinter::onH3(const std::string& word, unsigned pos)
{
  words[word].h3.push_back(pos);
}

void EventPrinter::onP(const std::string& word, unsigned pos)
{
  words[word].p.push_back(pos);
}

void EventPrinter::print() const
{
  for (WordsMapType::const_iterator w = words.begin(); w != words.end(); ++w)
  {
    std::cout << '"' << w->first << "\": ";
    print("h1", w->second.h1);
    print("h2", w->second.h2);
    print("h3", w->second.h3);
    print("p", w->second.p);
    std::cout << '\n';
  }
}

void EventPrinter::print(const std::string& cat, const PositionsType& p) const
{
  if (!p.empty())
  {
    std::cout << '\t' << cat << ':';
    for (PositionsType::const_iterator it = p.begin(); it != p.end(); ++it)
      std::cout << ' ' << *it;
  }
}

void process(std::istream& in)
{
  EventPrinter ev;
  zeno::ArticleParser parser(ev);
  char ch;
  while (in.get(ch))
    parser.parse(ch);
  parser.endparse();
  ev.print();
}

int main(int argc, char* argv[])
{
  try
  {
    log_init();

    if (argc <= 1)
      process(std::cin);
    else
    {
      for (int a = 1; a < argc; ++a)
      {
        std::ifstream in(argv[a]);
        process(in);
      }
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}

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

#include "zim/writer/zimindexer.h"
#include <set>
#include <cxxtools/log.h>

log_define("zim.writer.indexer")

namespace zim
{
  namespace writer
  {
    const unsigned Zimindexer::Wordentry::size;

    void Zimindexer::insertWord(const std::string& word, unsigned char weight, unsigned pos)
    {
      log_debug(word << '\t' << pos << '\t' << aid << '\t' << static_cast<unsigned>(weight));

      Wordentry w;
      w.aid = aid;
      w.pos = pos;
      w.weight = weight;
      ostream.write(word, reinterpret_cast<char*>(&w), Wordentry::size);
    }

    void Zimindexer::process(zim::size_type aid_, const std::string& title, const char* data, unsigned size)
    {
      aid = aid_;

      ArticleParser parser(*this);

      inTitle = true;
      parser.parse(title);
      parser.endparse();

      inTitle = false;
      parser.parse(data, size);
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
        insertWord(word, inTitle ? 0 : 3, pos);
    }

  }
}

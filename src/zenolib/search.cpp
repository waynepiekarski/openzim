/*
 * Copyright (C) 2007 Tommi Maekitalo
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

#include <zeno/search.h>
#include <sstream>
#include <cxxtools/log.h>
#include <map>
#include <math.h>
#include <cctype>

log_define("zeno.search")

namespace zeno
{
  namespace
  {
    class PriorityGt : public std::binary_function<bool, SearchResult, SearchResult>
    {
      public:
        bool operator() (const SearchResult& s1, const SearchResult& s2) const
        {
          return s1.getPriority() > s2.getPriority()
              || s1.getPriority() == s2.getPriority()
               && s1.getArticle().getTitle() > s2.getArticle().getTitle();
        }
    };
  }

  double SearchResult::getPriority() const
  {
    if (!wordList.empty() && priority == 0.0)
    {
      priority = 1.0;

      // weight occurencies of words
      for (WordListType::const_iterator itw = wordList.begin(); itw != wordList.end(); ++itw)
        priority *= 1.0 + log(itw->second * zeno::Search::getWeightOcc()) + zeno::Search::getWeightOccOff();

      // weight distinct words
      priority += zeno::Search::getWeightDistinctWords() * wordList.size();

      // weight distance between different words
      PosListType::const_iterator itp = posList.begin();
      std::string word = itp->second;
      uint32_t pos = itp->first + word.size();
      for (++itp; itp != posList.end(); ++itp)
      {
        if (word != itp->second)
        {
          uint32_t dist = itp->first > pos ? (itp->first - pos)
                        : itp->first < pos ? (pos - itp->first)
                        : 1;
          priority += zeno::Search::getWeightDist() / dist;
        }
        word = itp->second;
        pos = itp->first + word.size();
      }

      // weight position of words in the document
      for (itp = posList.begin(); itp != posList.end(); ++itp)
        priority += zeno::Search::getWeightPos() * itp->first / article.getDataLen();

      log_debug("priority of article \"" << article.getTitle() << "\", " << wordList.size() << " words: " << priority);
    }

    return priority;
  }

  double Search::weightOcc = 10.0;
  double Search::weightOccOff = 1.0;
  double Search::weightDist = 10;
  double Search::weightPos = 2;
  double Search::weightDistinctWords = 50;

  Search::Results Search::search(const std::string& expr)
  {
    std::istringstream ssearch(expr);
    std::string token;
    typedef std::map<uint32_t, SearchResult> IndexType;
    IndexType index;
    while (ssearch >> token)
    {
      for (std::string::iterator it = token.begin(); it != token.end(); ++it)
        *it = std::tolower(*it);

      log_debug("search for token \"" << token << '"');

      zeno::Article indexarticle = indexfile.getArticle("X/" + token);
      std::string data = indexarticle.getData();
      log_debug(data.size() / 8 << " articles found; collect statistics");
      for (unsigned off = 0; off + 4 <= data.size(); off += 8)
      {
        uint32_t articleIdx = fromLittleEndian(reinterpret_cast<const uint32_t*>(data.data() + off));
        uint32_t position = fromLittleEndian(reinterpret_cast<const uint32_t*>(data.data() + off + 4));

        IndexType::iterator it = index.insert(
          IndexType::value_type(articleIdx,
            SearchResult(articlefile.getArticle(articleIdx)))).first;

        it->second.foundWord(token, position);
      }
    }

    log_debug("copy/filter " << index.size() << " articles");
    Results searchResult;
    for (IndexType::const_iterator it = index.begin(); it != index.end(); ++it)
    {
      if (it->second.getCountPositions() > 1)
        searchResult.push_back(it->second);
    }

    if (searchResult.empty())
    {
      for (IndexType::const_iterator it = index.begin(); it != index.end(); ++it)
        searchResult.push_back(it->second);
    }

    log_debug("sort " << searchResult.size() << " articles");
    std::sort(searchResult.begin(), searchResult.end(), PriorityGt());

    log_debug("return articles");
    return searchResult;
  }
}

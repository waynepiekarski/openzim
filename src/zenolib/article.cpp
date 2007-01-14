/*
 * Copyright (C) 2006 Tommi Maekitalo
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

#include <zeno/article.h>
#include <cxxtools/log.h>
#include <tnt/inflatestream.h>
#include <sstream>

log_define("zeno.article")

namespace zeno
{
  const std::string& Article::getRawData() const
  {
    if (!dataRead)
    {
      data = const_cast<File&>(file).readData(getDataOffset(), getDataLen());
      dataRead = true;
    }
    return data;
  }

  const std::string& Article::getData() const
  {
    if (uncompressedData.empty() && !getRawData().empty())
    {
      if (isCompressionZip())
      {
        std::ostringstream u;
        tnt::InflateStream is(u);
        is << getRawData() << std::flush;
        uncompressedData = u.str();
      }
      else
      {
        uncompressedData = getRawData();
      }
    }
    return uncompressedData;
  }

  unsigned Article::getCountSubarticles() const
  {
    if (countArticles == 0)
    {
      size_type i = idx + 1;
      countArticles = 1;
      while (!const_cast<File&>(file).getArticle(i).isMainArticle())
      {
        ++i;
        ++countArticles;
      }
    }
    return countArticles - 1;
  }

  Article::const_iterator Article::end() const
  {
    return const_iterator(const_cast<Article*>(this), idx + 1 + const_cast<Article*>(this)->getCountSubarticles());
  }
}

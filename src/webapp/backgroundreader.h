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

#include <map>
#include <set>
#include <cxxtools/thread.h>
#include <zeno/files.h>
#include <zeno/article.h>

namespace zeno
{
  class Backgroundreader : public cxxtools::AttachedThread
  {
      zeno::Files files;
      zeno::Article lastArticle;
      zeno::Article currentArticle;
      bool stopRunning;

      typedef std::map<std::pair<char, QUnicodeString>, zeno::Article> CachedArticlesType;
      CachedArticlesType cachedArticles;
      CachedArticlesType prevCachedArticles;

      cxxtools::Mutex mutex;
      cxxtools::Condition newArticle;

      typedef std::set<std::pair<char, QUnicodeString> > UrlsType;
      void readUrls(UrlsType& urls, zeno::File file);
      void readImages();
      void readLinks();

      zeno::File getFile(char ns);

    public:
      explicit Backgroundreader(zeno::Files files);
      ~Backgroundreader();

      void run();

      zeno::Article getArticle(char ch, const QUnicodeString& path);
  };
};

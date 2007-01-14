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

#ifndef ZENO_FILEIMPL_H
#define ZENO_FILEIMPL_H

#include <tntdb/refcounted.h>
#include <cxxtools/thread.h>
#include <fstream>
#include <string>
#include <vector>
#include <zeno/zeno.h>

namespace zeno
{
  class Dirent;
  class Article;

  class FileImpl : public tntdb::RefCounted
  {
      std::ifstream zenoFile;
      cxxtools::Mutex mutex;

      typedef std::vector<offset_type> IndexOffsetsType;
      IndexOffsetsType indexOffsets;

      std::string readDataNolock(size_type count);
      std::string readDataNolock(offset_type off, size_type count);
      Dirent readDirentNolock();
      Dirent readDirentNolock(offset_type off);

    public:
      FileImpl(const char* fname);

      Article getArticle(const std::string& url);
      Article getArticle(size_type idx);
      std::pair<bool, size_type> findArticle(const std::string& url);
      Dirent getDirent(size_type idx);
      size_type getCountArticles() const  { return indexOffsets.size(); }

      std::string readData(offset_type off, size_type count);
  };

}

#endif // ZENO_FILEIMPL_H


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

#ifndef ZENO_FILE_H
#define ZENO_FILE_H

#include <string>
#include <iterator>
#include <zeno/zeno.h>
#include <zeno/smartptr.h>
#include <zeno/fileimpl.h>

namespace zeno
{
  class Article;
  class Dirent;

  class File
  {
      zeno::SmartPtr<FileImpl> impl;

    public:
      File()
        { }
      explicit File(const std::string& fname);
      explicit File(FileImpl* impl);

      std::string readData(offset_type off, size_type count);

      const std::string& getFilename() const   { return impl->getFilename(); }

      Article getArticle(const std::string& url);
      Article getArticle(size_type idx);
      Dirent getDirent(size_type idx);
      size_type getCountArticles() const;

      class const_iterator;

      const_iterator begin();
      const_iterator end();
      const_iterator find(const std::string& url);

      void cacheData(offset_type off, size_type count);
  };

}

#endif // ZENO_FILE_H


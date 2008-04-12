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

#ifndef ZENO_ARTICLE_H
#define ZENO_ARTICLE_H

#include <zeno/articlebase.h>
#include <zeno/file.h>

namespace zeno
{
  class Article : public ArticleBase
  {
    private:
      File file;
      size_type idx;
      bool dataRead;

    public:
      Article() : dataRead(true) { }
      Article(size_type idx_, const Dirent& dirent_, const File& file_,
              const std::string& data_)
        : ArticleBase(dirent_, data_),
          file(file_),
          idx(idx_),
          dataRead(true)
          { }
      Article(size_type idx_, const Dirent& dirent_, const File& file_)
        : ArticleBase(dirent_),
          file(file_),
          idx(idx_),
          dataRead(false)
          { }

      const File& getFile() const             { return file; }
      File&       getFile()                   { return file; }
      void        setFile(const File& f)      { file = f; }

      size_type   getIndex() const            { return idx; }
      void        setIndex(size_type i)       { idx = i; }

      QUnicodeString getUrl() const           { return QUnicodeString(std::string(1, getNamespace()) + '/' + getDirent().getTitle()); }

      const std::string& getRawData() const;
  };

}

#endif // ZENO_ARTICLE_H


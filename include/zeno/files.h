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

#ifndef ZENO_FILES_H
#define ZENO_FILES_H

#include <zeno/file.h>
#include <map>
#include <tnt/stringlessignorecase.h>

namespace zeno
{
  class Files
  {
      typedef std::map<std::string, File, tnt::StringLessIgnoreCase<std::string> > FilesType;
      FilesType files;

    public:
      typedef FilesType::iterator iterator;
      typedef FilesType::const_iterator const_iterator;

      Files() { }
      explicit Files(const std::string& dir)
        { addFiles(dir); }

      void addFile(const std::string& fname)  { files[fname] = File(fname); }
      void addFile(const std::string& fname, const File& file )  { files[fname] = file; }
      void addFiles(const std::string& dir);

      Files getFiles(char ns);
      File getFirstFile(char ns);

      Article getArticle(char ns, const QUnicodeString& url);
      Article getArticle(const std::string& file, char ns, const QUnicodeString& url);
      Article getBestArticle(const Article& article);

      iterator begin()                 { return files.begin(); }
      iterator end()                   { return files.end(); }
      const_iterator begin() const     { return files.begin(); }
      const_iterator end() const       { return files.end(); }
      bool empty()                     { return files.empty(); }
  };
}

#endif // ZENO_FILES_H

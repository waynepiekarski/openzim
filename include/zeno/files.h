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
#include <vector>

namespace zeno
{
  class Files
  {
      typedef std::vector<File> FilesType;
      FilesType files;

    public:
      typedef FilesType::iterator iterator;
      typedef FilesType::const_iterator const_iterator;

      Files() { }
      explicit Files(const std::string& dir)
        { addFiles(dir); }

      void addFile(const std::string& fname)  { files.push_back(File(fname)); }
      void addFile(const File& file)          { files.push_back(file); }
      void addFiles(const std::string& dir);

      Files getFiles(char ns);
      File getFirstFile(char ns);

      Article getArticle(char ns, const QUnicodeString& url);

      iterator begin()                 { return files.begin(); }
      iterator end()                   { return files.end(); }
      const_iterator begin() const     { return files.begin(); }
      const_iterator end() const       { return files.end(); }
      bool empty()                     { return files.empty(); }
  };
}

#endif // ZENO_FILES_H

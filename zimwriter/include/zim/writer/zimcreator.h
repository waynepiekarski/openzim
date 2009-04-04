/*
 * Copyright (C) 2009 Tommi Maekitalo
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

#ifndef ZIM_WRITER_ZIMCREATOR_H
#define ZIM_WRITER_ZIMCREATOR_H

#include <zim/writer/articlesource.h>
#include <zim/writer/dirent.h>
#include <vector>

namespace zim
{
  namespace writer
  {
    class ZimCreator
    {
        ArticleSource& src;

        unsigned minChunkSize;

        Fileheader header;

        typedef std::vector<Dirent> DirentsType;
        DirentsType dirents;

        typedef std::vector<offset_type> OffsetsType;
        OffsetsType clusterOffsets;

        void createDirents();
        void createClusters(const std::string& tmpfname);
        void fillHeader();
        void write(const std::string& fname, const std::string& tmpfname);

        size_type clusterCount() const        { return clusterOffsets.size(); }
        size_type articleCount() const        { return dirents.size(); }
        offset_type indexPtrSize() const      { return articleCount() * sizeof(offset_type); }
        offset_type indexPtrPos() const       { return Fileheader::size; }
        offset_type indexSize() const;
        offset_type indexPos() const          { return indexPtrPos() + indexPtrSize(); }
        offset_type clusterPtrSize() const    { return clusterCount() * sizeof(offset_type); }
        offset_type clusterPtrPos() const     { return indexPos() + indexSize(); }

      public:
        ZimCreator(int& argc, char* argv[], ArticleSource& src_);

        void create(const std::string& fname);
    };

  }

}

#endif // ZIM_WRITER_ZIMCREATOR_H

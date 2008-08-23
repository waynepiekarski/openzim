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

#ifndef ZENO_COMPRESSOR_H
#define ZENO_COMPRESSOR_H

#include <string>
#include <vector>
#include <map>
#include <deque>
#include <cxxtools/mutex.h>
#include <zeno/dirent.h>
#include <tntdb/connection.h>
#include <tntdb/statement.h>

namespace zeno
{
  struct CompressJob
  {
    struct Article
    {
      unsigned aid;
      unsigned direntlen;
      unsigned dataoffset;
      unsigned datasize;
      Article() { }
      Article(unsigned aid_, unsigned direntlen_, unsigned dataoffset_, unsigned datasize_)
        : aid(aid_),
          direntlen(direntlen_),
          dataoffset(dataoffset_),
          datasize(datasize_)
        { }
    };

    typedef std::vector<Article> ArticlesType;
    ArticlesType articles;
    std::string data;
    zeno::Dirent::CompressionType compression;
  };

  class ZenoCompressorThread;
  class ZenoCompressorContext;

  class ZenoCompressor
  {
      std::vector<ZenoCompressorThread*> compressorThreads;
      ZenoCompressorContext* context;

    public:
      ZenoCompressor(const std::string& dburl, unsigned zid, unsigned numThreads);
      ~ZenoCompressor();
      void put(const CompressJob& job);
  };

}

#endif // ZENO_COMPRESSOR_H

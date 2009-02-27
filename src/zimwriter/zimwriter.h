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

#ifndef ZIMWRITER_H
#define ZIMWRITER_H

#include <set>
#include <iosfwd>
#include <tntdb/connection.h>
#include <zim/qunicode.h>
#include <zim/fileheader.h>
#include <zim/dirent.h>
#include <cxxtools/mutex.h>

class Zenowriter
{
    tntdb::Connection conn;
    std::string basename;
    unsigned zid;
    unsigned commitRate;
    unsigned minChunkSize;
    zim::size_type countArticles;
    std::string outdir;
    std::string dburl;
    zim::Dirent::CompressionType compression;
    unsigned numThreads;
    bool createIndex;

    typedef std::set<unsigned> ArticleIdsType;
    ArticleIdsType articleIds;

    tntdb::Connection& getConnection();

    unsigned indexLen;

    zim::Fileheader header;

    void writeHeader(std::ostream& ofile);
    void writeIndexPtr(std::ostream& ofile);
    void writeDirectory(std::ostream& ofile);
    void writeData(std::ostream& ofile);

    static bool mimeDoCompress(zim::Dirent::MimeType t)
    { return t == zim::Dirent::zimMimeTextHtml
          || t == zim::Dirent::zimMimeTextPlain
          || t == zim::Dirent::zimMimeTextCss
          || t == zim::Dirent::zimMimeIndex
          || t == zim::Dirent::zimMimeApplicationJavaScript; }

  public:
    explicit Zenowriter(const char* basename);

    void init(bool withIndexarticle);
    void cleanup();
    void prepareFile();
    void prepareWordIndex();
    void createWordIndex();
    void prepareSort();
    void outputFile();

    unsigned getCommitRate() const  { return commitRate; }
    void setCommitRate(unsigned c)  { commitRate = c; }

    unsigned getMinChunkSize() const  { return minChunkSize; }
    void setMinChunkSize(unsigned s)  { minChunkSize = s; }

    const std::string& getOutdir() const  { return outdir; }
    void setOutdir(const std::string& o)  { outdir = o; }

    const std::string& getDburl() const  { return dburl; }
    void setDburl(const std::string& d)  { dburl = d; }

    void setCompression(zim::Dirent::CompressionType c)  { compression = c; }

    unsigned getNumThreads() const     { return numThreads; }
    void setNumThreads(unsigned n)     { numThreads = n; }

    bool getCreateIndex() const     { return createIndex; }
    void setCreateIndex(bool n)     { createIndex = n; }

};

class ZenoIndexEntry
{
    friend std::ostream& operator<< (std::ostream& out, const ZenoIndexEntry& entry);
    zim::size_type index;
    zim::size_type pos;

  public:
    ZenoIndexEntry() {}
    ZenoIndexEntry(zim::size_type index_, zim::size_type pos_)
    {
      setIndex(index_);
      setPos(pos_);
    }

    zim::size_type getIndex() const        { return index; }
    void        setIndex(zim::size_type o) { index = o; }

    zim::size_type getPos() const        { return pos; }
    void        setPos(zim::size_type o) { pos = o; }
};

inline std::ostream& operator<< (std::ostream& out, const ZenoIndexEntry& entry)
{
  zim::size_type data[2];
  data[0] = fromLittleEndian(&entry.index);
  data[1] = fromLittleEndian(&entry.pos);
  out.write(reinterpret_cast<const char*>(&data[0]), 2 * sizeof(zim::size_type));
  return out;
}

#endif

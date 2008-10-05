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

#ifndef ZENOWRITER2_H
#define ZENOWRITER2_H

#include <map>
#include <iosfwd>
#include <tntdb/connection.h>
#include <zeno/qunicode.h>
#include <zeno/fileheader.h>
#include <zeno/dirent.h>

class Zenowriter
{
    tntdb::Connection conn;
    std::string basename;
    unsigned zid;
    unsigned commitRate;
    unsigned minChunkSize;
    zeno::size_type countArticles;
    std::string outdir;
    std::string dburl;
    zeno::Dirent::CompressionType compression;
    unsigned numThreads;
    bool createIndex;

    tntdb::Connection& getConnection();

    unsigned indexLen;

    zeno::Fileheader header;

    void writeHeader(std::ostream& ofile);
    void writeIndexPtr(std::ostream& ofile);
    void writeDirectory(std::ostream& ofile);
    void writeData(std::ostream& ofile);

    static bool mimeDoCompress(zeno::Dirent::MimeType t)
    { return t == zeno::Dirent::zenoMimeTextHtml
          || t == zeno::Dirent::zenoMimeTextPlain
          || t == zeno::Dirent::zenoMimeTextCss
          || t == zeno::Dirent::zenoMimeIndex
          || t == zeno::Dirent::zenoMimeApplicationJavaScript; }

  public:
    explicit Zenowriter(const char* basename);

    void init();
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

    void setCompression(zeno::Dirent::CompressionType c)  { compression = c; }

    unsigned getNumThreads() const     { return numThreads; }
    void setNumThreads(unsigned n)     { numThreads = n; }

    bool getCreateIndex() const     { return createIndex; }
    void setCreateIndex(bool n)     { createIndex = n; }

};

class ZenoIndexEntry
{
    friend std::ostream& operator<< (std::ostream& out, const ZenoIndexEntry& entry);
    char data[2 * sizeof(zeno::size_type)];

  public:
    ZenoIndexEntry() {}
    ZenoIndexEntry(zeno::size_type index_, zeno::size_type pos_)
    {
      setIndex(index_);
      setPos(pos_);
    }

    zeno::size_type getIndex() const        { return fromLittleEndian<zeno::size_type>(data); }
    void        setIndex(zeno::size_type o) { *reinterpret_cast<zeno::size_type*>(&data[0]) = fromLittleEndian<zeno::size_type>(&o); }

    zeno::size_type getPos() const        { return fromLittleEndian<zeno::size_type>(&data[sizeof(zeno::size_type)]); }
    void        setPos(zeno::size_type o) { *reinterpret_cast<zeno::size_type*>(&data[sizeof(zeno::size_type)]) = fromLittleEndian<zeno::size_type>(&o); }
};

inline std::ostream& operator<< (std::ostream& out, const ZenoIndexEntry& entry)
{
  out.write(entry.data, 2 * sizeof(zeno::size_type));
  return out;
}

#endif

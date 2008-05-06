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

#ifndef ZENOWRITER_H
#define ZENOWRITER_H

#include <map>
#include <iosfwd>
#include <tntdb/connection.h>
#include <zeno/qunicode.h>
#include <zeno/fileheader.h>

class Zenowriter
{
    tntdb::Connection conn;
    std::string basename;
    unsigned zid;
    unsigned commitRate;
    std::string outdir;

    struct ArticleInfo
    {
      unsigned aid;
      unsigned titlesize;
      unsigned datasize;
      unsigned redirectsize;
    };

    typedef std::map<zeno::QUnicodeString, ArticleInfo> ArticleMapType;
    ArticleMapType articleMap;
    unsigned indexLen;
    unsigned indexPtrPos;
    unsigned indexPos;
    zeno::size_type dataPos;
    unsigned dataLen;

    zeno::Fileheader header;

    void writeHeader(std::ofstream& ofile);
    void writeIndexPtr(std::ofstream& ofile);
    void writeDirectory(std::ofstream& ofile);
    void writeData(std::ofstream& ofile);

  public:
    explicit Zenowriter(const char* basename);

    void readArticles();
    void dump();
    void prepareFile();
    void outputFile();

    unsigned getCommitRate() const  { return commitRate; }
    void setCommitRate(unsigned c)  { commitRate = c; }

    const std::string& getOutdir() const  { return outdir; }
    void setOutdir(const std::string& o)  { outdir = o; }

};

#endif

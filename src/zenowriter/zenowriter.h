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

class Zenowriter
{
    tntdb::Connection conn;
    std::string basename;
    unsigned zid;
    unsigned commitRate;
    unsigned minChunkSize;
    std::string outdir;
    std::string dburl;
    bool compressZlib;

    tntdb::Connection& getConnection();

    unsigned indexLen;

    zeno::Fileheader header;

    void writeHeader(std::ostream& ofile);
    void writeIndexPtr(std::ostream& ofile);
    void writeDirectory(std::ostream& ofile);
    void writeData(std::ostream& ofile);
    unsigned insertDataChunk(const std::string& data, unsigned did, tntdb::Statement& insData);

  public:
    explicit Zenowriter(const char* basename);

    void cleanup();
    void prepareFile();
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

    bool isCompressZlib() const           { return compressZlib; }
    void setCompressZlib(bool sw = true)  { compressZlib = sw; }
};

#endif

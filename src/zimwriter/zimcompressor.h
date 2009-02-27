/*
 * Copyright (C) 2008 Tommi Maekitalo
 *
 * This program is free software; you can redistribute it and/or
 *
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

#ifndef ZIMCOMPRESSOR_H
#define ZIMCOMPRESSOR_H

#include <string>
#include <vector>
#include <map>
#include <cxxtools/mutex.h>
#include <cxxtools/condition.h>
#include <tntdb/connection.h>
#include <tntdb/statement.h>
#include <compressor.h>

class ZenoCompressor;

struct ZenoCompressJob : public CompressJob
{
  struct Article
  {
    int aid;
    unsigned direntlen;
    unsigned dataoffset;
    unsigned datasize;
    Article() { }
    Article(int aid_, unsigned direntlen_, unsigned dataoffset_, unsigned datasize_)
      : aid(aid_),
        direntlen(direntlen_),
        dataoffset(dataoffset_),
        datasize(datasize_)
      { }
  };

  ZenoCompressor& compressor;

  ZenoCompressJob(ZenoCompressor& compressor_)
    : compressor(compressor_)
    { }

  typedef std::vector<Article> ArticlesType;
  ArticlesType articles;

  unsigned did;

  typedef cxxtools::SmartPtr<ZenoCompressJob> Ptr;

  // override from CompressJob:
  void ready();
};

class ZenoCompressor
{
    Compressor compressor;

    tntdb::Connection conn;
    tntdb::Statement insData;
    tntdb::Statement updArticle;
    tntdb::Statement updIndexarticle;
    unsigned zid;

    typedef std::map<unsigned, ZenoCompressJob::Ptr> ReadyJobsMap;  // map did=>job
    cxxtools::Mutex readyJobsMutex;
    ReadyJobsMap readyJobs;  // map did to job

    zim::offset_type datapos;
    unsigned processDid;  // next data-id to process after compression

    friend class ZenoCompressJob;
    // called by compress thread to indicate that compression is done
    void ready(ZenoCompressJob* job);
    void processReadyJob(ZenoCompressJob::Ptr job);

  public:
    ZenoCompressor(tntdb::Connection conn, unsigned zid, unsigned numThreads);
    ~ZenoCompressor();

    // called by main thread to add a compress job
    void compress(ZenoCompressJob::Ptr job);

    // called by main thread to write compressed data to database
    void processReadyJobs(bool flush = false);

    const std::string& getErrorMessage() const  { return compressor.getErrorMessage(); }
};

#endif // ZIMCOMPRESSOR_H

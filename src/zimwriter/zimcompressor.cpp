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

#include "zimcompressor.h"
#include <cxxtools/mutex.h>
#include <cxxtools/condition.h>
#include <cxxtools/fork.h>
#include <cxxtools/pipestream.h>
#include <cxxtools/log.h>
#include <tntdb/blob.h>
#include <zim/deflatestream.h>
#include <zim/bzip2stream.h>
#include <unistd.h>

log_define("zim.compressor.zim")

void ZenoCompressJob::ready()
{
  compressor.ready(this);
}

ZenoCompressor::ZenoCompressor(tntdb::Connection conn, unsigned zid, unsigned numThreads)
  : compressor(numThreads, numThreads * 2),
    datapos(0),
    processDid(0)
{
  insData = conn.prepare(
    "insert into zimdata"
    "  (zid, did, data)"
    " values (:zid, :did, :data)");

  updArticle = conn.prepare(
    "update zimarticles"
    "   set direntlen  = :direntlen,"
    "       dataoffset = :dataoffset,"
    "       datasize   = :datasize,"
    "       datapos    = :datapos,"
    "       did        = :did"
    " where zid = :zid"
    "   and aid = :aid");

  updIndexarticle = conn.prepare(
    "update indexarticle"
    "   set direntlen  = :direntlen,"
    "       dataoffset = :dataoffset,"
    "       datasize   = :datasize,"
    "       datapos    = :datapos,"
    "       did        = :did"
    " where zid = :zid"
    "   and xid = :xid");

  insData.set("zid", zid);
  updArticle.set("zid", zid);
  updIndexarticle.set("zid", zid);
}

ZenoCompressor::~ZenoCompressor()
{
  unsigned n = readyJobs.size();
  if (n)
    log_warn(n << " ready jobs left");
}

void ZenoCompressor::compress(ZenoCompressJob::Ptr job)
{
  log_debug("compress did=" << job->did);
  compressor.compress(job.getPointer());
}

void ZenoCompressor::ready(ZenoCompressJob* job)
{
  log_debug("job did=" << job->did << " ready");
  cxxtools::MutexLock lock(readyJobsMutex);
  readyJobs[job->did] = job;
}

void ZenoCompressor::processReadyJobs(bool flush)
{
  log_trace("process ready jobs");

  if (flush)
    compressor.waitReady();

  cxxtools::MutexLock lock(readyJobsMutex);

  ReadyJobsMap::iterator it;
  while ( it = readyJobs.begin(),
    (it != readyJobs.end() && it->first == processDid) )
  {
    processReadyJob(it->second);
    readyJobs.erase(it);
    ++processDid;
  }
}

void ZenoCompressor::processReadyJob(ZenoCompressJob::Ptr j)
{
  log_debug("insert datachunk " << j->did << " with " << j->zdata.size() << " bytes");
  tntdb::Blob bdata(j->zdata.data(), j->zdata.size());
  insData.set("did", j->did)
         .set("data", bdata)
         .execute();

  log_debug("update " << j->articles.size() << " articles of did " << j->did);
  for (ZenoCompressJob::ArticlesType::const_iterator it = j->articles.begin();
    it != j->articles.end(); ++it)
  {
    log_debug("update article " << it->aid << " direntlen=" << it->direntlen << " dataoffset=" << it->dataoffset
        << " datasize=" << it->datasize << " did=" << j->did);

    if (it->aid >= 0)
      updArticle.setUnsigned64("datapos", datapos)
                .set("aid", it->aid)
                .set("direntlen", it->direntlen)
                .set("dataoffset", it->dataoffset)
                .set("datasize", it->datasize)
                .set("did", j->did)
                .execute();
    else
      updIndexarticle.setUnsigned64("datapos", datapos)
                     .set("xid", -(1+it->aid))
                     .set("direntlen", it->direntlen)
                     .set("dataoffset", it->dataoffset)
                     .set("datasize", it->datasize)
                     .set("did", j->did)
                     .execute();
  }

  datapos += j->zdata.size();

}

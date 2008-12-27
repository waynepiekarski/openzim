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

#include <compressor.h>
#include <compressorimpl.h>
#include <cxxtools/log.h>
#include <cxxtools/fork.h>
#include <cxxtools/pipestream.h>
#include <zeno/deflatestream.h>
#include <zeno/bzip2stream.h>
#include <sstream>

log_define("zeno.compressor")

Compressor::Compressor(unsigned threads, unsigned maxqueue)
  : impl(new CompressorImpl(threads, maxqueue))
  { }

Compressor::~Compressor()
{
  impl->doStop();
  delete impl;
}
 
void Compressor::compress(CompressJob::Ptr job)
{
  impl->compress(job);
}

void Compressor::waitReady()
{
  impl->waitReady();
}

void Compressor::doStop()
{
  impl->doStop();
}

const std::string& Compressor::getErrorMessage() const
{
  return impl->getErrorMessage();
}


CompressorImpl::CompressorImpl(unsigned threads, unsigned maxqueue_)
  : maxqueue(maxqueue_),
    stop(STOP_NO)
{
  while (compressorThreads.size() < threads)
  {
    cxxtools::AttachedThread* th = new cxxtools::AttachedThread(cxxtools::callable(*this, &CompressorImpl::run));
    th->start();
    compressorThreads.push_back(th);
  }
}

void CompressorImpl::run()
{
  log_trace("compressor run");

  while (true)
  {
    cxxtools::MutexLock lock(mutex);
    while (stop == STOP_NO && queue.empty())
      notEmpty.wait(lock);

    if (stop != STOP_NO
      && (stop == STOP_NOW || queue.empty()))
    {
      log_debug("stopping with stop=" << stop << " queue.empty=" << queue.empty());
      break;
    }

    CompressJob::Ptr job = queue.front();
    log_debug("got job");

    queue.pop_front();
    notFull.signal();
    lock.unlock();

    try
    {
      doJob(*job);
    }
    catch (const std::exception& e)
    {
      std::ostringstream msg;
      msg << "error in compress job: " << e.what();
      log_error(msg.str());
      if (!errorMessage.empty())
        errorMessage += '\n';
      errorMessage += msg.str();
      doStop();
    }

    log_debug("job ready");
  }

  // wake another thread
  notEmpty.signal();
}

void CompressorImpl::doJob(CompressJob& job)
{
  if (job.compression == zeno::Dirent::zenocompZip)
  {
    log_trace("zlib compress " << job.data.size() << " bytes");

    std::ostringstream u;
    zeno::DeflateStream ds(u);
    ds << job.data << std::flush;
    ds.end();
    job.zdata = u.str();
    log_debug("after zlib compression " << job.zdata.size() << " bytes");
  }
  else if (job.compression == zeno::Dirent::zenocompBzip2)
  {
    log_trace("bzip2 compress " << job.data.size() << " bytes");

    std::ostringstream u;
    zeno::Bzip2Stream ds(u);
    ds << job.data << std::flush;
    ds.end();
    job.zdata = u.str();
    log_debug("after bzip2 compression " << job.zdata.size() << " bytes");
  }
  else if (job.compression == zeno::Dirent::zenocompLzma)
  {
    log_trace("lzma compress " << job.data.size() << " bytes");
    cxxtools::Pipestream compressedDataPipe;
    cxxtools::Fork senderProcess;
    if (senderProcess.parent())
    {
      // receive the compressed data from compressedDataPipe
      compressedDataPipe.closeWriteFd();
      std::ostringstream u;
      log_debug("read compressed data from pipe");
      u << compressedDataPipe.rdbuf();
      int ret = senderProcess.wait();
      if (WEXITSTATUS(ret) != 0)
        throw std::runtime_error("error in lzma compressor");
      job.zdata = u.str();
      log_debug("after lzma compression " << job.zdata.size() << " bytes");
    }
    else
    {
      // another fork for the lzma process
      compressedDataPipe.closeReadFd();

      cxxtools::Pipestream uncompressedDataPipe;
      cxxtools::Fork lzmaProcess;
      if (lzmaProcess.parent())
      {
        // send the data to the lzma process
        uncompressedDataPipe.closeReadFd();
        log_debug("sending uncompressed data to lzma process");
        uncompressedDataPipe << job.data << std::flush;
        uncompressedDataPipe.closeWriteFd();
        log_debug("wait lzma process to end");
        int ret = lzmaProcess.wait();
        log_debug("lzma process ended with return code " << WEXITSTATUS(ret) << " (" << ret << ')');
        exit ((WEXITSTATUS(ret) != 0 || uncompressedDataPipe.fail()) ? -1 : 0);
      }
      else
      {
        // redirect stdin and out and exec lzma

        uncompressedDataPipe.closeWriteFd();

        // redirect stdin to uncompressedDataPipe
        close(STDIN_FILENO);
        dup(uncompressedDataPipe.getReadFd());

        // redirect stdout to compressedDataPipe
        close(STDOUT_FILENO);
        dup(compressedDataPipe.getWriteFd());

        // execute
        const char* argv[] = { "lzma", "-c", "-z", "-q", 0 };
        log_debug("exec lzma process");
        execvp("/usr/bin/lzma", const_cast<char* const*>(argv));
        cxxtools::SysError e("execvp");
        log_fatal("error running lzma process: " << e.what());
        exit(-1);
      }
    }
  }
  else
  {
    log_debug("don't compress " << job.data.size() << " bytes");
    job.zdata = job.data;
  }

  job.ready();
}

void CompressorImpl::compress(CompressJob::Ptr job)
{
  cxxtools::MutexLock lock(mutex);

  while (stop == STOP_NO && queue.size() >= maxqueue)
    notFull.wait(lock);

  if (stop == STOP_NOW)
  {
    notFull.signal();
    return;
  }

  queue.push_back(job);
  notEmpty.signal();
}

void CompressorImpl::waitReady()
{
  stop = STOP_READY;

  // wake up
  notEmpty.signal();

  for (unsigned n = 0; n < compressorThreads.size(); ++n)
  {
    compressorThreads[n]->join();
    delete compressorThreads[n];
  }
  compressorThreads.clear();
}

void CompressorImpl::doStop()
{
  stop = STOP_NOW;

  // wake up all
  notEmpty.signal();
  notFull.signal();

  for (unsigned n = 0; n < compressorThreads.size(); ++n)
  {
    compressorThreads[n]->join();
    delete compressorThreads[n];
  }
  compressorThreads.clear();
}

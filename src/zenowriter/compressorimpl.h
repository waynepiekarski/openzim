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

#ifndef COMPRESSORIMPL_H
#define COMPRESSORIMPL_H

#include <compressor.h>
#include <deque>
#include <vector>
#include <cxxtools/noncopyable.h>
#include <cxxtools/thread.h>
#include <cxxtools/mutex.h>
#include <cxxtools/condition.h>

class CompressorImpl : private cxxtools::NonCopyable
{
    std::vector<cxxtools::AttachedThread*> compressorThreads;
    std::deque<CompressJob::Ptr> queue;

    cxxtools::Mutex mutex;
    cxxtools::Condition notEmpty;
    cxxtools::Condition notFull;
    unsigned maxqueue;
    enum { STOP_NO, STOP_READY, STOP_NOW } stop;

    std::string errorMessage;

    // the thread method
    void run();
    void doJob(CompressJob& job);

  public:
    CompressorImpl(unsigned threads, unsigned maxqueue);

    // add a compress job
    void compress(CompressJob::Ptr job);

    // wait until queue is empty
    void waitReady();

    // initiate end of compression
    void doStop();

    const std::string& getErrorMessage() const { return errorMessage; }
};

#endif // COMPRESSORIMPL_H

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

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <string>
#include <cxxtools/refcounted.h>
#include <cxxtools/smartptr.h>
#include <zim/dirent.h>

struct CompressJob : public cxxtools::RefCounted
{
  // input
  std::string data;
  zim::Dirent::CompressionType compression;

  // output
  std::string zdata;

  // ready signal
  virtual void ready()  {}

  typedef cxxtools::SmartPtr<CompressJob> Ptr;
};

class CompressorImpl;
class Compressor : private cxxtools::NonCopyable
{
    CompressorImpl* impl;

  public:
    Compressor(unsigned threads, unsigned maxqueue);
    ~Compressor();
    void compress(CompressJob::Ptr job);
    void waitReady();
    void doStop();
    const std::string& getErrorMessage() const;
};

#endif

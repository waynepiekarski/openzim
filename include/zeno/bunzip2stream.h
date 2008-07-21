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


#ifndef ZENO_BUNZIP2STREAM_H
#define ZENO_BUNZIP2STREAM_H

#include <iostream>
#include <stdexcept>
#include <bzlib.h>
#include <zeno/bzip2.h>

namespace zeno
{
  class Bzip2UncompressError : public Bzip2Error
  {
    public:
      Bzip2UncompressError(int zRet_, const std::string& msg)
        : Bzip2Error(zRet_, msg)
          { }
  };

  class Bunzip2StreamBuf : public std::streambuf
  {
      bz_stream stream;
      char_type* obuffer;
      unsigned bufsize;
      std::streambuf* sink;

    public:
      explicit Bunzip2StreamBuf(std::streambuf* sink_, bool small = false, unsigned bufsize = 8192);
      ~Bunzip2StreamBuf();

      /// see std::streambuf
      int_type overflow(int_type c);
      /// see std::streambuf
      int_type underflow();
      /// see std::streambuf
      int sync();

      void setSink(std::streambuf* sink_)   { sink = sink_; }
  };

  class Bunzip2Stream : public std::ostream
  {
      Bunzip2StreamBuf streambuf;

    public:
      explicit Bunzip2Stream(std::streambuf* sink, bool small = false, unsigned bufsize = 8192)
        : std::ostream(0),
          streambuf(sink, small, bufsize)
        { init(&streambuf); }
      explicit Bunzip2Stream(std::ostream& sink, bool small = false, unsigned bufsize = 8192)
        : std::ostream(0),
          streambuf(sink.rdbuf(), small, bufsize)
        { init(&streambuf); }

      void setSink(std::streambuf* sink)   { streambuf.setSink(sink); }
      void setSink(std::ostream& sink)     { streambuf.setSink(sink.rdbuf()); }
  };
}

#endif // ZENO_BUNZIP2STREAM_H


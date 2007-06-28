/*
 * Copyright (C) 2007 Tommi Maekitalo
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


#ifndef INFLATESTREAM_H
#define INFLATESTREAM_H

#include <iostream>
#include <stdexcept>
#include <zlib.h>

class InflateError : public std::runtime_error
{
    int zRet;

  public:
    InflateError(int zRet_, const std::string& msg)
      : std::runtime_error(msg),
        zRet(zRet_)
        { }

    int getRet() const  { return zRet; }
};

class InflateStreamBuf : public std::streambuf
{
    z_stream stream;
    char_type* buffer;
    unsigned bufsize;

  public:
    explicit InflateStreamBuf(const char* source, unsigned size, unsigned bufsize = 8192);
    ~InflateStreamBuf();

    /// see std::streambuf
    int_type overflow(int_type c);
    /// see std::streambuf
    int_type underflow();
    /// see std::streambuf
    int sync();

    uLong getAdler() const   { return stream.adler; }
};

class InflateStream : public std::ostream
{
    InflateStreamBuf streambuf;

  public:
    InflateStream(const char* source, unsigned size)
      : std::ostream(&streambuf),
        streambuf(source, size)
        { }
};

#endif // INFLATESTREAM_H


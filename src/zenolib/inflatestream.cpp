/* inflatestream.cpp
 * Copyright (C) 2003-2005 Tommi Maekitalo
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "inflatestream.h"
#include <cxxtools/log.h>
#include <cxxtools/dynbuffer.h>
#include <sstream>

log_define("inflatestream")

namespace
{
  void checkError(int ret, z_stream& stream)
  {
    if (ret != Z_OK && ret != Z_STREAM_END)
    {
      log_error("InflateError " << ret << ": \"" << (stream.msg ? stream.msg : "") << '"');
      std::ostringstream msg;
      msg << "inflate-error " << ret;
      if (stream.msg)
        msg << ": " << stream.msg;
      throw InflateError(ret, msg.str());
    }
  }
}

InflateStreamBuf::InflateStreamBuf(const char* source, unsigned size, unsigned bufsize_)
  : buffer(new char_type[bufsize_]),
    bufsize(bufsize_)
{
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = 0;
  stream.total_out = 0;
  stream.total_in = 0;
  stream.next_in = (Bytef*)source;
  stream.avail_in = size;

  checkError(::inflateInit(&stream), stream);
}

InflateStreamBuf::~InflateStreamBuf()
{
  ::inflateEnd(&stream);
  delete[] buffer;
}

InflateStreamBuf::int_type InflateStreamBuf::overflow(int_type c)
{
  return traits_type::eof();
}

InflateStreamBuf::int_type InflateStreamBuf::underflow()
{
  stream.next_out = reinterpret_cast<Bytef*>(buffer);
  stream.avail_out = bufsize;
  log_debug("pre:avail_out=" << stream.avail_out << " avail_in=" << stream.avail_in);
  checkError(::inflate(&stream, Z_SYNC_FLUSH), stream);
  log_debug("post:avail_out=" << stream.avail_out << " avail_in=" << stream.avail_in);

  if (stream.next_out == reinterpret_cast<Bytef*>(buffer))
    throw std::runtime_error("cannot uncompress data");

  setg(buffer, buffer, reinterpret_cast<char*>(stream.next_out));

  return traits_type::to_int_type(*gptr());
}

int InflateStreamBuf::sync()
{
  return 0;
}

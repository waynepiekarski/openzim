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

#include <zeno/bunzip2stream.h>
#include <cxxtools/log.h>
#include <cxxtools/dynbuffer.h>
#include <sstream>

log_define("zeno.bzip2.uncompress")

namespace zeno
{
  namespace
  {
    int checkError(int ret, bz_stream& stream)
    {
      if (ret != BZ_OK && ret != BZ_RUN_OK && ret != BZ_FLUSH_OK && ret != BZ_FINISH_OK
        && ret != BZ_STREAM_END)
      {
        std::ostringstream msg;
        msg << "bzip2-error " << ret << ": " << Bzip2Error::getErrorString(ret);
        log_error(msg.str());
        throw Bzip2UncompressError(ret, msg.str());
      }
      return ret;
    }
  }

  Bunzip2StreamBuf::Bunzip2StreamBuf(std::streambuf* sink_, bool small, unsigned bufsize_)
    : obuffer(new char_type[bufsize_]),
      bufsize(bufsize_),
      sink(sink_)
  {
    memset(&stream, 0, sizeof(bz_stream));

    checkError(::BZ2_bzDecompressInit(&stream, 0, static_cast<int>(small)), stream);
    setp(obuffer, obuffer + bufsize);
  }

  Bunzip2StreamBuf::~Bunzip2StreamBuf()
  {
    ::BZ2_bzDecompressEnd(&stream);
    delete[] obuffer;
  }

  Bunzip2StreamBuf::int_type Bunzip2StreamBuf::overflow(int_type c)
  {
    log_debug("Bunzip2StreamBuf::overflow");

    // initialize input-stream for
    stream.next_in = obuffer;
    stream.avail_in = pptr() - pbase();

    int ret;
    do
    {
      // initialize zbuffer
      cxxtools::Dynbuffer<char> zbuffer(bufsize);
      stream.next_out = zbuffer.data();
      stream.avail_out = bufsize;

      log_debug("pre:avail_out=" << stream.avail_out << " avail_in=" << stream.avail_in);
      ret = ::BZ2_bzDecompress(&stream);
      checkError(ret, stream);
      log_debug("post:avail_out=" << stream.avail_out << " avail_in=" << stream.avail_in << " ret=" << ret);

      // copy zbuffer to sink
      std::streamsize count = bufsize - stream.avail_out;
      std::streamsize n = sink->sputn(reinterpret_cast<char*>(zbuffer.data()), count);
      if (n < count)
        return traits_type::eof();
    } while (ret != BZ_STREAM_END && stream.avail_in > 0);

    // reset outbuffer
    setp(obuffer, obuffer + bufsize);
    if (c != traits_type::eof())
      sputc(traits_type::to_char_type(c));

    return 0;
  }

  Bunzip2StreamBuf::int_type Bunzip2StreamBuf::underflow()
  {
    return traits_type::eof();
  }

  int Bunzip2StreamBuf::sync()
  {
    if (overflow(traits_type::eof()) == traits_type::eof())
      return -1;
    return 0;
  }
}

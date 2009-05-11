/*
 * Copyright (C) 2009 Tommi Maekitalo
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

#include <zim/uuid.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <cxxtools/md5stream.h>
#include <cxxtools/log.h>

log_define("zim.uuid")

namespace zim
{
  namespace
  {
    char hex[] = "0123456789abcdef";
    inline char hi(char v)
    { return hex[(v >> 4) & 0xf]; }

    inline char lo(char v)
    { return hex[v & 0xf]; }
  }

  Uuid Uuid::generate()
  {
    cxxtools::Md5stream m;

    clock_t c = clock();

    struct timeval tv;
    gettimeofday(&tv, 0);

    m << c << tv.tv_sec << tv.tv_usec;

    Uuid ret;
    log_debug("generated uuid: " << m.getHexDigest());
    m.getDigest(reinterpret_cast<unsigned char*>(&ret.data[0]));

    return ret;
  }

  std::ostream& operator<< (std::ostream& out, const Uuid& uuid)
  {
    for (unsigned n = 0; n < 4; ++n)
      out << hi(uuid.data[n]) << lo(uuid.data[n]);
    out << '-';
    for (unsigned n = 4; n < 6; ++n)
      out << hi(uuid.data[n]) << lo(uuid.data[n]);
    out << '-';
    for (unsigned n = 6; n < 8; ++n)
      out << hi(uuid.data[n]) << lo(uuid.data[n]);
    out << '-';
    for (unsigned n = 6; n < 8; ++n)
      out << hi(uuid.data[n]) << lo(uuid.data[n]);
    out << '-';
    for (unsigned n = 8; n < 16; ++n)
      out << hi(uuid.data[n]) << lo(uuid.data[n]);
    return out;
  }

}
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

#include <zeno/zintstream.h>
#include <cxxtools/log.h>

log_define("cxxtools.zintstream")

namespace zeno
{
  bool ZIntStream::get(unsigned &value)
  {
    int ch = source->sbumpc();
    if (ch == std::streambuf::traits_type::eof())
      return false;

    unsigned ret = static_cast<unsigned>(static_cast<unsigned char>(std::streambuf::traits_type::to_char_type(ch)));
    unsigned numb = ret & 0x3;
    ret >>= 2;
    unsigned s = 6;
    while (numb && (ch = source->sbumpc()) != std::streambuf::traits_type::eof())
    {
      ret += static_cast<unsigned>(
               static_cast<unsigned char>(
                 std::streambuf::traits_type::to_char_type(ch))) + 1 << s;
      s += 8;
      --numb;
    }

    if (numb)
      log_error("incomplete bytestream");
    else
      value = ret;

    return numb == 0;
  }

}

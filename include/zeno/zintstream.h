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

#ifndef ZENO_ZINTSTREAM_H
#define ZENO_ZINTSTREAM_H

#include <string>
#include <iostream>

namespace zeno
{
  class ZIntStream
  {
      std::streambuf* source;

    public:
      explicit ZIntStream(std::streambuf* source_)
        : source(source_)
       { }
      explicit ZIntStream(std::istream& source_)
        : source(source_.rdbuf())
        { }

      bool get(unsigned &value);
      bool good() const      { return source->sgetc() != std::streambuf::traits_type::eof(); }
      operator bool() const  { return source->sgetc() != std::streambuf::traits_type::eof(); }
  };
}
#endif  //  ZENO_ZINTSTREAM_H

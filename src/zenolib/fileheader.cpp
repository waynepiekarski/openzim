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

#include <zeno/fileheader.h>
#include <iostream>
#include <cxxtools/log.h>

log_define("zeno.file.header")

namespace zeno
{
  const size_type Fileheader::MAGIC = 1439867043;
  const size_type Fileheader::VERSION = 3;
  const size_type Fileheader::headerSize;

  Fileheader::Fileheader()
  {
    std::fill(header, header + headerSize, '\0');

    *reinterpret_cast<size_type*>(header + 0) = fromLittleEndian<size_type>(&MAGIC);
    *reinterpret_cast<size_type*>(header + 4) = fromLittleEndian<size_type>(&VERSION);
  }

  std::ostream& operator<< (std::ostream& out, const Fileheader& fh)
  {
    out.write(fh.header, Fileheader::headerSize);
    for (unsigned i = 0; i < Fileheader::headerFill; ++i)
      out << '\0';
    return out;
  }

  std::istream& operator>> (std::istream& in, Fileheader& fh)
  {
    in.read(fh.header, Fileheader::headerSize);
    if (in.gcount() != Fileheader::headerSize)
      in.setstate(std::ios::failbit);
    else
    {
      in.ignore(57);

      if (fh.getMagicNumber() != Fileheader::MAGIC)
      {
        log_error("invalid magic number " << fh.getMagicNumber() << " found - "
            << Fileheader::MAGIC << " expected");
        in.setstate(std::ios::failbit);
      }
      else if (fh.getVersion() != Fileheader::VERSION)
      {
        log_error("invalid zenofile version " << fh.getVersion() << " found - "
            << Fileheader::VERSION << " expected");
        in.setstate(std::ios::failbit);
      }
    }

    return in;
  }

}

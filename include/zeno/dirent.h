/*
 * Copyright (C) 2006 Tommi Maekitalo
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

#ifndef ZENO_DIRENT_H
#define ZENO_DIRENT_H

#include <string>
#include <zeno/zeno.h>
#include <zeno/endian.h>

namespace zeno
{
  class Dirent
  {
    public:
      static const uint8_t zenocompDefault = 0;
      static const uint8_t zenocompNone = 1;
      static const uint8_t zenocompZip = 2;

    private:

      char header[26];
      std::string extra;

    public:
      Dirent();
      explicit Dirent(char header_[26], std::string extra_ = std::string())
        : extra(extra_)
        { std::copy(header_, header_ + 26, header); }

      offset_type getOffset() const        { return fromLittleEndian<offset_type>(header); }
      size_type   getSize() const          { return fromLittleEndian<size_type>(header + 8); }
      uint8_t     getCompression() const   { return static_cast<uint8_t>(header[12]); }
      bool        isCompressionZip() const { return getCompression() == zenocompZip; }
      uint8_t     getMimeType() const      { return static_cast<uint8_t>(header[13]); }
      uint8_t     getSubtype() const       { return static_cast<uint8_t>(header[14]); }
      size_type   getSubtypeParent() const { return fromLittleEndian<size_type>(header + 16); }
      uint16_t    getExtraLen() const      { return fromLittleEndian<uint16_t>(header + 24); }

      const std::string& getExtra() const  { return extra; }
      void setExtra(const std::string& extra_)  { extra = extra_; }
      std::string getTitle() const
      {
        std::string::size_type p = extra.find('\0');
        return p == std::string::npos ? extra : std::string(extra, 0, p);
      }
  };
}

#endif // ZENO_DIRENT_H


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
      enum CompressionType
      {
        zenocompDefault,
        zenocompNone,
        zenocompZip
      };

      enum MimeType
      {
        zenoMimeTextHtml,
        zenoMimeTextPlain,
        zenoMimeImageJpeg,
        zenoMimeImagePng,
        zenoMimeImageTiff,
        zenoMimeTextCss,
        zenoMimeImageGif,
        zenoMimeIndex,
        zenoMimeApplicationJavaScript,
        zenoMimeImageIcon
      };

    private:

      char header[26];
      std::string title;
      std::string parameter;

    public:
      Dirent();
      explicit Dirent(char header_[26], std::string extra_ = std::string())
        {
          std::copy(header_, header_ + 26, header);
          setExtra(extra_);
        }

      offset_type getOffset() const        { return fromLittleEndian<offset_type>(header); }
      size_type   getSize() const          { return fromLittleEndian<size_type>(header + 8); }
      CompressionType getCompression() const { return static_cast<CompressionType>(header[12]); }
      bool        isCompressionZip() const { return getCompression() == zenocompZip; }
      MimeType    getMimeType() const      { return static_cast<MimeType>(header[13]); }
      bool        getRedirectFlag() const  { return static_cast<bool>(header[14]); }
      char        getNamespace() const     { return static_cast<char>(header[15]); }
      uint16_t    getExtraLen() const      { return fromLittleEndian<uint16_t>(header + 24); }

      void setExtra(const std::string& extra_);
      const std::string& getTitle() const      { return title; }
      const std::string& getParameter() const   { return parameter; }
  };

}

#endif // ZENO_DIRENT_H


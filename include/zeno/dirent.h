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
      friend std::ostream& operator<< (std::ostream& out, const Dirent& fh);
      friend std::istream& operator>> (std::istream& in, Dirent& fh);

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

      static const unsigned headerSize = 26;

    private:

      char header[headerSize];
      std::string title;
      std::string parameter;

      void adjustExtraLen()
      { setExtraLen(parameter.empty() ? title.size() : (title.size() + parameter.size() + 1)); }

    public:
      Dirent();
      explicit Dirent(char header_[headerSize], std::string extra_ = std::string())
        {
          std::copy(header_, header_ + headerSize, header);
          setExtra(extra_);
        }

      offset_type getOffset() const        { return fromLittleEndian<offset_type>(header); }
      void        setOffset(offset_type o) { *reinterpret_cast<offset_type*>(header + 0) = fromLittleEndian<offset_type>(&o); }

      size_type   getSize() const          { return fromLittleEndian<size_type>(header + 8); }
      void        setSize(size_type s)     { *reinterpret_cast<size_type*>(header + 8) = fromLittleEndian<size_type>(&s); }

      CompressionType getCompression() const { return static_cast<CompressionType>(header[12]); }
      bool        isCompressionZip() const { return getCompression() == zenocompZip; }
      void        setCompression(CompressionType c)
                                           { header[12] = c; }

      MimeType    getMimeType() const      { return static_cast<MimeType>(header[13]); }
      void        setMimeType(MimeType m)  { header[13] = m; }
      bool        getRedirectFlag() const  { return static_cast<bool>(header[14]); }
      void        setRedirectFlag(bool sw = true)   { header[14] = sw; }

      char        getNamespace() const     { return static_cast<char>(header[15]); }
      void        setNamespace(char ns)    { header[15] = ns; }

      uint16_t    getExtraLen() const      { return fromLittleEndian<uint16_t>(header + 24); }
      void        setExtraLen(uint16_t l)  { *reinterpret_cast<uint16_t*>(header + 24) = fromLittleEndian<uint16_t>(&l); } 

      void        setExtra(const std::string& extra_);

      const std::string& getTitle() const      { return title; }
      void        setTitle(const std::string& t)  { title = t; adjustExtraLen(); }

      const std::string& getParameter() const  { return parameter; }
      void        setParameter(const std::string& p)  { parameter = p; adjustExtraLen(); }
      std::string getExtra() const          { return parameter.empty() ? title : (title + '\0' + parameter); }
  };

  std::ostream& operator<< (std::ostream& out, const Dirent& fh);
  std::istream& operator>> (std::istream& in, Dirent& fh);

}

#endif // ZENO_DIRENT_H


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

#ifndef ZENO_ARTICLEBASE_H
#define ZENO_ARTICLEBASE_H

#include <string>
#include <zeno/zeno.h>
#include <zeno/dirent.h>
#include <zeno/qunicode.h>

namespace zeno
{
  class ArticleBase
  {
    public:
      typedef Dirent::CompressionType CompressionType;
      typedef Dirent::MimeType MimeType;

    private:
      Dirent dirent;
      std::string data;
      mutable std::string uncompressedData;

    public:
      ArticleBase() { }
      explicit ArticleBase(const Dirent& dirent_, const std::string& data_ = std::string())
        : dirent(dirent_),
          data(data_)
          { }

      Dirent&       getDirent()                 { return dirent; }
      const Dirent& getDirent() const           { return dirent; }
      void          setDirent(const Dirent& d)  { dirent = d; }

      const std::string&
                  getParameter() const        { return dirent.getParameter(); }
      void        setParameter(const std::string& p)  { dirent.setParameter(p); }

      offset_type getDataOffset() const       { return dirent.getOffset(); }
      void        setDataOffset(offset_type o) { dirent.setOffset(o); }

      size_type   getDataLen() const          { return dirent.getSize(); }

      CompressionType getCompression() const  { return dirent.getCompression(); }
      bool        isCompressionZip() const    { return dirent.isCompressionZip(); }
      void        setCompression(CompressionType c, bool modifyData = false);
      /// compress, if it reduces size significantly
      void        tryCompress(double maxSize = 0.9);

      QUnicodeString getTitle() const         { return QUnicodeString(dirent.getTitle()); }
      void        setTitle(const QUnicodeString& title)   { dirent.setTitle(title.getValue()); }

      MimeType    getLibraryMimeType() const  { return dirent.getMimeType(); }
      void        setLibraryMimeType(MimeType m) { dirent.setMimeType(m); }
      const std::string&
                  getMimeType() const;

      bool        getRedirectFlag() const     { return dirent.getRedirectFlag(); }
      void        setRedirectFlag(bool sw = true)  { dirent.setRedirectFlag(sw); }

      char        getNamespace() const        { return dirent.getNamespace(); }
      void        setNamespace(char ns)       { dirent.setNamespace(ns); }

      operator bool()   { return getDataOffset() != 0; }

      bool operator< (const ArticleBase& a) const
        { return getNamespace() < a.getNamespace()
              || getNamespace() == a.getNamespace()
               && getTitle() < a.getTitle(); }

      virtual const std::string& getRawData() const;
      void setRawData(const std::string& data_)
      {
        data = data_;
        uncompressedData.clear();
      }

      const std::string& getData() const;
      void setData(const std::string& data);
  };

}

#endif // ZENO_ARTICLEBASE_H


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

#ifndef ZENO_ARTICLE_H
#define ZENO_ARTICLE_H

#include <string>
#include <zeno/zeno.h>
#include <zeno/file.h>
#include <zeno/dirent.h>
#include <zeno/qunicode.h>

namespace zeno
{
  class Article
  {
    public:
      typedef Dirent::CompressionType CompressionType;
      typedef Dirent::MimeType MimeType;

    private:
      Dirent dirent;
      File file;
      size_type idx;
      mutable bool dataRead;
      mutable std::string data;
      mutable std::string uncompressedData;
      mutable unsigned countArticles;

    public:
      Article() : dataRead(true) { }
      Article(size_type idx_, const Dirent& dirent_, const File& file_,
              const std::string& data_)
        : file(file_),
          idx(idx_),
          dirent(dirent_),
          dataRead(true),
          data(data_),
          countArticles(0)
          { }
      Article(size_type idx_, const Dirent& dirent_, const File& file_)
        : file(file_),
          idx(idx_),
          dirent(dirent_),
          dataRead(false),
          countArticles(0)
          { }

      Dirent&     getDirent()                 { return dirent; }
      const Dirent& getDirent() const         { return dirent; }
      void        setDirent(const Dirent& d)  { dirent = d; }

      const File& getFile() const             { return file; }
      File&       getFile()                   { return file; }
      void        setFile(const File& f)      { file = f; }

      size_type   getIndex() const            { return idx; }
      void        setIndex(size_type i)       { idx = i; }

      const std::string&
                  getParameter() const        { return dirent.getParameter(); }
      void        setParameter(const std::string& p)  { dirent.setParameter(p); }

      offset_type getDataOffset() const       { return dirent.getOffset(); }
      void        setDataOffset(offset_type o) { dirent.setOffset(o); }

      size_type   getDataLen() const          { return dirent.getSize(); }
      void        setSize(size_type s)        { dirent.setSize(s); }

      CompressionType getCompression() const  { return dirent.getCompression(); }
      bool        isCompressionZip() const    { return dirent.isCompressionZip(); }
      void        setCompression(CompressionType c)  { dirent.setCompression(c); }

      QUnicodeString getUrl() const           { return QUnicodeString(std::string(1, getNamespace()) + '/' + dirent.getTitle()); }
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

      bool operator< (const Article& a) const
        { return getNamespace() < a.getNamespace()
              || getNamespace() == a.getNamespace()
               && getUrl() < a.getUrl(); }

      const std::string& getRawData() const;
      const std::string& getData() const;
      void setData(const std::string& data);
  };

}

#endif // ZENO_ARTICLE_H


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

      const File& getFile() const             { return file; }
      File&       getFile()                   { return file; }
      size_type   getIndex() const            { return idx; }
      const std::string&
                  getExtra() const            { return dirent.getExtra(); }
      offset_type getDataOffset() const       { return dirent.getOffset(); }
      size_type   getDataLen() const          { return dirent.getSize(); }
      CompressionType getCompression() const      { return dirent.getCompression(); }
      bool        isCompressionZip() const    { return dirent.isCompressionZip(); }
      uint8_t     getType() const             { return dirent.getExtra()[0]; }
      std::string getUrl() const              { return dirent.getTitle(); }
      MimeType    getLibraryMimeType() const  { return dirent.getMimeType(); }
      const std::string&
                  getMimeType() const;
      int         getSubtype() const          { return dirent.getSubtype(); }
      size_type   getSubtypeParent() const    { return dirent.getSubtypeParent(); }
      bool        isMainArticle() const       { return getSubtype() == 0; }
      unsigned    getCountSubarticles() const;

      operator bool()   { return getDataOffset() != 0; }

      const std::string& getRawData() const;
      const std::string& getData() const;

      class const_iterator;

      const_iterator begin() const;
      const_iterator end() const;
  };

  class Article::const_iterator : public std::iterator<std::bidirectional_iterator_tag, Article>
  {
      size_type idx;
      Article* mainArticle;
      mutable Article currentArticle;

    public:
      explicit const_iterator(Article* article = 0, size_type idx_ = 0)
        : mainArticle(article),
          idx(idx_)
      {
        if (mainArticle)
          currentArticle = mainArticle->getFile().getArticle(idx);
      }

      bool operator== (const const_iterator& it) const
        { return idx == it.idx; }
      bool operator!= (const const_iterator& it) const
        { return idx != it.idx; }

      const_iterator& operator++()
      {
        currentArticle = mainArticle->getFile().getArticle(++idx);
        return *this;
      }

      const_iterator operator++(int)
      {
        const_iterator it = *this;
        currentArticle = mainArticle->getFile().getArticle(++idx);
        return it;
      }

      const_iterator& operator--()
      {
        currentArticle = mainArticle->getFile().getArticle(--idx);
        return *this;
      }

      const_iterator operator--(int)
      {
        const_iterator it = *this;
        currentArticle = mainArticle->getFile().getArticle(--idx);
        return it;
      }

      Article operator*() const    { return currentArticle; }
      pointer operator->() const   { return &currentArticle; }
  };

  inline Article::const_iterator Article::begin() const
  { return const_iterator(const_cast<Article*>(this), idx + 1); }
}

#endif // ZENO_ARTICLE_H


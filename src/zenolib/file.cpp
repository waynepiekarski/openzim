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

#include <zeno/file.h>
#include <zeno/fileiterator.h>
#include <zeno/fileimpl.h>
#include <cxxtools/log.h>

log_define("zeno.file")

namespace zeno
{
  File::File(const std::string& fname)
    : impl(new FileImpl(fname.c_str()))
  {
    impl->addRef();
  }

  File::File(FileImpl* impl_)
    : impl(impl_)
  {
    if (impl)
      impl->addRef();
  }

  File::File(const File& f)
    : impl(f.impl)
  {
    if (impl)
      impl->addRef();
  }

  File& File::operator= (const File& f)
  {
    if (impl != f.impl)
    {
      if (impl)
        impl->release();
      impl = f.impl;
      if (impl)
        impl->addRef();
    }
  }

  File::~File()
  {
    if (impl)
      impl->release();
  }

  std::string File::readData(offset_type off, size_type count)
  {
    log_debug("readData(" << off << ", " << count << ')');
    return impl->readData(off, count);
  }

  Article File::getArticle(const std::string& url)
  {
    log_debug("getArticle(\"" << url << "\")");
    return impl->getArticle(url);
  }

  Article File::getArticle(size_type idx)
  {
    log_debug("getArticle(" << idx << ')');
    return impl->getArticle(idx);
  }

  Dirent File::getDirent(size_type idx)
  {
    log_debug("getDirent(" << idx << ')');
    return impl->getDirent(idx);
  }

  size_type File::getCountArticles() const
  {
    log_debug("getCountArticles()");
    return impl->getCountArticles();
  }

  File::const_iterator File::begin()
  {
    return const_iterator(this);
  }

  File::const_iterator File::end()
  {
    return const_iterator();
  }

  File::const_iterator File::find(const std::string& url)
  {
    return const_iterator(this, impl->findArticle(url).second);
  }

  File::const_iterator::const_iterator(File* file_)
    : file(file_),
      idx(0)
  {
    if (file)
    {
      while (idx < file->getCountArticles())
      {
        article = file->getArticle(idx);
        if (article.isMainArticle())
          break;
        ++idx;
      }
    }
  }

  File::const_iterator::const_iterator(File* file_, size_type idx_)
    : file(file_),
      idx(idx_)
  {
    article = file->getArticle(idx);
  }

  File::const_iterator& File::const_iterator::operator++()
  {
    do
    {
      ++idx;
      if (idx >= file->getCountArticles())
        break;
      article = file->getArticle(idx);
    } while (!article.isMainArticle());

    return *this;
  }

  File::const_iterator& File::const_iterator::operator--()
  {
    do
    {
      --idx;
      if (idx <= 0)
        break;
      article = file->getArticle(idx);
    } while (!article.isMainArticle());

    return *this;
  }

}

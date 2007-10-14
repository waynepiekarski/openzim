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
  { }

  File::File(FileImpl* impl_)
    : impl(impl_)
  { }

  std::string File::readData(offset_type off, size_type count)
  {
    log_debug("readData(" << off << ", " << count << ')');
    return impl->readData(off, count);
  }

  Article File::getArticle(char ns, const std::string& url)
  {
  log_debug("getArticle('" << ns << "', \"" << url << "\")");
    return impl->getArticle(ns, url);
  }

  Article File::getArticle(char ns, const QUnicodeString& url)
  {
    log_debug("getArticle('" << ns << "', \"" << url << "\")");
    return impl->getArticle(ns, url);
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

  File::const_iterator File::find(char ns, const std::string& url)
  {
    return const_iterator(this, impl->findArticle(ns, QUnicodeString(url)).second);
  }

  File::const_iterator File::find(char ns, const QUnicodeString& url)
  {
    return const_iterator(this, impl->findArticle(ns, url).second);
  }

  File::const_iterator::const_iterator(File* file_, size_type idx_)
    : file(file_),
      idx(idx_)
  {
    article = file->getArticle(idx);
  }

}

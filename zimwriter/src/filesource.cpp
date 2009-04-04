/*
 * Copyright (C) 2009 Tommi Maekitalo
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

#include <zim/writer/filesource.h>
#include <zim/blob.h>
#include <zim/zim.h>
#include <cxxtools/arg.h>
#include <cxxtools/systemerror.h>
#include <fstream>
#include <unistd.h>
#include <errno.h>
#include <iterator>
#include <limits>

namespace zim
{
  namespace writer
  {
    std::string FileArticle::getAid() const
    {
      return path;
    }

    char FileArticle::getNamespace() const
    {
      return ns;
    }

    std::string FileArticle::getTitle() const
    {
      return fname;
    }

    bool FileArticle::isRedirect() const
    {
      char p[256];
      ssize_t s = ::readlink(path.c_str(), p, sizeof(p));

      if (s < 0 && errno != EINVAL)
        throw cxxtools::SystemError( CXXTOOLS_ERROR_MSG("readlink failed") );

      return s > 0;
    }

    MimeType FileArticle::getMimeType() const
    {
      return zimMimeTextHtml;  // TODO
    }

    std::string FileArticle::getRedirectAid() const
    {
      char p[256];
      ssize_t s = ::readlink(path.c_str(), p, sizeof(p));

      if (s < 0)
        throw cxxtools::SystemError( CXXTOOLS_ERROR_MSG("readlink failed") );

      return p;
    }

    FileSource::FileSource(int& argc, char* argv[])
      : directory(cxxtools::Arg<const char*>(argc, argv, "--dir", "").getValue())
    {
    }

    const Article* FileSource::getNextArticle()
    {
      if (current == directory.end())
        current = directory.begin();
      else
        ++current;

      article = FileArticle('A', current.path(), *current);
      return &article;
    }

    Blob FileSource::getData(const std::string& aid)
    {
      std::ifstream in(aid.c_str());
      data.clear();
      std::copy(std::istream_iterator<char>(in), std::istream_iterator<char>(), std::back_inserter(data));
      return Blob(data.c_str(), data.size());
    }

    Uuid FileSource::getUuid()
    {
      return Uuid();
    }

    unsigned FileSource::getMainPage()
    {
      return std::numeric_limits<unsigned>::max();
    }

    unsigned FileSource::getLayoutPage()
    {
      return std::numeric_limits<unsigned>::max();
    }

  }

}

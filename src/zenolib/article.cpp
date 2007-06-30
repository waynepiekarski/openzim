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

#include <zeno/article.h>
#include <cxxtools/log.h>
#include <tnt/inflatestream.h>
#include <sstream>

log_define("zeno.article")

namespace zeno
{
  const std::string& Article::getMimeType() const
  {
    static const std::string textHtml = "text/html";
    static const std::string textPlain = "text/plain";
    static const std::string imageJpeg = "image/jpeg";
    static const std::string imagePng = "image/png";
    static const std::string imageTiff = "image/tiff";
    static const std::string textCss = "text/css";
    static const std::string imageGif = "image/gif";
    static const std::string index = "text/plain";
    static const std::string applicationJavaScript = "application/x-javascript";
    static const std::string imageIcon = "image/x-icon";

    switch (getLibraryMimeType())
    {
      case Dirent::zenoMimeTextHtml:
        return textHtml;
      case Dirent::zenoMimeTextPlain:
        return textPlain;
      case Dirent::zenoMimeImageJpeg:
        return imageJpeg;
      case Dirent::zenoMimeImagePng:
        return imagePng;
      case Dirent::zenoMimeImageTiff:
        return imageTiff;
      case Dirent::zenoMimeTextCss:
        return textCss;
      case Dirent::zenoMimeImageGif:
        return imageGif;
      case Dirent::zenoMimeIndex:
        return index;
      case Dirent::zenoMimeApplicationJavaScript:
        return applicationJavaScript;
      case Dirent::zenoMimeImageIcon:
        return imageIcon;
    }

    return textHtml;
  }

  const std::string& Article::getRawData() const
  {
    if (!dataRead)
    {
      data = const_cast<File&>(file).readData(getDataOffset(), getDataLen());
      dataRead = true;
    }
    return data;
  }

  const std::string& Article::getData() const
  {
    if (uncompressedData.empty() && !getRawData().empty())
    {
      if (isCompressionZip())
      {
        std::ostringstream u;
        tnt::InflateStream is(u);
        is << getRawData() << std::flush;
        uncompressedData = u.str();
      }
      else
      {
        uncompressedData = getRawData();
      }
    }
    return uncompressedData;
  }

  unsigned Article::getCountSubarticles() const
  {
    if (countArticles == 0)
    {
      size_type i = idx + 1;
      countArticles = 1;
      while (!const_cast<File&>(file).getArticle(i).isMainArticle())
      {
        ++i;
        ++countArticles;
      }
    }
    return countArticles - 1;
  }

  Article::const_iterator Article::end() const
  {
    return const_iterator(const_cast<Article*>(this), idx + 1 + const_cast<Article*>(this)->getCountSubarticles());
  }

  QUnicodeString Article::getTitle() const
  {
    std::string url = dirent.getTitle();
    std::string::size_type n = url.find_first_of('/');
    return QUnicodeString(n == std::string::npos ? url : url.substr(n + 1));
  }
}

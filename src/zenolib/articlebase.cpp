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

#include <zeno/articlebase.h>
#include <cxxtools/log.h>
#include <zeno/inflatestream.h>
#include <zeno/deflatestream.h>
#include <sstream>

log_define("zeno.article.base")

namespace zeno
{
  const std::string& ArticleBase::getMimeType() const
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

  const std::string& ArticleBase::getRawData() const
  {
    return data;
  }

  std::string ArticleBase::getData() const
  {
    if (uncompressedData.empty() && !getRawData().empty())
    {
      if (isCompressionZip())
      {
        log_debug("uncompress data (zlib)");
        std::ostringstream u;
        zeno::InflateStream is(u);
        is << getRawData() << std::flush;
        uncompressedData = u.str();
      }
      else if (isCompressionBzip2())
      {
        // TODO
        log_debug("uncompress data (bzip2)");
      }
      else if (isCompressionLzma())
      {
        // TODO
        log_debug("uncompress data (lzma)");
      }
      else
      {
        uncompressedData = getRawData();
      }
    }

    return getArticleSize() > 0 ? uncompressedData.substr(getArticleOffset(), getArticleSize())
                                : uncompressedData;
  }

  void ArticleBase::tryCompress(double maxSize)
  {
    if (isCompressed())
      return;

    setCompression(Dirent::zenocompZip, true);
    unsigned rawSize = getRawData().size();
    unsigned compressedSize = getData().size();
    double r = static_cast<double>(rawSize) / static_cast<double>(compressedSize);
    log_debug("raw size=" << rawSize << " compressed size=" << compressedSize << " ratio=" << r);
    if (r > maxSize)
    {
      log_debug("not worth compressing");
      setCompression(Dirent::zenocompNone);
    }
  }

  void ArticleBase::setCompression(CompressionType c, bool modifyData)
  {
    if (dirent.getCompression() != c)
    {
      if (modifyData)
      {
        if (isCompressed())
        {
          // we need to uncompress the data

          uncompressedData = getData(); // this returns the uncompressed data
          data = uncompressedData;
          dirent.setCompression(c);     // disable compression
        }
        else
        {
          // we need to compress the data

          dirent.setCompression(c);     // enable compression
          setData(data);  // setting the data with compression enabled compresses it
        }
      }
      else
        dirent.setCompression(c);
    }
  }

  void ArticleBase::setData(const std::string& data_)
  {
    uncompressedData = data_;
    if (isCompressionZip())
    {
      log_debug("compress data");
      std::ostringstream u;
      zeno::DeflateStream ds(u);
      ds << data_ << std::flush;
      ds.end();
      data = u.str();
    }
    else
    {
      data = data_;
    }
    dirent.setSize(data.size());
  }

}

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

#include <zim/writer/zimcreator.h>
#include <zim/fileheader.h>
#include <zim/cluster.h>
#include <zim/blob.h>
#include <zim/endian.h>
#include <cxxtools/arg.h>
#include <cxxtools/log.h>
#include <algorithm>
#include <fstream>
#include <unistd.h>

log_define("zim.writer.creator")

namespace zim
{
  namespace writer
  {
    ZimCreator::ZimCreator(int& argc, char* argv[], ArticleSource& src_)
      : src(src_)
    {
      minChunkSize = cxxtools::Arg<unsigned>(argc, argv, 's', 512);
    }

    void ZimCreator::create(const std::string& fname)
    {
      log_debug("create directory entries");
      createDirents();
      log_debug(dirents.size() << " directory entries created");

      log_debug("create clusters");
      createClusters(fname + ".tmp");
      log_debug(clusterOffsets.size() << " clusters created");

      log_debug("fill header");
      fillHeader();

      log_debug("write zimfile");
      write(fname + ".zim", fname + ".tmp");

      ::unlink((fname + ".tmp").c_str());

      log_debug("ready");
    }

    void ZimCreator::createDirents()
    {
      const Article* article;
      while ((article = src.getNextArticle()) != 0)
      {
        Dirent dirent;
        dirent.setAid(article->getAid());
        dirent.setTitle(article->getNamespace(), QUnicodeString::fromUtf8(article->getTitle()));

        log_debug("article " << dirent.getNamespace() << '/' << dirent.getTitle().toUtf8() << " fetched");

        if (article->isRedirect())
        {
          dirent.setRedirectAid(article->getRedirectAid());
          log_debug("is redirect to " << dirent.getRedirectAid());
        }
        else
        {
          dirent.setArticle(article->getMimeType(), 0, 0);
          log_debug("is article; mimetype " << dirent.getMimeType());
        }

        dirents.push_back(dirent);
      }

      // remove invalid redirects
      DirentsType::iterator di = dirents.begin();
      while (di != dirents.end())
      {
        if (di->isRedirect())
        {
          // check, if redirect article is found
          DirentsType::const_iterator ddi;
          for (ddi = dirents.begin(); ddi != dirents.end(); ++ddi)
            if (ddi->getAid() == di->getRedirectAid())
              break;

          if (ddi == dirents.end())
          {
            dirents.erase(di);
            di = dirents.begin();
          }
          else
            ++di;
        }
        else
          ++di;
      }

      // sort
      std::sort(dirents.begin(), dirents.end());

      // fill index
      unsigned idx = 0;
      for (DirentsType::iterator di = dirents.begin(); di != dirents.end(); ++di)
        di->setIdx(idx++);

      // translate redirect aid to index
      for (DirentsType::iterator di = dirents.begin(); di != dirents.end(); ++di)
      {
        if (di->isRedirect())
        {
          for (DirentsType::const_iterator ddi = dirents.begin(); ddi != dirents.end(); ++ddi)
          {
            if (ddi->getAid() == di->getRedirectAid())
            {
              di->setRedirect(ddi->getIdx());
              break;
            }
          }
        }
      }
    }

    void ZimCreator::createClusters(const std::string& tmpfname)
    {
      std::ofstream out(tmpfname.c_str());

      Cluster cluster;
      cluster.setCompression(zimcompBzip2);

      for (DirentsType::iterator di = dirents.begin(); di != dirents.end(); ++di)
      {
        if (di->isRedirect())
          continue;

        di->setCluster(clusterOffsets.size(), cluster.count());

        Blob blob = src.getData(di->getAid());
        cluster.addBlob(blob);
        if (cluster.size() >= minChunkSize * 1024)
        {
          clusterOffsets.push_back(out.tellp());
          out << cluster;
          cluster.clear();
        }
      }

      if (cluster.count() > 0)
      {
        clusterOffsets.push_back(out.tellp());
        out << cluster;
      }
    }

    void ZimCreator::fillHeader()
    {
      header.setUuid( src.getUuid() );
      header.setArticleCount( dirents.size() );
      header.setIndexPtrPos( indexPtrPos() );
      header.setClusterCount( clusterOffsets.size() );
      header.setClusterPtrPos( clusterPtrPos() );
      header.setMainPage( src.getMainPage() );
      header.setLayoutPage( src.getLayoutPage() );

      log_debug("indexPtrSize=" << indexPtrSize()
        << " indexPtrPos=" << indexPtrPos()
        << " indexSize=" << indexSize()
        << " indexPos=" << indexPos()
        << " clusterPtrSize=" << clusterPtrSize()
        << " clusterPtrPos=" << clusterPtrPos()
        << " clusterCount=" << clusterCount()
        << " articleCount=" << articleCount());

      log_debug("articleCount=" << dirents.size()
        << " indexPtrPos=" << header.getIndexPtrPos()
        << " clusterCount=" << header.getClusterCount()
        << " clusterPtrPos=" << header.getClusterPtrPos());
    }

    void ZimCreator::write(const std::string& fname, const std::string& tmpfname)
    {
      std::ofstream zimfile(fname.c_str());
      zimfile << header;

      log_debug("after writing header - pos=" << zimfile.tellp());

      offset_type off = Fileheader::size + dirents.size() * sizeof(offset_type);
      for (DirentsType::const_iterator it = dirents.begin(); it != dirents.end(); ++it)
      {
        offset_type ptr0 = fromLittleEndian<offset_type>(&off);
        zimfile.write(reinterpret_cast<const char*>(&ptr0), sizeof(ptr0));
        off += it->getDirentSize();
      }

      log_debug("after writing direntPtr - pos=" << zimfile.tellp());

      for (DirentsType::const_iterator it = dirents.begin(); it != dirents.end(); ++it)
      {
        zimfile << *it;
        log_debug("write " << it->getTitle() << " dirent.size()=" << it->getDirentSize() << " pos=" << zimfile.tellp());
      }

      log_debug("after writing dirents - pos=" << zimfile.tellp());

      off += clusterOffsets.size() * sizeof(offset_type);
      for (OffsetsType::const_iterator it = clusterOffsets.begin(); it != clusterOffsets.end(); ++it)
      {
        offset_type o = (off + *it);
        offset_type ptr0 = fromLittleEndian<offset_type>(&o);
        zimfile.write(reinterpret_cast<const char*>(&ptr0), sizeof(ptr0));
      }

      log_debug("after writing clusterOffsets - pos=" << zimfile.tellp());

      std::ifstream blobsfile(tmpfname.c_str());
      zimfile << blobsfile.rdbuf();

      log_debug("after writing clusterData - pos=" << zimfile.tellp());
    }

    offset_type ZimCreator::indexSize() const
    {
      offset_type s = 0;
      for (DirentsType::const_iterator it = dirents.begin(); it != dirents.end(); ++it)
      {
        s += it->getDirentSize();
        log_debug("title=" << it->getTitle() << " size=" << s << " dirent.size=" << it->getDirentSize());
      }
      return s;
    }

  }
}

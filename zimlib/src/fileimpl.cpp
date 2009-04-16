/*
 * Copyright (C) 2006,2009 Tommi Maekitalo
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

#include <zim/fileimpl.h>
#include <zim/error.h>
#include <zim/dirent.h>
#include <zim/endian.h>
#include <cxxtools/log.h>

log_define("zim.file.impl")

namespace zim
{
  //////////////////////////////////////////////////////////////////////
  // FileImpl
  //
  FileImpl::FileImpl(const char* fname)
    : zimFile(fname),
      direntCache(64),
      clusterCache(16)
  {
    if (!zimFile)
      throw ZenoFileFormatError(std::string("can't open zim-file \"") + fname + '"');

    filename = fname;

    // read header
    zimFile >> header;
    if (zimFile.fail())
      throw ZenoFileFormatError("error reading zim-file header");

    // read index offsets
    {
      size_type indexOffsetsSize = header.getArticleCount() * sizeof(OffsetsType::value_type);
      log_debug("read " << indexOffsetsSize << " bytes indexptr");
      zimFile.seekg(header.getIndexPtrPos());
      indexOffsets.resize(header.getArticleCount());
      zimFile.read(reinterpret_cast<char*>(&indexOffsets[0]), indexOffsetsSize);
    }

    if (isBigEndian())
    {
      for (OffsetsType::iterator it = indexOffsets.begin(); it != indexOffsets.end(); ++it)
        *it = fromLittleEndian(&*it);
    }

    // read cluster offsets
    {
      size_type clusterOffsetsSize = header.getClusterCount() * sizeof(OffsetsType::value_type);
      log_debug("read " << clusterOffsetsSize << " bytes clusterptr");
      zimFile.seekg(header.getClusterPtrPos());
      clusterOffsets.resize(header.getClusterCount());
      zimFile.read(reinterpret_cast<char*>(&clusterOffsets[0]), clusterOffsetsSize);
    }

    if (isBigEndian())
    {
      for (OffsetsType::iterator it = clusterOffsets.begin(); it != clusterOffsets.end(); ++it)
        *it = fromLittleEndian(&*it);
    }

  }

  Dirent FileImpl::getDirent(size_type idx)
  {
    log_trace("FileImpl::getDirent(" << idx << ')');

    if (idx >= indexOffsets.size())
      throw ZenoFileFormatError("article index out of range");

    if (!zimFile)
    {
      log_warn("file in error state");
      throw ZenoFileFormatError("file in error state");
    }

    std::pair<bool, Dirent> v = direntCache.getx(idx);
    if (v.first)
    {
      log_debug("dirent " << idx << " found in cache");
      return v.second;
    }

    log_debug("seek to " << indexOffsets[idx]);
    zimFile.seekg(indexOffsets[idx]);
    if (!zimFile)
    {
      log_warn("failed to seek to directory entry");
      throw ZenoFileFormatError("failed to seek to directory entry");
    }

    log_debug("seeked to directory entry");

    Dirent dirent;
    zimFile >> dirent;

    if (!zimFile)
    {
      log_warn("failed to read to directory entry");
      throw ZenoFileFormatError("failed to read directory entry");
    }

    log_debug("reading dirent was successful");
    direntCache.put(idx, dirent);

    return dirent;
  }

  Cluster FileImpl::getCluster(size_type idx)
  {
    log_trace("getCluster(" << idx << ')');

    if (idx >= clusterOffsets.size())
      throw ZenoFileFormatError("article index out of range");

    Cluster cluster = clusterCache.get(idx);
    if (cluster)
    {
      log_debug("cluster found in cache; count=" << cluster.count() << " size=" << cluster.size());
      return cluster;
    }

    log_debug("read cluster " << idx << " from offset " << clusterOffsets[idx]);
    zimFile.seekg(clusterOffsets[idx]);
    zimFile >> cluster;

    if (zimFile.fail())
      throw ZenoFileFormatError("error reading cluster data");

    if (cluster.isCompressed())
    {
      log_debug("put cluster " << idx << " into cluster cache");
      clusterCache.put(idx, cluster);
    }
    else
      log_debug("cluster " << idx << " is not compressed - do not cache");

    return cluster;
  }

  size_type FileImpl::getNamespaceBeginOffset(char ch)
  {
    log_trace("getNamespaceBeginOffset(" << ch << ')');

    NamespaceCache::const_iterator it = namespaceBeginCache.find(ch);
    if (it != namespaceBeginCache.end())
      return it->second;

    size_type lower = 0;
    size_type upper = getCountArticles();
    Dirent d = getDirent(0);
    while (upper - lower > 1)
    {
      size_type m = lower + (upper - lower) / 2;
      Dirent d = getDirent(m);
      if (d.getNamespace() >= ch)
        upper = m;
      else
        lower = m;
    }

    size_type ret = d.getNamespace() < ch ? upper : lower;
    namespaceBeginCache[ch] = ret;

    return ret;
  }

  size_type FileImpl::getNamespaceEndOffset(char ch)
  {
    log_trace("getNamespaceEndOffset(" << ch << ')');

    NamespaceCache::const_iterator it = namespaceEndCache.find(ch);
    if (it != namespaceEndCache.end())
      return it->second;

    size_type lower = 0;
    size_type upper = getCountArticles();
    log_debug("namespace " << ch << " lower=" << lower << " upper=" << upper);
    while (upper - lower > 1)
    {
      size_type m = lower + (upper - lower) / 2;
      Dirent d = getDirent(m);
      if (d.getNamespace() > ch)
        upper = m;
      else
        lower = m;
      log_debug("namespace " << d.getNamespace() << " m=" << m << " lower=" << lower << " upper=" << upper);
    }

    namespaceEndCache[ch] = upper;

    return upper;

  }

  std::string FileImpl::getNamespaces()
  {
    if (namespaces.empty())
    {
      Dirent d = getDirent(0);
      namespaces = d.getNamespace();

      size_type idx;
      while ((idx = getNamespaceEndOffset(d.getNamespace())) < getCountArticles())
      {
        d = getDirent(idx);
        namespaces += d.getNamespace();
      }

    }
    return namespaces;
  }

}

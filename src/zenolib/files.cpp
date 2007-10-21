/*
 * Copyright (C) 2007 Tommi Maekitalo
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

#include <zeno/files.h>
#include <zeno/error.h>
#include <cxxtools/dir.h>
#include <cxxtools/log.h>

log_define("zeno.files");

namespace zeno
{
  void Files::addFiles(const std::string& dir)
  {
    log_debug("search for zeno-files in directory " << dir);
    cxxtools::Dir d(dir.c_str());
    for (cxxtools::Dir::const_iterator it = d.begin(); it != d.end(); ++it)
    {
      std::string fname = *it;
      if (fname.size() > 4 && fname.compare(fname.size() - 5, 5, ".zeno") == 0)
      {
        log_debug("file \"" << fname << "\" found");
        try
        {
          files.push_back(File(dir + '/' + fname));
        }
        catch (const ZenoFileFormatError& e)
        {
          log_error('"' << fname << "\" is no zeno file: " << e.what());
        }
      }
    }
  }

  Files::FilesType Files::getFiles(char ns)
  {
    FilesType ret;
    for (FilesType::iterator it = files.begin(); it != files.end(); ++it)
    {
      if (it->getNamespaceBeginOffset(ns) < it->getNamespaceEndOffset(ns))
        ret.push_back(*it);
    }
    return ret;
  }
}

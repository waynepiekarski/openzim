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

#include <iostream>
#include <cxxtools/loginit.h>
#include <cxxtools/log.h>
#include <zeno/search.h>
#include <zeno/files.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h> 

int main(int argc, char* argv[])
{
  try
  {
    log_init();
    if (argc != 3)
    {
      std::cerr << "usage: " << argv[0] << " zeno-directory searchstring" << std::endl;
      return 1;
    }

    zeno::Files files;

    struct stat st;
    if (stat(argv[1], &st) != 0)
      throw cxxtools::SysError("stat");

    if (st.st_mode & S_IFDIR)
      files.addFiles(argv[1]);
    else
      files.addFile(argv[1]);

    zeno::Search search(files);
    zeno::Search::Results result;
    search.search(result, argv[2]);

    for (zeno::Search::Results::const_iterator it = result.begin(); it != result.end(); ++it)
    {
      std::cout << "article " << it->getArticle().getIndex() << "\tpriority " << it->getPriority() << "\t:\t" << it->getArticle().getTitle() << std::endl;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}


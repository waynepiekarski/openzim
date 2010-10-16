/*
 * Copyright (C) 2010 Tommi Maekitalo
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
#include <cxxtools/arg.h>
#include <cxxtools/log.h>
#include <zim/writer/search.h>
#include <zim/writer/zimcreator.h>

log_define("zim.writer.search")

#define INFO(e) \
    do { \
        log_info(e); \
        std::cout << e << std::endl; \
    } while(false)

int main(int argc, char* argv[])
{
  try
  {
    cxxtools::Arg<std::string> search(argc, argv, 'S'); // create zimfile from search result

    INFO("create zim file from search result");
    std::string fname = argv[1];
    zim::writer::ZimCreator creator(argc, argv);
    zim::writer::SearchSource source(search, argc, argv);
    creator.create(fname, source);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}


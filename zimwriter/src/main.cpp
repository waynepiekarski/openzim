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

#include <iostream>
#include <cxxtools/loginit.h>
#include <zim/writer/dbsource.h>
#include <zim/writer/filesource.h>
#include <zim/writer/zimcreator.h>

int create(int argc, char* argv[], zim::writer::ArticleSource& articleSource)
{
  zim::writer::ZimCreator creator(argc, argv, articleSource);

  if (argc != 2)
  {
    std::cerr << "usage: " << argv[0] << " [options] filename" << std::endl;
    return 1;
  }

  std::string fname = argv[1];
  articleSource.setFilename(fname);
  creator.create(fname);
}

int main(int argc, char* argv[])
{
  try
  {
    log_init();

    try
    {
      zim::writer::FileSource fileSource(argc, argv);
      return create(argc, argv, fileSource);
    }
    catch (const cxxtools::IOError&)
    {
      zim::writer::DbSource dbSource(argc, argv);
      return create(argc, argv, dbSource);
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}


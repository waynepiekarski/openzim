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

#ifndef ZIM_WRITER_FILESOURCE_H
#define ZIM_WRITER_FILESOURCE_H

#include <zim/writer/articlesource.h>
#include <iosfwd>
#include <cxxtools/directory.h>

namespace zim
{
  namespace writer
  {
    class FileArticle : public Article
    {
        char ns;
        std::string path;
        std::string fname;

      public:
        FileArticle()   { }
        FileArticle(char ns_, const std::string& path_, const std::string& fname_)
          : ns(ns_),
            path(path_),
            fname(fname_)
          { }

        virtual std::string getAid() const;
        virtual char getNamespace() const;
        virtual std::string getTitle() const;
        virtual bool isRedirect() const;
        virtual MimeType getMimeType() const;
        virtual std::string getRedirectAid() const;
    };

    class FileSource : public ArticleSource
    {
        cxxtools::Directory directory;
        cxxtools::Directory::const_iterator current;
        FileArticle article;
        std::string data;

      public:
        FileSource(int& argc, char* argv[]);

        bool ok() const  { return !directory.path().empty(); }

        virtual const Article* getNextArticle();
        virtual Blob getData(const std::string& aid);
        virtual Uuid getUuid();
        virtual unsigned getMainPage();
        virtual unsigned getLayoutPage();
    };

  }
}

#endif // ZIM_WRITER_FILESOURCE_H

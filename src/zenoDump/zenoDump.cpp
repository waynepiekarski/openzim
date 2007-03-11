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

#include <iostream>
#include <zeno/file.h>
#include <zeno/fileiterator.h>
#include <cxxtools/arg.h>
#include <cxxtools/loginit.h>
#include <stdexcept>

log_define("zeno.dumper");

class ZenoDumper
{
    zeno::File file;
    zeno::File::const_iterator pos;

  public:
    explicit ZenoDumper(const char* fname)
      : file(fname),
        pos(file.begin())
      { }
    
    void printInfo();
    void locateArticle(zeno::size_type idx);
    void findArticle(const char* url);
    void dumpArticle(bool raw = false);
    void listArticles(bool info);
    static void listArticle(const zeno::Article& article, bool listsubs);
    void listArticle(bool listsubs)
      { listArticle(*pos, listsubs); }
};

void ZenoDumper::printInfo()
{
  std::cout << "count-articles: " << file.getCountArticles() << std::endl;
}

void ZenoDumper::locateArticle(zeno::size_type idx)
{
  log_debug("locateArticle(" << idx << ')');

  if (idx > file.getCountArticles())
    throw std::range_error("index too large");
  pos = zeno::File::const_iterator(&file, idx);
}

void ZenoDumper::findArticle(const char* url)
{
  log_debug("findArticle(" << url << ')');
  pos = file.find(url);
  log_debug("findArticle(" << url << ") => idx=" << pos.getIndex());
}

void ZenoDumper::dumpArticle(bool raw)
{
  std::cout << (raw ? pos->getRawData() : pos->getData()) << std::flush;
}

void ZenoDumper::listArticles(bool info)
{
  for (zeno::File::const_iterator it = pos; it != file.end(); ++it)
  {
    if (info)
      listArticle(*it);
    else
      std::cout << it->getUrl() << '\n';
  }
}

void ZenoDumper::listArticle(const zeno::Article& article, bool listsubs)
{
  std::cout << "url: " << article.getUrl() << "\n"
    "\tidx:             " << article.getIndex() << "\n"
    "\toff:             " << article.getDataOffset() << "\n"
    "\tlen:             " << article.getDataLen() << "\n"
    "\tcompression:     " << static_cast<unsigned>(article.getCompression()) << "\n";
  if (article.getCompression())
    std::cout <<
    "\tuncompressedlen: " << article.getData().size() << "\n";
  std::cout <<
    "\ttype:            " << static_cast<unsigned>(article.getType()) << "\n"
    "\tmime-type:       " << article.getLibraryMimeType() << "\n"
    "\tsubtype:         " << article.getSubtype() << "\n"
    "\tsubtype-parent:  " << article.getSubtypeParent() << "\n"
    "\tsub-articles:    " << article.getCountSubarticles() << std::endl;
  if (article.isMainArticle() && listsubs)
    for (zeno::Article::const_iterator it = article.begin(); it != article.end(); ++it)
      listArticle(*it, false);
}

int main(int argc, char* argv[])
{
  try
  {
    log_init();

    cxxtools::Arg<bool> fileinfo(argc, argv, 'F');
    cxxtools::Arg<bool> info(argc, argv, 'i');
    cxxtools::Arg<bool> data(argc, argv, 'd');
    cxxtools::Arg<bool> rawdump(argc, argv, 'r');
    cxxtools::Arg<const char*> find(argc, argv, 'f');
    cxxtools::Arg<bool> list(argc, argv, 'l');
    cxxtools::Arg<bool> subarticles(argc, argv, 's');
    cxxtools::Arg<zeno::size_type> indexOffset(argc, argv, 'o');

    if (argc <= 1)
    {
      std::cerr << "usage: " << argv[0] << " [options] zenofile\n"
                   "\n"
                   "options:\n"
                   "  -F        print fileinfo\n"
                   "  -i        print info about articles\n"
                   "  -d        print data of articles\n"
                   "  -r        print raw data (possibly compressed data)\n"
                   "  -f url    find article\n"
                   "  -l        list articles or subarticles\n"
                   "  -o idx    locate article\n"
                   "  -s        list subarticles\n"
                   "\n"
                   "examples:\n"
                   "  " << argv[0] << " -F wikipedia.zeno\n"
                   "  " << argv[0] << " -l wikipedia.zeno\n"
                   "  " << argv[0] << " -f A/Auto -i wikipedia.zeno\n"
                   "  " << argv[0] << " -f A/Auto -d wikipedia.zeno\n"
                   "  " << argv[0] << " -f A/Auto -l wikipedia.zeno\n"
                   "  " << argv[0] << " -f A/Auto -l -i wikipedia.zeno\n"
                   "  " << argv[0] << " -o 123159 -l -i wikipedia.zeno\n"
                 << std::flush;
      return -1;
    }

    // initalize app
    ZenoDumper app(argv[1]);

    // global info
    if (fileinfo)
      app.printInfo();

    // locate article
    if (indexOffset.isSet())
      app.locateArticle(indexOffset);
    else if (find.isSet())
      app.findArticle(find);

    // print requested info
    if (data || rawdump)
      app.dumpArticle(rawdump);
    else if (info)
      app.listArticle(list);
    else if (list)
      app.listArticles(info);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return -2;
  }
  return 0;
}


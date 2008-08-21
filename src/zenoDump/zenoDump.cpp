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
#include <sstream>
#include <set>
#include <zeno/file.h>
#include <zeno/files.h>
#include <zeno/fileiterator.h>
#include <zeno/zintstream.h>
#include <zeno/indexarticle.h>
#include <cxxtools/arg.h>
#include <cxxtools/loginit.h>
#include <cxxtools/hirestime.h>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>

log_define("zeno.dumper")

class ZenoDumper
{
    zeno::File file;
    zeno::File::const_iterator pos;

  public:
    explicit ZenoDumper(const char* fname)
      : file(fname),
        pos(file.begin())
      { }
    
    static void listZenoFiles(const std::string& directory);
    static void listZenoFiles(const std::string& directory, char ns);
    void printInfo();
    void printNsInfo(char ch);
    void locateArticle(zeno::size_type idx);
    void findArticle(char ns, const char* url, bool collate);
    void dumpArticle(bool raw = false);
    static void printIndexcontent(zeno::IndexArticle article);
    void listArticles(bool info, bool extra, bool verbose, bool indexcontent);
    static void listArticle(const zeno::Article& article, bool extra, bool verbose, bool indexcontent);
    void listArticle(bool extra, bool verbose, bool indexcontent)
      { listArticle(*pos, extra, verbose, indexcontent); }
    void dumpFiles(const std::string& directory);
};

void ZenoDumper::listZenoFiles(const std::string& directory)
{
  zeno::Files zenofiles(directory);
  for (zeno::Files::iterator it = zenofiles.begin(); it != zenofiles.end(); ++it)
    std::cout << it->first << ": " << it->second.getFilename() << " ns " << it->second.getNamespaces() << std::endl;
}

void ZenoDumper::listZenoFiles(const std::string& directory, char ns)
{
  zeno::Files zenofiles(directory);
  zeno::Files files = zenofiles.getFiles(ns);
  for (zeno::Files::iterator it = files.begin(); it != files.end(); ++it)
    std::cout << it->first << ": " << it->second.getFilename()
      << " offset " << it->second.getNamespaceBeginOffset(ns) << " - " << it->second.getNamespaceEndOffset(ns) << std::endl;
}

void ZenoDumper::printInfo()
{
  std::cout << "count-articles: " << file.getCountArticles() << "\n"
               "namespaces: " << file.getNamespaces() << "\n"
               "index ptr pos: " << file.getFileheader().getIndexPtrPos() << "\n"
               "index ptr len: " << file.getFileheader().getIndexPtrLen() << "\n"
               "index pos: " << file.getFileheader().getIndexPos() << "\n"
               "index len: " << file.getFileheader().getIndexLen() << "\n"
               "data pos: " << file.getFileheader().getDataPos() << std::endl;
}

void ZenoDumper::printNsInfo(char ch)
{
  std::cout << "namespace " << ch << "\n"
               "lower bound idx: " << file.getNamespaceBeginOffset(ch) << "\n"
               "upper bound idx: " << file.getNamespaceEndOffset(ch) << std::endl;
}

void ZenoDumper::locateArticle(zeno::size_type idx)
{
  log_debug("locateArticle(" << idx << ')');
  pos = zeno::File::const_iterator(&file, idx);
}

void ZenoDumper::findArticle(char ns, const char* url, bool collate)
{
  log_debug("findArticle(" << ns << ", " << url << ", " << collate << ')');
  pos = file.find(ns, url, collate);
  log_debug("findArticle(" << ns << ", " << url << ", " << collate << ") => idx=" << pos.getIndex());
}

void ZenoDumper::dumpArticle(bool raw)
{
  log_trace("dump article; raw data size=" << pos->getRawData().size());
  std::string data = raw ? pos->getRawData() : pos->getData();
  //log_debug("data size=" << data.size());
  std::cout << (raw ? pos->getRawData() : pos->getData()) << std::flush;
}

void ZenoDumper::printIndexcontent(zeno::IndexArticle article)
{
  for (unsigned c = 0; c <= 3; ++c)
  {
    std::cout << "category " << c << "\n";
    zeno::IndexArticle::EntriesType e = article.getCategory(c);
    for (zeno::IndexArticle::EntriesType::const_iterator it = e.begin();
         it != e.end(); ++it)
      std::cout << "\tindex " << it->index << "\tpos " << it->pos << '\n';
  }
}

void ZenoDumper::listArticles(bool info, bool extra, bool verbose, bool indexcontent)
{
  for (zeno::File::const_iterator it = pos; it != file.end(); ++it)
  {
    if (info)
      listArticle(*it, extra, verbose, indexcontent);
    else
      std::cout << it->getUrl() << '\n';
  }
}

void ZenoDumper::listArticle(const zeno::Article& article, bool extra, bool verbose, bool indexcontent)
{
  std::cout <<
      "title: " << article.getTitle() << "\n"
    "\tidx:             " << article.getIndex() << "\n"
    "\tnamespace:       " << article.getNamespace() << "\n"
    "\tredirect:        " << article.getRedirectFlag() << "\n";

  if (article.getRedirectFlag())
  {
    std::cout <<
      "\tredirect index:  " << article.getRedirectIndex() << "\n";
  }
  else
  {
    std::cout <<
      "\toff:             " << article.getDataOffset() << "\n"
      "\tlen:             " << article.getDataLen() << "\n"
      "\tmime-type:       " << article.getLibraryMimeType() << "\n"
      "\tarticle-size:    " << article.getArticleSize() << "\n"
      "\tarticle-offset:  " << article.getArticleOffset() << "\n"
      "\tcompression:     " << static_cast<unsigned>(article.getCompression()) << "\n";

    if (verbose && article.getCompression())
    {
      cxxtools::HiresTime t0 = cxxtools::HiresTime::gettimeofday();
      zeno::size_type len = article.getUncompressedLen();
      cxxtools::HiresTime t1 = cxxtools::HiresTime::gettimeofday();
      std::cout <<
      "\tuncompressedlen: " << len << "\n"
      "\tuncompress time: " << (t1 - t0) << " s\n";
    }
  }

  if (extra)
  {
    std::string parameter = article.getParameter();
    std::cout << "\textra:           ";
    static char hexdigit[] = "0123456789abcdef";
    for (std::string::const_iterator it = parameter.begin(); it != parameter.end(); ++it)
    {
      unsigned val = static_cast<unsigned>(static_cast<unsigned char>(*it));
      std::cout << hexdigit[val >> 4] << hexdigit[val & 0xf] << ' ';
    }
    std::cout << ':';

    if (parameter.size() > 1)
    {
      std::istringstream s(parameter);
      s.get(); // skip length byte
      zeno::ZIntStream in(s);

      unsigned val;
      while (in.get(val))
        std::cout << '\t' << val;
    }

    std::cout << std::endl;

    if (indexcontent)
      printIndexcontent(article);
  }
}

void ZenoDumper::dumpFiles(const std::string& directory)
{
  std::set<char> ns;
  for (zeno::File::const_iterator it = pos; it != file.end(); ++it)
  {
    std::string d = directory + '/' + it->getNamespace();
    if (ns.find(it->getNamespace()) == ns.end())
      ::mkdir(d.c_str(), 0777);
    std::string t = it->getTitle().getValue();
    std::string::size_type p;
    while ((p = t.find('/')) != std::string::npos)
      t.replace(p, 1, "%2f");
    std::string f = d + '/' + t;
    std::ofstream out(f.c_str());
    out << it->getData();
    if (!out)
      throw std::runtime_error("error writing file " + f);
  }
}

int main(int argc, char* argv[])
{
  try
  {
    log_init();

    cxxtools::Arg<bool> fileinfo(argc, argv, 'F');
    cxxtools::Arg<bool> listzenofiles(argc, argv, 'L');
    cxxtools::Arg<char> nsinfo(argc, argv, 'N');
    cxxtools::Arg<bool> info(argc, argv, 'i');
    cxxtools::Arg<bool> data(argc, argv, 'd');
    cxxtools::Arg<bool> rawdump(argc, argv, 'r');
    cxxtools::Arg<const char*> find(argc, argv, 'f');
    cxxtools::Arg<bool> list(argc, argv, 'l');
    cxxtools::Arg<zeno::size_type> indexOffset(argc, argv, 'o');
    cxxtools::Arg<bool> extra(argc, argv, 'x');
    cxxtools::Arg<bool> indexcontent(argc, argv, 'X');
    cxxtools::Arg<char> ns(argc, argv, 'n', 'A');  // namespace
    cxxtools::Arg<bool> collate(argc, argv, 'c');
    cxxtools::Arg<const char*> dumpAll(argc, argv, 'D');
    cxxtools::Arg<bool> verbose(argc, argv, 'v');

    if (argc <= 1)
    {
      std::cerr << "usage: " << argv[0] << " [options] zenofile\n"
                   "\n"
                   "options:\n"
                   "  -F        print fileinfo\n"
                   "  -L        list zenofiles in directory with contained namespaces\n"
                   "  -N ns     print info about namespace\n"
                   "  -i        print info about articles\n"
                   "  -d        print data of articles\n"
                   "  -r        print raw data (possibly compressed data)\n"
                   "  -f url    find article\n"
                   "  -l        list articles\n"
                   "  -o idx    locate article\n"
                   "  -x        print extra parameters\n"
                   "  -X        print index contents\n"
                   "  -n ns     specify namespace (default 'A')\n"
                   "  -D dir    dump all files into directory\n"
                   "  -v        verbose (print uncompressed length of articles when -i is set)\n"
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

    if (listzenofiles)
    {
      if (ns.isSet())
        ZenoDumper::listZenoFiles(argv[1], ns);
      else
        ZenoDumper::listZenoFiles(argv[1]);
      return 0;
    }

    // initalize app
    ZenoDumper app(argv[1]);

    // global info
    if (fileinfo)
      app.printInfo();

    // namespace info
    if (nsinfo.isSet())
      app.printNsInfo(nsinfo);

    // locate article
    if (indexOffset.isSet())
      app.locateArticle(indexOffset);
    else if (find.isSet())
      app.findArticle(ns, find, collate);

    // dump files
    if (dumpAll.isSet())
      app.dumpFiles(dumpAll.getValue());

    // print requested info
    if (data || rawdump)
      app.dumpArticle(rawdump);
    else if (list)
      app.listArticles(info, extra, verbose, indexcontent);
    else if (info)
      app.listArticle(extra, verbose, indexcontent);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return -2;
  }
  return 0;
}


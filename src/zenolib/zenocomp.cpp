////////////////////////////////////////////////////////////////////////
// zenocomp.cpp
// generated with ecppc
// date: Wed Jan  3 20:19:18 2007
//

#include <tnt/ecpp.h>
#include <tnt/convert.h>
#include <tnt/httprequest.h>
#include <tnt/httpreply.h>
#include <tnt/httpheader.h>
#include <tnt/http.h>
#include <tnt/data.h>
#include <tnt/componentfactory.h>
#include <tnt/componentguard.h>
#include <tnt/objecttemplate.h>
#include <tnt/objectptr.h>
#include <tnt/comploader.h>
#include <tnt/tntconfig.h>
#include <cxxtools/log.h>
#include <stdexcept>

log_define("component.zenocomp")

// <%pre>
#line 1 "zenocomp.ecpp"

#include <zeno/file.h>
#include <zeno/article.h>

// </%pre>

namespace
{
template <typename T> inline void use(const T&) { }

// <%declare_shared>
// </%declare_shared>
class zenocomp : public tnt::EcppComponent
{
    zenocomp& main()  { return *this; }

    // <%declare>
    // </%declare>

  protected:
    ~zenocomp();

  public:
    zenocomp(const tnt::Compident& ci, const tnt::Urlmapper& um, tnt::Comploader& cl);

    unsigned operator() (tnt::HttpRequest& request, tnt::HttpReply& reply, cxxtools::QueryParams& qparam);
    void drop();

    // <%config>
    static std::string ZenoFile;
    // </%config>

};

class zenocompFactory : public tnt::SingletonComponentFactory
{
  public:
    zenocompFactory(const std::string& componentName)
      : tnt::SingletonComponentFactory(componentName)
      { }
    virtual tnt::Component* doCreate(const tnt::Compident& ci,
      const tnt::Urlmapper& um, tnt::Comploader& cl);
    virtual void doConfigure(const tnt::Tntconfig& config);
};

tnt::Component* zenocompFactory::doCreate(const tnt::Compident& ci,
  const tnt::Urlmapper& um, tnt::Comploader& cl)
{
  return new zenocomp(ci, um, cl);
}

void zenocompFactory::doConfigure(const tnt::Tntconfig& config)
{
  // <%config>
    if (config.hasValue("ZenoFile"))
      zenocomp::ZenoFile = config.getValue("ZenoFile");
  // </%config>
}

zenocompFactory factory("zenocomp");

static const char* rawData = "\014\000\000\000Z\000\000\000l\000\000\000<html>\n <head>\n  <base href=\"http://localhost:8000/\"></base>\n </head>\n <body>\n\n </body>\n</html>\n";

// <%shared>
// </%shared>

// <%config>
std::string zenocomp::ZenoFile =  "wikipedia.zeno";
// </%config>

zenocomp::zenocomp(const tnt::Compident& ci, const tnt::Urlmapper& um, tnt::Comploader& cl)
  : EcppComponent(ci, um, cl)
{
  // <%init>
  // </%init>
}

zenocomp::~zenocomp()
{
  // <%cleanup>
  // </%cleanup>
}

unsigned zenocomp::operator() (tnt::HttpRequest& request, tnt::HttpReply& reply, cxxtools::QueryParams& qparam)
{
  tnt::DataChunks data(rawData);

  TNT_APPLICATION_COMPONENT_VAR(zeno::File, file, "file", (ZenoFile.c_str()));   // <%application> zeno::File file(ZenoFile.c_str())
  // <%args>
  // </%args>

  // <%cpp>
#line 11 "zenocomp.ecpp"


cxxtools::QueryParams q;
q.parse_url(request.getPathInfo());
if (q.paramcount() == 0)
  return HTTP_NOT_FOUND;

zeno::Article article = file.getArticle(q[0]);
if (!article)
  return DECLINED;

log_debug("article " << request.getPathInfo() << " fetched - mime-type " << article.getLibraryMimeType());


  reply.out() << data[0]; // <html>\n <head>\n  <base href="http://localhost:8000/"></base>\n </head>\n <body>\n
#line 30 "zenocomp.ecpp"
  reply.out() << ( article.getData() );
  reply.out() << data[1]; // \n </body>\n</html>\n
  // <%/cpp>
  return HTTP_OK;
}

void zenocomp::drop()
{
  factory.drop(this);
}

} // namespace


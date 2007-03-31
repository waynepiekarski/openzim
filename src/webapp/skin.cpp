////////////////////////////////////////////////////////////////////////
// skin.cpp
// generated with ecppc
// date: Sat Mar 31 17:27:21 2007
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
#include <cxxtools/log.h>
#include <stdexcept>

log_define("component.skin")

// <%pre>
// </%pre>

namespace
{
template <typename T> inline void use(const T&) { }

// <%declare_shared>
// </%declare_shared>
class skin : public tnt::EcppComponent
{
    skin& main()  { return *this; }

    // <%declare>
    // </%declare>

  protected:
    ~skin();

  public:
    skin(const tnt::Compident& ci, const tnt::Urlmapper& um, tnt::Comploader& cl);

    unsigned operator() (tnt::HttpRequest& request, tnt::HttpReply& reply, cxxtools::QueryParams& qparam);
    void drop();

};

class skinFactory : public tnt::SingletonComponentFactory
{
  public:
    skinFactory(const std::string& componentName)
      : tnt::SingletonComponentFactory(componentName)
      { }
    virtual tnt::Component* doCreate(const tnt::Compident& ci,
      const tnt::Urlmapper& um, tnt::Comploader& cl);
};

tnt::Component* skinFactory::doCreate(const tnt::Compident& ci,
  const tnt::Urlmapper& um, tnt::Comploader& cl)
{
  return new skin(ci, um, cl);
}

skinFactory factory("skin");

static const char* rawData = "\024\000\000\000I\001\000\000.\005\000\000\320\007\000\0008\021\000\000<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"de\" lang=\"de\" dir=\"ltr\">\n  <head>\n   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\" />\n   <base href='http:///'></base>\";\n   <link rel=\"shortcut icon\" href=\"/favicon.ico\" />\n   <link rel=\"copyright\" href=\"http://www.gnu.org/copyleft/fdl.html\" />\n    <style type=\"text/css\" media=\"screen,projection\">/*<![CDATA[*/ @import \"-/main.css\"; /*]]>*/</style>\n    <link rel=\"stylesheet\" type=\"text/css\" media=\"print\" href=\"-/commonPrint.css\" />\n    <!--[if lt IE 5.5000]><style type=\"text/css\">@import \"-/IE50Fixes.css\";</style><![endif]-->\n    <!--[if IE 5.5000]><style type=\"text/css\">@import \"-/IE55Fixes.css\";</style><![endif]-->\n    <!--[if IE 6]><style type=\"text/css\">@import \"-/IE60Fixes.css\";</style><![endif]-->\n    <!--[if IE 7]><style type=\"text/css\">@import \"-/IE70Fixes.css\";</style><![endif]-->\n    <!--[if lt IE 7]><script type=\"text/javascript\" src=\"-/IEFixes.js\"></script>\n    <meta http-equiv=\"imagetoolbar\" content=\"no\" /><![endif]-->\n\t\t<script type=\" text/javascript\">\n\t\t\tvar skin = \"monobook\";\n\t\t\tvar stylepath = \"\";\n\t\t\tvar wgArticlePath = \"A/$1\";\n\t\t\tvar wgScriptPath = \"\";\n\t\t\tvar wgServer = \"\";\n\t\t\tvar wgCanonicalNamespace = \"\";\n\t\t\tvar wgNamespaceNumber = 0;\n\t\t\tvar wgPageName = \"\";\n\t\t\tvar wgTitle = \"\";\n\t\t\tvar wgArticleId = 0;\n\t\t\tvar wgIsArticle = true;\n\t\t\tvar wgUserName = null;\n\t\t\tvar wgUserLanguage = \"de\";\n\t\t\tvar wgContentLanguage = \"de\";\n\t\t</script>\n\t\t<script type=\"text/javascript\" src=\"-/wikibits.js\"><!-- wikibits js --></script>\n\t\t<style type=\"text/css\">/*<![CDATA[*/\n@import \"-/common.css\";\n@import \"-/monobookde.css\";\n@import \"-/user.css\";\n@import \"-/zenowp.css\";\n/*]]>*/</style>\n  </head>\n  <body  class=\"mediawiki ns--1 ltr\">\n    <div id=\"globalWrapper\">\n      <div id=\"column-content\">\n        <div id=\"content\">\n          <a name=\"top\" id=\"top\"></a>\n\n<div class=\"visualClear\"></div>\n          </div>\n        </div>\n      </div>\n      <div id=\"column-one\">\n\t\t  <div id=\"p-cactions\" class=\"portlet\">\n\t\t\t <h5>Diese Seite</h5>\n\t\t\t <ul>\n\t\t\t   <li id=\"ca-nstab-special\" class=\"selected\"><a href=\"\">Spezialseite</a></li>\n\t\t\t </ul>\n\t\t  </div>\n\t\t  \t<div class=\"portlet\" id=\"p-personal\">\n\t\t  \t\t<h5>Ausgabe</h5>\n\t\t  \t\t<div class=\"pBody\">\n\t\t  \t\t\t<ul>\n\t\t  \t\t\t\t<li>DVD-ROM-Ausgabe vom 20. September 2006</li>\n\t\t  \t\t\t</ul>\n\t\t  \t\t</div>\n\t\t  \t</div>\n\t\t  <div class=\"portlet\" id=\"p-logo\">\n\t\t\t <a style=\"background-image: url(/-/Wiki.png);\" href=\"\" title=\"Hauptseite\"></a>\n\t\t  </div>\n\t\t  <script type=\"text/javascript\"> if (window.isMSIE55) fixalpha(); </script>\n\t\t  <div class=\"portlet\" id=\"p-navigation\">\n\t\t\t <h5>Navigation</h5>\n\t\t\t <div class=\"pBody\">\n\t\t\t   <ul>\n\t\t\t\t <li id=\"n-mainpage\"><a href=\"\">Hauptseite</a></li>\n\t\t\t\t <li id=\"n-topics\"><a href=\"P/Wikipedia_nach_Themen\">Themenportale</a></li>\n\t\t\t\t <li id=\"n-alphindex\"><a href=\"~/browse\?n=A&amp;s=0&amp;c=100&amp;a=A\">Von A bis Z</a></li>\n\t\t\t\t <li id=\"n-images\"><a href=\"~/browse\?n=I&amp;s=0&amp;c=20\">Abbildungen</a></li>\n\t\t\t\t <li id=\"n-randompage\"><a href=\"~/random\?n=A\">Zuf&auml;lliger Artikel</a></li>\n\t\t\t   </ul>\n\t\t\t </div>\n\t\t  </div>\n\t\t  <div id=\"p-search\" class=\"portlet\">\n\t\t\t<h5><label for=\"searchInput\">Suche</label></h5>\n\t\t\t<div id=\"searchBody\" class=\"pBody\">\n\t\t\t  <form method=\"get\" action=\"~/search\" id=\"searchform\">\n\t\t\t\t<div>\n\t\t\t\t  <input type=\"hidden\" name=\"l\" value=\"Wikipedia.index\" />\n\t\t\t\t  <input type=\"text\" id=\"searchInput\" name=\"e\" accesskey=\"f\" value=\"\" />\n\t\t\t\t  <input type=\"submit\" name=\"go\" class=\"searchButton\" id=\"searchGoButton\"\tvalue=\"Artikel\" />&nbsp;\n\t\t\t\t  <input type=\"submit\" name=\"ft\" class=\"searchButton\" value=\"Volltext\" />\n\t\t\t\t</div>\n\t\t\t  </form>\n\t\t\t</div>\n\t\t  </div>\n\t\t  <div class=\"portlet\"><h5>Powered by...</h5><a href=\"http://www.tntnet.org/\"><img src=\"/~/tntnet.png\" alt=\"powered by tntnet\" title=\"powered by tntnet\"/></a></div>\n      </div>\n      <div class=\"visualClear\"></div>\n      <div id=\"footer\">\n        <p>Der Inhalt der Seite steht unter der <a href=\"-/GFDL_(englisch).html\">GNU-Lizenz f&uuml;r freie Dokumentation</a>.</p>\n        <p><a href=\"-/Impressum.html\">Impressum</a> - <a href=\"http://www.directmedia.de\">Directmedia</a></p>\n      </div>\n      <script type=\"text/javascript\">if (window.runOnloadHook) runOnloadHook();</script>\n    </div>\n  </body>\n</html>\n";

// <%shared>
// </%shared>

// <%config>
// </%config>

skin::skin(const tnt::Compident& ci, const tnt::Urlmapper& um, tnt::Comploader& cl)
  : EcppComponent(ci, um, cl)
{
  // <%init>
  // </%init>
}

skin::~skin()
{
  // <%cleanup>
  // </%cleanup>
}

unsigned skin::operator() (tnt::HttpRequest& request, tnt::HttpReply& reply, cxxtools::QueryParams& qparam)
{
  tnt::DataChunks data(rawData);

  // <%args>
std::string nextComp = qparam.param("nextComp");
  // </%args>

  // <%cpp>
  reply.out() << data[0]; // <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">\n<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="de" lang="de" dir="ltr">\n  <head>\n   <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />\n   <base href='http://
#line 8 "skin.ecpp"
  reply.sout() << (request.getHeader(tnt::httpheader::host));
  reply.out() << data[1]; // /'></base>";\n   <link rel="shortcut icon" href="/favicon.ico" />\n   <link rel="copyright" href="http://www.gnu.org/copyleft/fdl.html" />\n    <style type="text/css" media="screen,projection">/*<![CDATA[*/ @import "-/main.css"; /*]]>*/</style>\n    <link rel="stylesheet" type="text/css" media="print" href="-/commonPrint.css" />\n    <!--[if lt IE 5.5000]><style type="text/css">@import "-/IE50Fixes.css";</style><![endif]-->\n    <!--[if IE 5.5000]><style type="text/css">@import "-/IE55Fixes.css";</style><![endif]-->\n    <!--[if IE 6]><style type="text/css">@import "-/IE60Fixes.css";</style><![endif]-->\n    <!--[if IE 7]><style type="text/css">@import "-/IE70Fixes.css";</style><![endif]-->\n    <!--[if lt IE 7]><script type="text/javascript" src="-/IEFixes.js"></script>\n    <meta http-equiv="imagetoolbar" content="no" /><![endif]-->\n\t\t<script type=" text/javascript">\n\t\t\tvar skin = "monobook";\n\t\t\tvar stylepath = "";\n\t\t\tvar wgArticlePath = "A/$1";\n\t\t\tvar wgScriptPath = "";\n\t\t\tvar wgServer = "
#line 24 "skin.ecpp"
  reply.sout() << (request.getHeader(tnt::httpheader::host));
  reply.out() << data[2]; // ";\n\t\t\tvar wgCanonicalNamespace = "";\n\t\t\tvar wgNamespaceNumber = 0;\n\t\t\tvar wgPageName = "";\n\t\t\tvar wgTitle = "";\n\t\t\tvar wgArticleId = 0;\n\t\t\tvar wgIsArticle = true;\n\t\t\tvar wgUserName = null;\n\t\t\tvar wgUserLanguage = "de";\n\t\t\tvar wgContentLanguage = "de";\n\t\t</script>\n\t\t<script type="text/javascript" src="-/wikibits.js"><!-- wikibits js --></script>\n\t\t<style type="text/css">/*<![CDATA[*/\n@import "-/common.css";\n@import "-/monobookde.css";\n@import "-/user.css";\n@import "-/zenowp.css";\n/*]]>*/</style>\n  </head>\n  <body  class="mediawiki ns--1 ltr">\n    <div id="globalWrapper">\n      <div id="column-content">\n        <div id="content">\n          <a name="top" id="top"></a>\n
#line 48 "skin.ecpp"
 log_debug("call next component \"" << nextComp << '"');

  // <& (nextComp) ...
#line 49 "skin.ecpp"
  callComp((nextComp), request, reply, qparam);
  // &>
  reply.out() << data[3]; // \n<div class="visualClear"></div>\n          </div>\n        </div>\n      </div>\n      <div id="column-one">\n\t\t  <div id="p-cactions" class="portlet">\n\t\t\t <h5>Diese Seite</h5>\n\t\t\t <ul>\n\t\t\t   <li id="ca-nstab-special" class="selected"><a href="">Spezialseite</a></li>\n\t\t\t </ul>\n\t\t  </div>\n\t\t  \t<div class="portlet" id="p-personal">\n\t\t  \t\t<h5>Ausgabe</h5>\n\t\t  \t\t<div class="pBody">\n\t\t  \t\t\t<ul>\n\t\t  \t\t\t\t<li>DVD-ROM-Ausgabe vom 20. September 2006</li>\n\t\t  \t\t\t</ul>\n\t\t  \t\t</div>\n\t\t  \t</div>\n\t\t  <div class="portlet" id="p-logo">\n\t\t\t <a style="background-image: url(/-/Wiki.png);" href="" title="Hauptseite"></a>\n\t\t  </div>\n\t\t  <script type="text/javascript"> if (window.isMSIE55) fixalpha(); </script>\n\t\t  <div class="portlet" id="p-navigation">\n\t\t\t <h5>Navigation</h5>\n\t\t\t <div class="pBody">\n\t\t\t   <ul>\n\t\t\t\t <li id="n-mainpage"><a href="">Hauptseite</a></li>\n\t\t\t\t <li id="n-topics"><a href="P/Wikipedia_nach_Themen">Themenportale</a></li>\n\t\t\t\t <li id="n-alphindex"><a href="~/browse\?n=A&amp;s=0&amp;c=100&amp;a=A">Von A bis Z</a></li>\n\t\t\t\t <li id="n-images"><a href="~/browse\?n=I&amp;s=0&amp;c=20">Abbildungen</a></li>\n\t\t\t\t <li id="n-randompage"><a href="~/random\?n=A">Zuf&auml;lliger Artikel</a></li>\n\t\t\t   </ul>\n\t\t\t </div>\n\t\t  </div>\n\t\t  <div id="p-search" class="portlet">\n\t\t\t<h5><label for="searchInput">Suche</label></h5>\n\t\t\t<div id="searchBody" class="pBody">\n\t\t\t  <form method="get" action="~/search" id="searchform">\n\t\t\t\t<div>\n\t\t\t\t  <input type="hidden" name="l" value="Wikipedia.index" />\n\t\t\t\t  <input type="text" id="searchInput" name="e" accesskey="f" value="" />\n\t\t\t\t  <input type="submit" name="go" class="searchButton" id="searchGoButton"\tvalue="Artikel" />&nbsp;\n\t\t\t\t  <input type="submit" name="ft" class="searchButton" value="Volltext" />\n\t\t\t\t</div>\n\t\t\t  </form>\n\t\t\t</div>\n\t\t  </div>\n\t\t  <div class="portlet"><h5>Powered by...</h5><a href="http://www.tntnet.org/"><img src="/~/tntnet.png" alt="powered by tntnet" title="powered by tntnet"/></a></div>\n      </div>\n      <div class="visualClear"></div>\n      <div id="footer">\n        <p>Der Inhalt der Seite steht unter der <a href="-/GFDL_(englisch).html">GNU-Lizenz f&uuml;r freie Dokumentation</a>.</p>\n        <p><a href="-/Impressum.html">Impressum</a> - <a href="http://www.directmedia.de">Directmedia</a></p>\n      </div>\n      <script type="text/javascript">if (window.runOnloadHook) runOnloadHook();</script>\n    </div>\n  </body>\n</html>\n
  // <%/cpp>
  return HTTP_OK;
}

void skin::drop()
{
  factory.drop(this);
}

} // namespace


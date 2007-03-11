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

#include "skin.h"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cxxtools/log.h>

log_define("zenoreader.skin")

namespace zenoreader
{
  Skin::Skin(const std::string& skin)
    : std::ostream(data.rdbuf())
  {
    if (skin.empty())
    {
      log_debug("empty skin, using default");
      std::ostringstream d;
      d <<
        "<html>\n"
        " <head>\n"
        "  <title>TntZenoReader</title>\n"
        "  <base href=\"http://<!--%%HOST%%-->/\"></base>\n"
        " </head>\n"
        " <body bgcolor='#eee'>\n"
        "<!--%%CONTENT%%-->\n"
        " </body>\n"
        "</html>\n";
      skindata = d.str();
    }
    else
    {
      log_debug("read skinfile \"skin/" << skin << '"');

      std::ifstream in(("skin/" + skin).c_str());
      std::copy(std::istreambuf_iterator<char>(in),
                std::istreambuf_iterator<char>(),
                std::back_inserter(skindata));

      log_debug("skinfile has " << skindata.size() << " bytes");
    }
  }

  std::string Skin::getData(const std::string& host) const
  {
    static const std::string hostTag = "<!--%%HOST%%-->";
    static const std::string headTag = "<!--%%HEAD%%-->";
    static const std::string contentTag = "<!--%%CONTENT%%-->";
    static const std::string searchformTag = "<!--%%SEARCH%%-->";

    std::string ret = skindata;

    std::string headdata = "<base href='http://" + host + "/'></base>";
    std::string searchform = "<form action='search'><input type='text' name='search' size='20'><br><input type='submit' name='getpage' value='Artikel'><input type='submit' name='dosearch' value='Volltext'></form>\n";

    std::string::size_type pos = 0;
    log_debug("replace head");
    while ((pos = ret.find(headTag, pos)) != std::string::npos)
    {
      log_debug("head found at pos " << pos);
      ret.replace(pos, headTag.size(), headdata);
      pos += headdata.size();
    }

    pos = 0;
    log_debug("replace host");
    while ((pos = ret.find(hostTag, pos)) != std::string::npos)
    {
      log_debug("host found at pos " << pos);
      ret.replace(pos, hostTag.size(), data.str());
      pos += headdata.size();
    }

    pos = 0;
    log_debug("replace content");
    while ((pos = ret.find(contentTag, pos)) != std::string::npos)
    {
      log_debug("content found at pos " << pos);
      ret.replace(pos, contentTag.size(), data.str());
      pos += headdata.size();
    }

    pos = 0;
    log_debug("replace searchform");
    while ((pos = ret.find(searchformTag, pos)) != std::string::npos)
    {
      log_debug("searchform found at pos " << pos);
      ret.replace(pos, searchformTag.size(), searchform);
      pos += headdata.size();
    }

    log_debug("data has " << ret.size() << " bytes");

    return ret;
  }
}

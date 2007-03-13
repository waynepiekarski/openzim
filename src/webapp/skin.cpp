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
  namespace
  {
    static const std::string hostTag = "<!--%%HOST%%-->";
    static const std::string headTag = "<!--%%HEAD%%-->";
    static const std::string contentTag = "<!--%%CONTENT%%-->";
    static const std::string searchformTag = "<!--%%SEARCH%%-->";
    static const std::string searchform = "<form action='search'><input type='text' name='search' size='20'><br><input type='submit' name='getpage' value='Artikel'><input type='submit' name='dosearch' value='Volltext'></form>\n";
  }

  Skin::Skin(const std::string& skin, const std::string& host_)
    : std::ostream(data.rdbuf()),
      host(host_)
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

  std::string Skin::getData() const
  {
    std::string ret = skindata;
    std::string headdata = "<base href='http://" + host + "/'></base>";

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
      ret.replace(pos, hostTag.size(), host);
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

  std::ostream& operator<< (std::ostream& out, const Skin& skin)
  {
    const std::string& str = skin.skindata;
    std::string::size_type pos = 0;
    std::string::size_type hostPos = str.find(hostTag);
    std::string::size_type headPos = str.find(headTag);
    std::string::size_type contentPos = str.find(contentTag);
    std::string::size_type searchformPos = str.find(searchformTag);

    while (pos < str.size())
    {
      std::string::size_type nextPos = str.size();
      if (hostPos != std::string::npos && hostPos < nextPos)
        nextPos = hostPos;
      if (headPos != std::string::npos && headPos < nextPos)
        nextPos = headPos;
      if (contentPos != std::string::npos && contentPos < nextPos)
        nextPos = contentPos;
      if (searchformPos != std::string::npos && searchformPos < nextPos)
        nextPos = searchformPos;

      std::copy(str.begin() + pos, str.begin() + nextPos, std::ostreambuf_iterator<char>(out));

      if (nextPos == hostPos)
      {
        out << skin.host;
        pos = nextPos + hostTag.size();
        hostPos = str.find(hostTag, pos);
      }
      else if (nextPos == headPos)
      {
        out << "<base href='http://" << skin.host << "/'></base>";
        pos = nextPos + headTag.size();
        headPos = str.find(headTag, pos);
      }
      else if (nextPos == contentPos)
      {
        out << skin.data.str();
        pos = nextPos + contentTag.size();
        contentPos = str.find(contentTag, pos);
      }
      else if (nextPos == searchformPos)
      {
        out << searchform;
        pos = nextPos + searchformTag.size();
        searchformPos = str.find(searchformTag, pos);
      }
      else
        pos = nextPos;
    }

    return out;
  }
}

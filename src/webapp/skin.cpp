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
#include <cxxtools/log.h>

log_define("zenoreader.skin")

namespace zenoreader
{
  Skin::Skin(const std::string& skin, const std::string& host, const std::string& content)
  {
    log_debug("read skinfile \"skin/" << skin << '"');

    std::ifstream in(("skin/" + skin).c_str());
    std::copy(std::istreambuf_iterator<char>(in),
              std::istreambuf_iterator<char>(),
              std::back_inserter(data));

    log_debug("skinfile has " << data.size() << " bytes");

    static const std::string headTag = "<!--%%HEAD%%-->";
    static const std::string contentTag = "<!--%%CONTENT%%-->";
    static const std::string searchformTag = "<!--%%SEARCH%%-->";

    std::string headdata = "<base href='http://" + host + "/'></base>";
    std::string searchform = "<form action='search'><input type='text' name='search' size='20'><br><input type='submit' name='getpage' value='Artikel'><input type='submit' name='dosearch' value='Volltext'></form>\n";

    std::string::size_type pos = 0;
    log_debug("replace head");
    while ((pos = data.find(headTag, pos)) != std::string::npos)
    {
      log_debug("head found at pos " << pos);
      data.replace(pos, headTag.size(), headdata);
      pos += headdata.size();
    }

    pos = 0;
    log_debug("replace content");
    while ((pos = data.find(contentTag, pos)) != std::string::npos)
    {
      log_debug("content found at pos " << pos);
      data.replace(pos, contentTag.size(), content);
      pos += headdata.size();
    }

    pos = 0;
    log_debug("replace searchform");
    while ((pos = data.find(searchformTag, pos)) != std::string::npos)
    {
      log_debug("searchform found at pos " << pos);
      data.replace(pos, searchformTag.size(), searchform);
      pos += headdata.size();
    }

    log_debug("data has " << data.size() << " bytes");
  }
}

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

#include <zeno/indexarticle.h>
#include <zeno/zintstream.h>
#include <sstream>
#include <stdexcept>
#include <cxxtools/log.h>

log_define("zeno.indexarticle")

namespace zeno
{
  void IndexArticle::readEntries()
  {
    if (!*this || categoriesRead)
      return;

    log_debug("read entries for article " << getUrl());

    std::istringstream s(getParameter());
    s.get();  // skip length byte
    zeno::ZIntStream extra(s);

    unsigned flagfield;  // field with one bit (bits 0-3) for each cateogry
    extra.get(flagfield);

    log_debug("flags: h" << std::hex << flagfield);

    unsigned offset = 0;
    for (unsigned c = 0; c <= 3; ++c)
    {
      bool catNotEmpty = (flagfield & 1);
      flagfield >>= 1;

      if (catNotEmpty)
      {
        log_debug("read category " << c);

        unsigned len;
        Entry entry;
        bool s = extra.get(len) && extra.get(entry.index);
        if (s && getNamespace() == 'X')
          s = extra.get(entry.pos);
        else
          entry.pos = 0;

        unsigned pos = entry.pos;

        if (!s)
          throw std::runtime_error("invalid index entry");

        log_debug("first index " << entry.index << " pos " << entry.pos);

        log_debug("read data from offset " << offset << " len " << len);
        std::istringstream data(getData().substr(offset, len));
        ZIntStream zdata(data);

        while (zdata.get(entry.index))
        {
          if (getNamespace() == 'X')
          {
            unsigned p;
            if (!zdata.get(p))
              throw std::runtime_error("invalid index entry");
            pos += p;
            entry.pos = p;
          }
          else
            entry.pos = 0;

          log_debug("index " << entry.index << " pos " << entry.pos);

          entries[c].push_back(entry);
        }

        offset += len;
      }
    }
  }

}

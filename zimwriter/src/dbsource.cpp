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

#include <zim/writer/dbsource.h>
#include <zim/blob.h>
#include <tntdb/connect.h>
#include <tntdb/error.h>
#include <cxxtools/arg.h>
#include <cxxtools/log.h>
#include <limits>

log_define("zim.writer.dbsource")

namespace zim
{
  namespace writer
  {
    std::string DbArticle::getAid() const
    {
      log_debug("getAid");
      return row[0].getString();
    }

    char DbArticle::getNamespace() const
    {
      log_debug("getNamespace");
      return row[1].getChar();
    }

    std::string DbArticle::getTitle() const
    {
      log_debug("getTitle");
      return row[2].getString();
    }

    bool DbArticle::isRedirect() const
    {
      log_debug("isRedirect");
      return !row[4].isNull();
    }

    MimeType DbArticle::getMimeType() const
    {
      log_debug("getMimeType");
      return static_cast<MimeType>(row[3].getInt());
    }

    std::string DbArticle::getRedirectAid() const
    {
      log_debug("getRedirectAid");
      return row[4].getString();
    }

    DbSource::DbSource(int& argc, char* argv[])
      : initialized(false)
    {
      cxxtools::Arg<const char*> dburl(argc, argv, "--db", "postgresql:dbname=zim");

      conn = tntdb::connect(dburl.getValue());

      selData = conn.prepare("select data from article where aid = :aid");
    }

    void DbSource::setFilename(const std::string& fname)
    {
      tntdb::Statement s = conn.prepare("select zid from zimfile where filename = :filename");
      tntdb::Value v = s.set("filename", fname).selectValue();
      unsigned zid = v.getUnsigned();

      stmt = conn.prepare(
        "select a.aid, a.namespace, a.title, a.mimetype, r.aid"
        "  from article a"
        "  join zimarticles z"
        "    on a.aid = z.aid"
        "  left outer join article r"
        "    on a.redirect = r.title"
        " where z.zid = :zid"
        "   and (a.redirect is null"
        "          or r.aid is not null)");
      stmt.set("zid", zid);

      current = stmt.end();
    }

    const Article* DbSource::getNextArticle()
    {
      log_debug("getNextArticle");

      if (initialized)
      {
        log_debug("fetch next");
        ++current;
      }
      else
      {
        log_debug("initalize cursor");
        current = stmt.begin();
        initialized = true;
      }

      if (current == stmt.end())
      {
        log_debug("the end");
        return 0;
      }

      article = DbArticle(*current);

      return &article;
    }

    Blob DbSource::getData(const std::string& aid)
    {
      dataValue = selData.set("aid", aid)
                         .selectValue();
      dataValue.getBlob(dataBlob);
      return Blob(dataBlob.data(), dataBlob.size());
    }

    Uuid DbSource::getUuid()
    {
      return Uuid::generate();
    }

    unsigned DbSource::getMainPage()
    {
      return std::numeric_limits<unsigned>::max();
    }

    unsigned DbSource::getLayoutPage()
    {
      return std::numeric_limits<unsigned>::max();
    }

  }

}

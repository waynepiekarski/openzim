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

#ifndef ZENO_QUNICODE_H
#define ZENO_QUNICODE_H

#include <string>

namespace zeno
{
  class QUnicodeChar
  {
      int value;

    public:
      QUnicodeChar(char ch)
        : value(ch)
        { }
      QUnicodeChar(int ch = 0)
        : value(ch)
        { }
      int getValue() const  { return value; }
      unsigned char getCollateValue() const;

      bool operator< (const QUnicodeChar& v) const
      { return getCollateValue() < v.getCollateValue(); }

      bool operator== (const QUnicodeChar& v) const
      { return value == v.value; }
  };

  std::istream& operator>> (std::istream& in, QUnicodeChar& qc);

  class QUnicodeString
  {
      std::string value;

    public:
      QUnicodeString(const std::string& v)
        : value(v)
        { }

      const std::string& getValue() const { return value; }

      int compare(const QUnicodeString& v) const;

      bool operator< (const QUnicodeString& v) const
        { return compare(v) < 0; }
      bool operator== (const QUnicodeString& v) const
        { return value == v.value; }
      bool operator> (const QUnicodeString& v) const
        { return v < *this; }
      bool operator!= (const QUnicodeString& v) const
        { return value != v.value; }
      bool operator<= (const QUnicodeString& v) const
        { !(*this > v); }
      bool operator>= (const QUnicodeString& v) const
        { !(*this < v); }

      std::string toXML() const;
  };
}

#endif // ZENO_QUNICODE_H


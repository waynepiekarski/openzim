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

#ifndef ZIM_WRITER_MSTREAM_H
#define ZIM_WRITER_MSTREAM_H

#include <iostream>
#include <fstream>
#include <map>
#include <string>

namespace zim
{
  namespace writer
  {
    class MStream
    {
        std::fstream tmpfile;

        class Stream
        {
            std::fstream* backfile;

            static const std::streampos npos;
            static const unsigned buffersize = 120;

            std::streampos first;
            std::streampos last;  // when reading pointer to last offset written to backfile
                                  // when writing pointer to next offset to read from backfile
            char data[buffersize];
            unsigned putpos;
            unsigned getpos;

            void overflow();
            bool underflow();   // return false on eof

          public:
            Stream() {}
            explicit Stream(std::fstream& backfile_)
              : backfile(&backfile_),
                first(npos),
                last(npos),
                putpos(0)
              { }

            void put(char ch)
            {
              if (putpos >= buffersize)
              {
                overflow();
                putpos = 0;
              }
              data[putpos++] = ch;
            }

            void write(const char* data, unsigned size)
            {
              for (unsigned n = 0; n < size; ++n)
                put(data[n]);
            }

            void setRead();

            bool get(char& ch);   // return false on eof

            void read(std::string& s)
            {
              char ch;
              while (get(ch))
                s += ch;
            }

            bool read(char* data, unsigned size)
            {
              for (unsigned n = 0; n < size; ++n)
                if (!get(data[n]))
                  return false;
              return true;
            }
        };

        typedef std::map<std::string, Stream> StreamMap;
        StreamMap streams;
        std::fstream backfile;

      public:
        explicit MStream(const char* backfilename)
          : backfile(backfilename, std::ios::in | std::ios::out | std::ios::trunc)
          { }

        typedef StreamMap::const_iterator const_iterator;
        typedef StreamMap::iterator iterator;

        typedef StreamMap::value_type value_type;

        const_iterator begin() const   { return streams.begin(); }
        const_iterator end() const     { return streams.end(); }

        iterator begin()         { return streams.begin(); }
        iterator end()           { return streams.end(); }

        void write(const std::string& streamname, const char* ptr, unsigned size);
        void write(const std::string& streamname, const std::string& str)
          { write(streamname, str.data(), str.size()); }

        void setRead();

        iterator find(const std::string& streamname)
          { return streams.find(streamname); }
    };

  }
}

#endif //  ZIM_WRITER_MSTREAM_H

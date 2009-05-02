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

#include "zim/writer/mstream.h"
#include <stdexcept>

namespace zim
{
  namespace writer
  {
    const std::streampos MStream::Stream::npos = -1;
    const unsigned MStream::Stream::buffersize;

    void MStream::Stream::overflow()
    {
      if (!backfile)
        throw std::logic_error("no backfile");

      backfile->seekp(0, std::ios::end);
      std::streampos next = backfile->tellp();

      if (last != npos)
      {
        backfile->seekp(last);
        backfile->write(reinterpret_cast<const char*>(&next), sizeof(next));
        backfile->seekp(0, std::ios::end);
      }

      backfile->write(reinterpret_cast<const char*>(&npos), sizeof(npos));
      backfile->write(data, buffersize);

      last = next;

      if (first == npos)
        first = last;
    }

    bool MStream::Stream::underflow()
    {
      backfile->seekg(last);
      backfile->read(reinterpret_cast<char*>(&last), sizeof(last));
      backfile->read(data, buffersize);
      getpos = 0;
      return *backfile;
    }

    void MStream::Stream::setRead()
    {
      if (first != npos)
      {
        if (putpos > 0)
          overflow();

        last = first;
        underflow();
      }

      getpos = 0;
    }

    bool MStream::Stream::get(char& ch)
    {
      if (last == npos)
      {
        // we are in the last block
        if (getpos >= putpos)
          return false;
        ch = data[getpos++];
        return true;
      }
      else
      {
        if (getpos >= buffersize && !underflow())
          return false;

        if (last == npos && getpos >= putpos)
          return false;

        ch = data[getpos++];
        return true;
      }
    }

    //////////////////////////////////////////////////////////////////////
    // MStream
    //
    void MStream::write(const std::string& streamname, const char* ptr, unsigned size)
    {
      StreamMap::iterator it = streams.find(streamname);
      if (it == streams.end())
        it = streams.insert(StreamMap::value_type(streamname, Stream(backfile))).first;
      it->second.write(ptr, size);
    }

    void MStream::setRead()
    {
      for (StreamMap::iterator it = streams.begin(); it != streams.end(); ++it)
        it->second.setRead();
    }

  }
}

//
// $Id: fstream.h,v 1.1 2004/02/27 13:32:59 mjuric Exp $ 
//  
//  gzstream::iostream
//  Copyright (C) 2002 Christian Holm Christensen <cholm@nbi.dk> 
//
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public License 
//  as published by the Free Software Foundation; either version 2.1 
//  of the License, or (at your option) any later version. 
//
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free 
//  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 
//  02111-1307 USA 
//

#include "config.h"

#ifndef GZSTREAM_iostream
#define GZSTREAM_iostream
#ifndef GZSTREAM_streambuf
#include <astro/io/gzstream/streambuf.h>
#endif
#ifndef __IOSTREAM__
#include <iostream>
#endif

/** @file   iostream.h
    @author Christian Holm
    @date   Wed Oct  9 04:11:25 2002
    @brief  I/O streams on zlib files */

namespace peyton {
namespace io {
namespace gzstream 
{
  //==================================================================
  /** @class basic_iostream iostream.h <gzstream/iostream.h> 
      @brief Basic stream using a zlib gzFile streambuffer 
      @param Stream_Type the kind of stream to derive from 
      @ingroup gzstream
  */
  template <typename Stream_Type, typename Streambuf, std::ios_base::openmode mode = std::ios::in|std::ios::out>
  class basic_iostream : public Stream_Type
  {
  public:
    /// The stream type 
    typedef Stream_Type stream_type;
    typedef Streambuf streambuf_type;
  private:
    streambuf_type _buf;
  public: 
    /// Create new stream
    basic_iostream() : stream_type(&_buf), _buf(0) {}
    /** Create new stream from a pointer a zlib @c gzFile
	@param fp the zlib @c gzFile pointer. */
    basic_iostream(gzFile fp) : stream_type(&_buf), _buf(fp) {}
    /** Create new stream from a filename
	@param fn the filename. */
    basic_iostream(const char *fn) : stream_type(&_buf), _buf(fn, mode) {}
    /** Open new stream from a filename
    @param fn the filename. */
    void open(const char* fn, std::ios_base::openmode _mode = mode)
    {
	_buf.open(fn, _mode);
    }
    /** Get the streambuffer.
	@return A pointer to the streambuf object */
    streambuf_type* rdbuf() const { return &_buf; }
    /** Get a pointer to the zlib @c gzFile object.
	@return A Pointer to the zlib @c gzFile object. */
    operator gzFile () { return _buf; }
  };

  //==================================================================
  /** @brief zlib input stream. 
      @ingroup gzstream
  */
  typedef basic_iostream<std::istream, streambuf, std::ios::in> ifstream;
  /** @brief zlib output stream. 
      @ingroup gzstream
  */
  typedef basic_iostream<std::ostream, streambuf, std::ios::out> ofstream;
  /** @brief zlib wide-character input stream. 
      @ingroup gzstream
  */
#ifdef GZSTREAM_WHCAR_SUPPORT
  typedef basic_iostream<std::wistream, wstreambuf, std::ios::in> wifstream;
  /** @brief zlib wide-character output stream.
      @ingroup gzstream
  */
  typedef basic_iostream<std::wostream, wstreambuf, std::ios::out> wofstream;
#endif
}
}
}

#define __peyton_io peyton::io

#endif

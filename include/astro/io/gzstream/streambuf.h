//
// $Id: streambuf.h,v 1.2 2006/04/09 15:08:01 mjuric Exp $ 
//  
//  gzstream::streambuf
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

#ifndef GZSTREAM_streambuf
#define GZSTREAM_streambuf
#ifndef GZSTREAM_file
# include <astro/io/gzstream/file.h>
#endif
#ifndef GZSTREAM_compat
# include <astro/io/gzstream/compat.h>
#endif
#ifndef __IOSTREAM__
# include <iostream>
#endif

/** @file   streambuf.h
    @author Christian Holm
    @date   Wed Oct  9 04:06:43 2002
    @brief  Stream buffer over an zlib gzFile */

namespace peyton {
namespace io {
namespace gzstream 
{
  //==================================================================
  /** Basic zlib gzFile stream buffer. 
      @param T The kind of streambuf to derive from.  Should be a
      sub-class of std::streambuf
      @ingroup gzstream
  */
  template <typename T>
  class basic_streambuf : public T
  {
  public:
    /// The streambuf type we're deriving from 
    typedef T                                    streambuf_type;
    /// The character type 
    typedef typename streambuf_type::char_type   char_type;
    /// The integer type 
    typedef typename streambuf_type::int_type    int_type;
    /// The type of traits 
    typedef typename streambuf_type::traits_type traits_type;
    /// The underlying C file interface type 
    typedef basic_file<char_type, traits_type>   file_type;
    /// The position type 
    typedef typename traits_type::pos_type       pos_type;
    /// The offset type
    typedef typename traits_type::off_type       off_type;
  private:
    /// The underlying zlib gzFile abstraction layer. 
    file_type _file;
  protected:
    /** @name std::basic_streambuf interface. 
	Forwards call to zlib gzFile abstraction layer. */ 
    /*@{*/
    /** write a character to output. 
	@param c The character to write. 
	@return EOF on failure or @p c on success. */
    int_type overflow(int_type c=EOF) { return _file.overflow(c); }
    /** Read the pending character from the stream and put it back. 
	@return the next pending input. */
    int_type underflow() { return _file.underflow(); }
    /** Read the pending character from the stream. 
	@return the next pending input. */
    int_type uflow() { return _file.uflow(); }
    /** Put back one character to the input stream. 
	@param c the character to put back. 
	@return EOF on failure, otherwise @p c */
    int_type pbackfail(int_type c=EOF) { return _file.pbackfail(c); }
    /** Read a block of characters from the file. 
	@param buf Buffer to read into. 
	@param n Size of the buffer to fill. 
	@return # of characters read. */
    std::streamsize xsgetn(char_type* buf, std::streamsize n);
    /** Put back a block of characters to the file. 
	@param buf Buffer to read from. 
	@param n Size of the buffer to write out. 
	@return # of characters writen. */
    std::streamsize xsputn(char_type* buf, std::streamsize n);
    /** Syncronise the stream. 
	@return EOF on error. */
    int sync() { return _file.sync(); }
    /** Seek to an offset in the file in the stream. 
	@param pos The position to seek to. 
	@param dir The direction to seek in. 
	@param mode The open mode. 
	@return new offset in the file. */
    pos_type seekoff(off_type pos, 
		     std::ios_base::seekdir dir,
		     std::ios_base::openmode mode=
		     std::ios::in|std::ios::out);
    /** Seek to a position in the stream. 
	@param pos The position to seek to. 
	@param mode The open mode. 
	@return new offset in the file. */
    pos_type seekpos(off_type pos, 
		     std::ios_base::openmode mode=
		     std::ios::in|std::ios::out);
    
    /*@}*/
  public:
    /** Constructor. */
    basic_streambuf() : streambuf_type() { this->setbuf(0, 0); };
    /** Constrcutor with an explicit zlib gzFile argument. 
	This constructor is non-standard. 
	@param fp A zlib gzFile pointer. */
    basic_streambuf(gzFile fp) : streambuf_type(), _file(fp) { this->setbuf(0, 0); }
    /** Constrcutor with a gzipped filename argument. 
	This constructor is non-standard. 
	@param fn A zlib gzipped file filename. */
    basic_streambuf(const char *fn, std::ios_base::openmode mode) : streambuf_type(), _file(0) { this->setbuf(0, 0); open(fn, mode); }
    /** Destructor. */
    virtual ~basic_streambuf() {}
    /** @name std::basic_filebuf interface. 
	Forwards call to zlib gzFile abstraction layer. */ 
    /*@{*/
    /** Opens a file. 
	@param name the file to open. 
	@param mode the mode to open the file in. 
	@return a pointer to self. */
    streambuf_type* open(const char* name, std::ios_base::openmode mode);
    /** Closes a file.
	@return a pointer to self. */
    streambuf_type* close();
    /** Test if file is open. 
	@return true if open, false otherwise. */
    bool is_open() const { return _file.is_open(); }
    /*@}*/
    /** Conversion operator. 
	@return self as a pointer to zlib @c gzFile object. */
    operator gzFile () { return _file._file; }
  };

  //__________________________________________________________________
  template<typename T>
  std::streamsize 
  basic_streambuf<T>::xsgetn(basic_streambuf<T>::char_type* buf, 
			      std::streamsize n)
  {
    return _file.xsgetn(buf, n);
  }

  //__________________________________________________________________
  template <typename T>
  std::streamsize 
  basic_streambuf<T>::xsputn(basic_streambuf<T>::char_type* buf, 
			   std::streamsize n)
  {
    return _file.xsgetn(buf, n);
  }

  //__________________________________________________________________
  template <typename T>
  typename basic_streambuf<T>::pos_type 
  basic_streambuf<T>::seekoff(basic_streambuf<T>::off_type pos, 
			       std::ios_base::seekdir dir,
			       std::ios_base::openmode mode) 
  {
    return _file.seekoff(pos, dir, mode);
  }
  
  //__________________________________________________________________
  template <typename T>
  typename basic_streambuf<T>::pos_type 
  basic_streambuf<T>::seekpos(basic_streambuf<T>::off_type pos, 
			 std::ios_base::openmode mode) 
  {
    return _file.seekpos(pos, mode);
  }
  

  //__________________________________________________________________
  template <typename T>
  typename basic_streambuf<T>::streambuf_type*
  basic_streambuf<T>::open(const char* name, 
			    std::ios_base::openmode mode) 
  {
    return _file.open(name, mode) ? this : 0;
  }
  
  //__________________________________________________________________
  template <typename T>
  typename basic_streambuf<T>::streambuf_type*
  basic_streambuf<T>::close()
  {
    return _file.close() ? this : 0; 
  }
  
  //==================================================================
  /** zlib gzFile normal stream buffer. 
      @ingroup gzstream
  */
#ifdef GZSTREAM_NEED_STREAMBUF_TYPES
  typedef basic_streambuf<compatibility::streambuf> streambuf;
#else 
  typedef basic_streambuf<std::streambuf> streambuf;
#endif
#ifdef GZSTREAM_HAVE_WCHAR_SUPPORT
  /** zlib gzFile wide-character stream buffer.
      @ingroup gzstream
  */ 
  typedef basic_streambuf<std::wstreambuf> wstreambuf;
#endif
}
}
}

#endif
//____________________________________________________________________
//
// EOF
//



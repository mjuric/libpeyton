//
// $Id: file.h,v 1.2 2004/02/27 13:52:31 mjuric Exp $ 
//  
//  gzstream::c_file
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

#include "./config.h"

#ifndef GZSTREAM_file
#define GZSTREAM_file
#ifndef __CSTDIO__
#include <zlib.h>
#endif
#ifndef GZSTREAM_compat
# include <astro/io/gzstream/compat.h>
#endif

/** @file   file.h
    @author Christian Holm
    @date   Tue Oct  8 12:57:03 2002
    @brief  zlib gzFile file buffer abstraction layer. */

/** @brief zlib gzFile interface via stream buffers. 
    This module of classes defines an abstraction layer over zlib
    gzFile. */

namespace peyton {
namespace io {
namespace gzstream
{

  /** @class basic_file file.h <gzstream/file.h>
      @brief zlib gzFile file buffer abstraction layer. 
      @param Char_Type the character type to use 
      (one of @c char or @c wchar_t )
      @param Traits The character traits to use.  Per default this is
      std::char_traits <tt>&lt;Char_Type&gt;</tt>
      @ingroup gzstream
  */
  template <typename Char_Type, typename Traits=std::char_traits<Char_Type> >
  class basic_file
  {
  protected:
    /// Buffer for simulating unget calls - zlib does not support unget
    char _ungetbuf[1];
    /// Is the unget buffer being used?
    bool _ungetfull;
    int gzungetc(int c, gzFile stream) {
       if(_ungetfull) { return EOF; }
       _ungetbuf[0] = (char)c;
       return c;
    }
    int Xgetc (gzFile file) {
      if(!_ungetfull) { return gzgetc(file); }

      _ungetfull = false;
      return _ungetbuf[0];
    }
    int Xread (gzFile file, voidp buf, unsigned len) {
      if(!_ungetfull || len == 0) { return gzread(file, buf, len); }

      ((char *)buf)[0] = _ungetbuf[0];
      len--; ((char *&)buf)++;
      int ret = gzread(file, buf, len);

      if(ret < 0) return 0; // in case of error, return 0

      _ungetfull = false;
      return ret + 1; // number of bytes read
    }
    z_off_t Xseek (gzFile file, z_off_t offset, int whence)
    {
      if(!_ungetfull || whence != SEEK_CUR) { return gzseek(file, offset, whence); }

      offset -= 1;
      z_off_t off = gzseek(file, offset, SEEK_CUR);
      if(off == -1) return -1;

      _ungetfull = false;
      return off - 1;
    }
    z_off_t Xtell (gzFile file)
    {
      if(!_ungetfull) { return gztell(file); }
      return gztell(file) - 1;
    }

  public:
    /// The type of the buffers 
    typedef Char_Type char_type;
    /// The type of the traits 
    typedef Traits traits_type;
    /// Integer type
    typedef typename traits_type::int_type  int_type;
    /// The position type 
    typedef typename traits_type::pos_type  pos_type;
    /// The offset type 
    typedef typename traits_type::off_type  off_type;
    /// Pointer to C gzFile abstraction layer.
    gzFile _file;
    /// Did we create the file? 
    bool _created;
    /** write a character to output.
	@param c The character to write. 
	@return EOF on failure or @p c on success. */
    int_type overflow(int_type c=EOF);
    /** Read the pending character from the stream and put it back. 
	@return the next pending input. */
    int_type underflow();
    /** Read the pending character from the stream. 
	@return the next pending input. */
    int_type uflow();
    /** Put back one character to the input stream.
	@param c the character to put back.
	@return EOF on failure, otherwise @p c */
    int_type pbackfail(int_type c=EOF);
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
    int sync();
    /** Seek to an offset in the file in the stream. 
	@param pos The position to seek to. 
	@param dir The direction to seek in.
	@param mode The open mode. 
	@return new offset in the file. */
    pos_type seekoff(off_type pos, 
		     std::ios::seekdir dir,
		     std::ios::openmode mode=std::ios::in|std::ios::out);
    /** Seek to a position in the stream. 
	@param pos The position to seek to. 
	@param mode The open mode. 
	@return new offset in the file. */
    pos_type seekpos(pos_type pos, 
		     std::ios::openmode mode=std::ios::in|std::ios::out);
    /** Default constructor. */
    basic_file();
    /** Construct from zlib gzFile pointer (not standard). 
	@param fp zlib gzFile pointer. */
    basic_file(gzFile fp);
    /** Destructor. */
    virtual ~basic_file();
    /** open a file. 
	@param name the file name to open. 
	@param mode What mode to open in. 
	@return A pointer to this if succesful, otherwise 0. */
    basic_file* open(const char* name, std::ios_base::openmode mode);
    /** Close the file.
	@return A pointer to this if succesful, otherwise 0. */
    basic_file* close();
    /** Test if file is open. 
	@return true if file is open, otherwise false. */    
    bool is_open() const { return _file != 0; }
    /** Conversion operator. */
    operator gzFile () { return _file; }
  };

  //__________________________________________________________________
  inline basic_file<char>::int_type
  basic_file<char>::overflow(basic_file::int_type c)
  {
    return c != EOF ? gzputc(_file, c) : EOF;
  }
  //__________________________________________________________________
  inline basic_file<char>::int_type
  basic_file<char>::underflow()
  {
    int c = Xgetc(_file);
    if (c != EOF) gzungetc(c, _file);
    return c;
  }
  //__________________________________________________________________
  inline basic_file<char>::int_type
  basic_file<char>::uflow()
  {
    return Xgetc(_file);
  }
  //__________________________________________________________________
  inline basic_file<char>::int_type
  basic_file<char>::pbackfail(basic_file::int_type c)
  {
    return c != EOF ? gzungetc(c, _file) : EOF;
  }

#ifdef GZSTREAM_HAVE_WCHAR_SUPPORT
  //__________________________________________________________________
  basic_file<wchar_t>::int_type
  basic_file<wchar_t>::overflow(basic_file::int_type c)
  {
    return c != WEOF ? fputwc(c, _file) : WEOF;
  }
  //__________________________________________________________________
  basic_file<wchar_t>::int_type
  basic_file<wchar_t>::underflow() 
  {
    int c = getwc(_file);
    if (c != WEOF) gzungetwc(c, _file);
    return c;
  }
  //__________________________________________________________________
  basic_file<wchar_t>::int_type
  basic_file<wchar_t>::uflow()
  {
    return fgetwc(_file);
  }
  //__________________________________________________________________
  basic_file<wchar_t>::int_type
  basic_file<wchar_t>::pbackfail(basic_file::int_type c) 
  {
    return c != WEOF ? gzungetwc(c, _file) : WEOF;
  }
#endif

  //__________________________________________________________________
  template <typename Char_Type, typename Traits>
  int
  basic_file<Char_Type, Traits>::sync() 
  {
    return gzflush(_file, Z_SYNC_FLUSH);
  }


  //__________________________________________________________________
  template <typename Char_Type, typename Traits>
  std::streamsize 
  basic_file<Char_Type, Traits>::xsgetn(basic_file::char_type* buf, 
					std::streamsize n) 
  {
    return Xread(_file, buf, sizeof(char_type) * n);
  }

  //__________________________________________________________________
  template <typename Char_Type, typename Traits>
  std::streamsize 
  basic_file<Char_Type, Traits>::xsputn(basic_file::char_type* buf, 
					std::streamsize n) 
  {
    return gzwrite(_file, buf, sizeof(char_type) * n);
  }

  //__________________________________________________________________
  template <typename Char_Type, typename Traits>
  typename basic_file<Char_Type, Traits>::pos_type
  basic_file<Char_Type, Traits>::seekoff(basic_file<Char_Type, 
					            Traits>::off_type off, 
					 std::ios::seekdir dir, 
					 std::ios::openmode)
  {
    Xseek(_file, off, dir); 
    return Xtell(_file);
  }

  //__________________________________________________________________
  template <typename Char_Type, typename Traits>
  typename basic_file<Char_Type, Traits>::pos_type
  basic_file<Char_Type, Traits>::seekpos(basic_file<Char_Type, 
					            Traits>::pos_type pos, 
					 std::ios::openmode) 
  {
    Xseek(_file, pos, std::ios::beg); 
    return Xtell(_file);
  }
  
  //__________________________________________________________________
  template <typename Char_Type, typename Traits>
  basic_file<Char_Type, Traits>::basic_file()
    : _file(NULL), _created(false), _ungetfull(false)
  {}

  //__________________________________________________________________
  template <typename Char_Type, typename Traits>
  basic_file<Char_Type, Traits>::basic_file(gzFile fp)
    : _file(fp), _created(false), _ungetfull(false)
  {}

  //__________________________________________________________________
  template <typename Char_Type, typename Traits>
  basic_file<Char_Type, Traits>::~basic_file()
  {
    if (!is_open())
      return;
    // gzflush(_file, Z_SYNC_FLUSH); // should not be necessary, because gzclose will do the flushing
    close();
  }

  //__________________________________________________________________
  template <typename Char_Type, typename Traits>
  basic_file<Char_Type, Traits>* 
  basic_file<Char_Type, Traits>::open(const char* name,
				      std::ios_base::openmode mode) 
  {
    if (is_open()) return 0;

    bool b = mode & std::ios::binary;
    bool i = mode & std::ios::in;
    bool o = mode & std::ios::out;
    bool t = mode & std::ios::trunc;
    bool a = mode & std::ios::app;
    char rw[4] = { '\0', '\0', '\0', '\0' };

    if (!i && o  && !t && !a)   rw[0] = 'w';
    if (!i && o  && !t && a)    rw[0] = 'a';
    if (!i && o  && t  && !a)   rw[0] = 'w';
    if (i  && !o && !t && !a)   rw[0] = 'r';
    if (i  && o  && !t && !a) { rw[0] = 'r'; rw[1] = '+'; }
    if (i  && o  && t  && a)  { rw[0] = 'w'; rw[1] = '+'; }
    if (b)                      rw[0] = 'b';

    if (!(_file = gzopen(name, rw))) return 0;
  
    _created = true; 
    return this; 
  }

  //__________________________________________________________________
  template <typename Char_Type, typename Traits>
  basic_file<Char_Type, Traits>* 
  basic_file<Char_Type, Traits>::close() 
  {
    if (!_created || !gzclose(_file))
      return 0;
    return this;
  }

}
}
}

#endif
//____________________________________________________________________
//
// EOF
//

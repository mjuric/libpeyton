// 
// $Id: compat.h,v 1.1 2004/02/27 13:32:59 mjuric Exp $
//

#include "config.h"

#ifndef GZSTREAM_compat
#define GZSTREAM_compat

/** @file   compat.h
    @author Christian Holm
    @date   Fri Jun 20 22:29:05 2003
    @brief  Compatibility definitions
*/

#if defined(__GNUC__) && __GNUC__ < 3
# define ios_base ios
#else
# ifndef __IOS__
#  include <ios>
# endif
#endif

#ifdef GZSTREAM_NEED_STREAMBUF_H
# ifndef _STREAMBUF_H
#  include <streambuf.h>
# endif
#else
# ifndef __STREAMBUF__
#  include <streambuf>
# endif
#endif

/** @defgroup compat Compatibility classes */

#if defined(GZSTREAM_NEED_CHAR_TRAITS) || defined(GZSTREAM_NEED_WCHAR_TRAITS)
# ifndef __CWCHAR__
#  include <cwchar>
# endif

namespace std 
{
  /** @class char_traits compat.h <gzstream/compat.h> 
      @brief Template for character traits. 
      Some older compilers does not define this template. Hence, we
      supply it here if GZSTREAM_NEED_CHAR_TRAITS or
      GZSTREAM_NEED_WCHAR_TRAITS are defined.  The implementation is
      from GCC 3.2
      @param Char_Type The character type
      @ingroup compat 
      @see char_traits<char>
      @see char_traits<wchar_t>
  */
  template<class Char_Type>
  struct char_traits
  {
    typedef Char_Type     char_type;  /** The character type */
    typedef unsigned long int_type;   /** Integer type */
    typedef streampos     pos_type;   /** Position type */
    typedef streamoff 	  off_type;   /** The offset type  */
    typedef mbstate_t 	  state_type; /** The state type   */

    /** Assign one character to the value of another 
	@param c1 Character to assign to
	@param c2 Character value to assign from */
    static void assign(char_type& c1, const char_type& c2);
    /** Compare two characters for equility
	@param c1 left hand side 
	@param c2 right hand side
	@return true if @a c1 and @a c2 are equal */
    static bool eq(const char_type& c1, const char_type& c2);
    /** Lexical compare two characters
	@param c1 left hand side 
	@param c2 right hand side 
	@return true if @a c1 lexically comes before @a c2 */
    static bool lt(const char_type& c1, const char_type& c2);
    /** Compare lexically two character strings
	@param s1 left hand side
	@param s2 right hand side
	@param n number of characters to compare 
	@return 0 if @a s1 and @a s2 are equal, <0 if @a s1 is smaller
	than @a s2, or >0 otherwise */
    static int compare(const char_type* s1, const char_type* s2, size_t n); 
    /** Get the lenght of a character string 
	@param s The string 
	@return The length (position of terminating '@\0') of @a s */
    static size_t length(const char_type* s);
    /** Find position of a character in a character string
	@param s The string to search
	@param n The maximum number of characters in @a s to look at
	@param a The character to search for 
	@return pointer to @a a if found, otherwise 0 */
    static const char_type* find(const char_type* s, size_t n, 
				 const char_type& a);
    /** Copy one character string to another.  Strings may overlap. 
	@param s1 The character string to copy to
	@param s2 The character string to copy from
	@param n  The number of characters to copy
	@return @a s1 */
    static char_type* move(char_type* s1, const char_type* s2, size_t n);
    /** Copy one character string to another.  Strings may @e not overlap. 
	@param s1 The character string to copy to
	@param s2 The character string to copy from
	@param n  The number of characters to copy
	@return @a s1 */
    static char_type* copy(char_type* s1, const char_type* s2, size_t n);
    /** Set all characters of a character string to one character value
	@param s The string to set
	@param n The number of characters in the string to set
	@param a The character value to set @a s to
	@return @a s */
    static char_type* assign(char_type* s, size_t n, char_type a);
    /** Turn an integer into a character 
	@param c The integer to convert
	@return @a c as a character */
    static char_type to_char_type(const int_type& c);
    /** Turn a character into an integer 
	@param c The character to convert
	@return @a c as a character */
    static int_type to_int_type(const char_type& c);
    /** Compare two integers for equality
	@param c1 left hand side
	@param c2 Right hand side
	@return true iff @f$ c1 = c2@f$ */
    static bool eq_int_type(const int_type& c1, const int_type& c2);
    /** Get the End-Of-File marker 
	@return End-Of-File marker */
    static int_type eof();
    /** Test if a character is the End-Of-File marker
	@param c Character to test
	@return 0 if @f$ c = eof()@f$, @a c otherwise */
    static int_type not_eof(const int_type& c);
  };
}
#endif

#ifdef GZSTREAM_NEED_CHAR_TRAITS
# ifndef __CSTRING__
#  include <cstring>
# endif

/** @brief The standard C++ library namespace. */
namespace std 
{
  /** @class char_traits<char> compat.h <gzstream/compat.h> 
      @brief Speciliasation of char_traits<typename CharT> for char. 
      @ingroup compat 
  */
  template<>
  struct char_traits<char>
  {
    typedef char 	char_type;  /** The character type  */
    typedef int 	int_type;   /** The integer type  */
    typedef streampos 	pos_type;   /** The position type  */
    typedef streamoff 	off_type;   /** The offset type  */
    typedef mbstate_t 	state_type; /** The state type   */

    /** Assign one character to the value of another 
	@param c1 Character to assign to
	@param c2 Character value to assign from */
    static void assign(char_type& c1, const char_type& c2) { c1 = c2; }
    /** Compare two characters for equility
	@param c1 left hand side 
	@param c2 right hand side
	@return true if @a c1 and @a c2 are equal */
    static bool eq(const char_type& c1, const char_type& c2) { 
      return c1 == c2; }
    /** Lexical compare two characters
	@param c1 left hand side 
	@param c2 right hand side 
	@return true if @a c1 lexically comes before @a c2 */
    static bool lt(const char_type& c1, const char_type& c2) { 
      return c1 < c2; }
    /** Compare lexically two character strings
	@param s1 left hand side
	@param s2 right hand side
	@param n number of characters to compare 
	@return 0 if @a s1 and @a s2 are equal, <0 if @a s1 is smaller
	than @a s2, or >0 otherwise */
    static int compare(const char_type* s1, const char_type* s2, size_t n) { 
      return memcmp(s1, s2, n); }
    /** Get the lenght of a character string 
	@param s The string 
	@return The length (position of terminating '@\0') of @a s */
    static size_t length(const char_type* s) { return strlen(s); }
    /** Find position of a character in a character string
	@param s The string to search
	@param n The maximum number of characters in @a s to look at
	@param a The character to search for 
	@return pointer to @a a if found, otherwise 0 */
    static const char_type* find(const char_type* s, size_t n, 
				 const char_type& a) { 
      return (const char_type*)(memchr(s, a, n)); }
    /** Copy one character string to another.  Strings may overlap. 
	@param s1 The character string to copy to
	@param s2 The character string to copy from
	@param n  The number of characters to copy
	@return @a s1 */
    static char_type* move(char_type* s1, const char_type* s2, size_t n) { 
      return static_cast<char_type*>(memmove(s1, s2, n)); }
    /** Copy one character string to another.  Strings may @e not overlap. 
	@param s1 The character string to copy to
	@param s2 The character string to copy from
	@param n  The number of characters to copy
	@return @a s1 */
    static char_type* copy(char_type* s1, const char_type* s2, size_t n) {  
      return static_cast<char_type*>(memcpy(s1, s2, n)); }
    /** Set all characters of a character string to one character value
	@param s The string to set
	@param n The number of characters in the string to set
	@param a The character value to set @a s to
	@return @a s */
    static char_type* assign(char_type* s, size_t n, char_type a) { 
      return static_cast<char_type*>(memset(s, a, n)); }
    /** Turn an integer into a character 
	@param c The integer to convert
	@return @a c as a character */
    static char_type to_char_type(const int_type& c) { 
      return static_cast<char_type>(c); }
    /** Turn a character into an integer 
	@param c The character to convert
	@return @a c as a character */
    static int_type to_int_type(const char_type& c) { 
      return static_cast<int_type>(static_cast<unsigned char>(c)); }
    /** Compare two integers for equality
	@param c1 left hand side
	@param c2 Right hand side
	@return true iff @f$ c1 = c2@f$ */
    static bool eq_int_type(const int_type& c1, const int_type& c2) { 
      return c1 == c2; }
    /** Get the End-Of-File marker 
	@return End-Of-File marker */
    static int_type eof() { return static_cast<int_type>(EOF); }
    /** Test if a character is the End-Of-File marker
	@param c Character to test
	@return 0 if @f$ c = eof()@f$, @a c otherwise */
    static int_type not_eof(const int_type& c) { 
      return (c == eof()) ? 0 : c; }
  };
}
#endif

#if defined(GZSTREAM_NEED_WCHAR_TRAITS) && defined(GZSTREAM_HAVE_WCHAR_FUNCS)
# ifndef __CSTRING__
#  include <cstring>
# endif
namespace std 
{
  /** @class char_traits<wchar_t> compat.h <gzstream/compat.h> 
      Speciliasation of char_traits<typename CharT> for wchar_t. 
      @ingroup compat 
  */
  template<>
  struct char_traits<wchar_t>
  {
    typedef wchar_t    char_type;  /** The character type*/
    typedef wint_t     int_type;   /** The integer type */
    typedef streamoff  off_type;   /** The stream offset type */
    typedef streampos  pos_type;   /** The stream position type */
    typedef mbstate_t  state_type; /** The state type */
       
    /** Assign one character to the value of another 
	@param c1 Character to assign to
	@param c2 Character value to assign from */
    static void assign(char_type& c1, const char_type& c2) { c1 = c2; } 
    /** Compare two characters for equility
	@param c1 left hand side 
	@param c2 right hand side
	@return true if @a c1 and @a c2 are equal */
    static bool eq(const char_type& c1, const char_type& c2) { 
      return c1 == c2; }
    /** Lexical compare two characters
	@param c1 left hand side 
	@param c2 right hand side 
	@return true if @a c1 lexically comes before @a c2 */
    static bool lt(const char_type& c1, const char_type& c2) { 
      return c1 < c2; }
    /** Compare lexically two character strings
	@param s1 left hand side
	@param s2 right hand side
	@param n number of characters to compare 
	@return 0 if @a s1 and @a s2 are equal, <0 if @a s1 is smaller
	than @a s2, or >0 otherwise */
    static int compare(const char_type* s1, const char_type* s2, size_t n) { 
      return wmemcmp(s1, s2, n); } 
    /** Get the lenght of a character string 
	@param s The string 
	@return The length (position of terminating '@\0') of @a s */
    static size_t length(const char_type* s) { return wcslen(s); } 
    /** Find position of a character in a character string
	@param s The string to search
	@param n The maximum number of characters in @a s to look at
	@param a The character to search for 
	@return pointer to @a a if found, otherwise 0 */
    static const char_type* find(const char_type* s, size_t n, 
				 const char_type& a) { 
      return wmemchr(s, a, n); } 
    /** Copy one character string to another.  Strings may overlap. 
	@param s1 The character string to copy to
	@param s2 The character string to copy from
	@param n  The number of characters to copy
	@return @a s1 */
    static char_type* move(char_type* s1, const char_type* s2, int_type n) { 
      return wmemmove(s1, s2, n); }
    /** Copy one character string to another.  Strings may @e not overlap. 
	@param s1 The character string to copy to
	@param s2 The character string to copy from
	@param n  The number of characters to copy
	@return @a s1 */
    static char_type* copy(char_type* s1, const char_type* s2, size_t n) { 
      return wmemcpy(s1, s2, n); } 
    /** Set all characters of a character string to one character value
	@param s The string to set
	@param n The number of characters in the string to set
	@param a The character value to set @a s to
	@return @a s */
    static char_type* assign(char_type* s, size_t n, char_type a) { 
      return wmemset(s, a, n); } 
    /** Turn an integer into a character 
	@param c The integer to convert
	@return @a c as a character */
    static char_type to_char_type(const int_type& c) { return char_type(c); } 
    /** Turn a character into an integer 
	@param c The character to convert
	@return @a c as a character */
    static int_type to_int_type(const char_type& c) { return int_type(c); } 
    /** Compare two integers for equality
	@param c1 left hand side
	@param c2 Right hand side
	@return true iff @f$ c1 = c2@f$ */
    static bool eq_int_type(const int_type& c1, const int_type& c2) { 
      return c1 == c2; } 
    /** Get the End-Of-File marker 
	@return End-Of-File marker */
    static int_type eof() { return static_cast<int_type>(WEOF); } 
    /** Test if a character is the End-Of-File marker
	@param c Character to test
	@return 0 if @f$ c = eof()@f$, @a c otherwise */
    static int_type not_eof(const int_type& c) { 
      return eq_int_type(c, eof()) ? 0 : c; }
  };
}
#endif

#ifdef GZSTREAM_NEED_STREAMBUF_TYPES
/** @brief Compatibility namespace */
namespace peyton {
namespace io {
namespace gzstream {
namespace compatibility
{
  /** @class streambuf compat.h <gzstream/compat.h> 
      Compatibility streambuf structure
      @ingroup compat 
  */
  struct streambuf : public std::streambuf 
  {
    typedef char                        char_type;
    typedef int                         int_type;
    typedef std::char_traits<char_type> traits_type; 
  };
}
}
}
}
#endif
  
#endif
//
// EOF
//

#ifndef __astro__exceptions_h
#define __astro__exceptions_h

#include <iostream>
#include <string>

#include "system/log.h"

/**
	\file
	\brief Exception class declarations and macros
	
	This file contains a set of macros for easy derivation of your own libpeyton compatible
	exception classes together with the declarations of base libpeyton exception classes.
*/

/**
	\brief Macro for deriving new exceptions from the parent
		
	Use this macro to derive your exceptions from parent exception, if you do not need to add any aditional
	methods or member variables. The purpose of the macro is to relive you from writing the constructor, which
	will be the same as the standard EAny constructor. Eg.:
		
\code
DERIVE_EXCEPTION(EMyException, EAny);
\endcode

	\param ex		The name of the exception you're declaring
	\param parent	The name of parent exception from which your exception will be derived
*/
#define DERIVE_EXCEPTION(ex, parent) \
	class ex : public parent { \
	public: \
		ex(const std::string &inf, const std::string &fun, const std::string &f, int l, const std::string &st) \
			: parent(inf, fun, f, l, st) {} \
	}

/// Macro for deriving exceptions from EAny
#define SIMPLE_EXCEPTION(ex) DERIVE_EXCEPTION(ex, peyton::exceptions::EAny)

/// Macro for deriving exceptions from EIOException
#define IO_EXCEPTION(ex) DERIVE_EXCEPTION(ex, peyton::exceptions::EIOException)

namespace peyton { namespace exceptions {
	class EAny;

	extern int exceptionRaised;
	void log_exception(const EAny &e) throw();

	template<typename E, typename INF>
	void do_throw(const INF &inf, const char *PF, const char *F, int L, const std::string &ST) throw(E)
	{
		E e(inf, PF, F, L, ST);
		if(!peyton::exceptions::exceptionRaised)
		{
			peyton::exceptions::exceptionRaised++;
			e.thrown = true;
			throw e;
		}
		else
		{
			log_exception(e);
		}
	}
} }
/**
	\brief The preferred way to throw EAny derived exceptions
		
	Use this macro instead of C++ throw keyword.
*/
#define THROW(ex, inf) peyton::exceptions::do_throw<ex>(inf, __PRETTY_FUNCTION__, __FILE__, __LINE__, peyton::system::stacktrace());


//////////////////////////////////////////////////////////////////////////////

namespace peyton {
/// Exception classes
namespace exceptions {

	/**
		\brief	Base class for exceptions
		
		All exceptions are derived from this class.
	*/
	class EAny {
	protected:
		bool thrown;
		template<typename E, typename INF>
		friend void do_throw(const INF &inf, const char *PF, const char *F, int L, const std::string &ST) throw(E);
	public:
		std::string info;	///< Descriptive information about the exception (eg. "File not found")

		int line;		///< Source line number at which this exception was thrown.
		std::string func,	///< Function from which this exception was thrown.
			file,		///< Source file containing the function from which the exception was thrown
			stacktrace;	///< Stack trace (if available)
	public:
		EAny(const std::string &inf, const std::string &fun, const std::string &f, int l, const std::string &st)
			: info(inf), func(fun), file(f), line(l), stacktrace(st), thrown(false)
		{ }
		EAny(const EAny &b);
		void print() const throw();
		operator std::string() const;

		~EAny() throw();
	private:
		/**
			Empty constructor is hidden because we want to enforce passing descriptive
			information (eg. filling out the inf member) when the exception is thrown.
		*/
		EAny() {}
	};

	/**
		\brief	C-style errno variable value wrapper
		
		The constructor automatically stores the value of ::errno to err member variable
	*/
	class EErrno : public EAny {
	public:
		int err;	///< value of cstd::errno
		/*
			\overload EAny::EAny(std::string inf, std::string fun, std::string f, int l)
		*/
		EErrno(const std::string &inf, const std::string &fun, const std::string &f, int l, const std::string &st);
	};

	DERIVE_EXCEPTION(EIOException, EErrno);
	IO_EXCEPTION(EEOF);
	IO_EXCEPTION(EFile);
	
	SIMPLE_EXCEPTION(ENotImplemented);
}
}

#define __peyton_exceptions peyton::exceptions

#endif

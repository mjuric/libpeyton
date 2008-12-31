#ifndef __astro_system_fs_h
#define __astro_system_fs_h

#include <astro/exceptions.h>
#include <astro/util.h>

#include <vector>
#include <string>
#include <iosfwd>

namespace peyton {

	namespace exceptions {
		/// Exception while manipulating an environment variable. Thrown by peyton::system::EnvVar class.
		SIMPLE_EXCEPTION(EEnvVar);
		/// Faliure to set the value of an environment variable. Thrown by peyton::system::EnvVar::operator=().
		DERIVE_EXCEPTION(EEnvVarNotSet, EEnvVar);
	}

	namespace io {
		/// useful utilities
		bool file_exists(const std::string &s);	/// test file existence
		off_t file_size(const std::string &s);	/// get file size

		/// useful macros
		#define FILE_EXISTS_OR_THROW(fn) \
			if(!peyton::io::file_exists(fn)) { THROW(EIOException, "Could not access file " + fn); }
#if 0
		/// class representing a filename/path
		class Filename : public std::string
		{
		public:
			Filename(const std::string &fn) : std::string(fn) {}
			bool exists() const;
			off_t size() const;
		};
#endif
		class dir : public std::vector<std::string>
		{
			public:
				dir(const std::string &path);
		};
	}

	namespace system {
		/// class representing an environment variable
		class EnvVar
		{
		protected:
			std::string nm;
		public:	
			EnvVar(const std::string &n) : nm(n) {}

			/// read the value of the environment variable
			operator std::string() const;
			/// read the value of the environment variable
			const char *c_str() const;
			/// set the value of an environment variable, overwriting the former value
			EnvVar &operator =(const std::string &v) { set(v); }
			/// is the environment variable set?
			operator bool() const throw();

			/// set the value of an environment variable, overwrite previous if requested
			void set(const std::string &v, bool overwrite = true);
			void unset() throw();

			std::string name() const throw() { return nm; }
		};

		OSTREAM(const EnvVar &v);

	}
}

#define __peyton_system peyton::system
#define __peyton_io peyton::io

#endif

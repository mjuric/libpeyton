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

	namespace system {

		/// class representing a filename/path
		class Filename : public std::string
		{
		public:
			Filename(const std::string &fn) : std::string(fn) {}
			bool exists() const;
			long long size() const;
		};

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

		class dir : public std::vector<std::string>
		{
		public:
			dir(const std::string &path);
		};

	}
}

#define __peyton_system peyton::system

#endif

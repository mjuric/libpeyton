#include <astro/system/log.h>
#include <astro/system/fs.h>
#include <astro/ui/term.h>

#include <boost/thread/mutex.hpp>

#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <iostream>
#include <fstream>

// for stack traces
#include <execinfo.h>
#include <stdlib.h>
#include <cxxabi.h>
#include <dlfcn.h>

using namespace peyton::system;

LOG_DEFINE(peyton, exception, true);
LOG_DEFINE(app, verb1, true);
LOG_DEFINE(debug, verb5, false);
LOG_DEFINE(message, verb1, true);

/* Obtain a backtrace and print it to stdout. */
void peyton::system::print_trace()
{
	void *array[15];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace(array, 15);
//	MLOG(error) << "Obtained " << size << " stack frames.";
#if 0
	strings = backtrace_symbols(array, size);

	for (i = 0; i < size; i++)
	{
		MLOG(error) << strings[i];
	}

	free(strings);
#else
	// learnt this from http://stupefydeveloper.blogspot.com/2008/10/cc-call-stack.html
	// need to link with -ldl and -rdynamic for it to work
	using namespace abi;

	Dl_info dlinfo;
	int status;
	const char *symname;
	char *demangled;

	for (int i=1; i<size; ++i)
	{
		if(!dladdr(array[i], &dlinfo))
			continue;

		symname = dlinfo.dli_sname;

		demangled = __cxa_demangle(symname, NULL, 0, &status);
		if(status == 0 && demangled)
			symname = demangled;

//		printf("object: %s, function: %s\n", dlinfo.dli_fname, symname);
		if(symname == NULL) { symname = "null"; }
		std::cerr << "  #" << i << ": " << symname << "\n";// << " [" << dlinfo.dli_fname << "]";

		if (demangled)
			free(demangled);
	}
#endif
}

/* Obtain a backtrace and print it to stdout. */
std::string peyton::system::stacktrace()
{
	void *array[15];
	size_t size;
	std::stringstream out;

	size = backtrace(array, 15);
//	out << "Backtrace (" << size << " stack frames):\n";
	out << "Backtrace:\n";

	// learnt this from http://stupefydeveloper.blogspot.com/2008/10/cc-call-stack.html
	// need to link with -ldl and -rdynamic for it to work
	using namespace abi;

	Dl_info dlinfo;
	int status;
	const char *symname;
	char *demangled;

	for (int i=0; i<size; ++i)
	{
		if(!dladdr(array[i], &dlinfo))
			continue;

		symname = dlinfo.dli_sname;

		demangled = __cxa_demangle(symname, NULL, 0, &status);
		if(status == 0 && demangled)
			symname = demangled;

		if(symname == NULL) { symname = "null"; }
		out << "    " << dlinfo.dli_fname << '(' << symname << ")";
		if(i+1<size) out << "\n";

		if (demangled)
			free(demangled);
	}

	return out.str();
}

int Log::level(int newlevel)
{
	if(!loggingOn) { return -10; }
	if(newlevel < 0) { return logLevel; }

	std::swap(newlevel, logLevel);
	return newlevel;
}

Log::~Log()
{
//	std::cerr << "Destructing " << this << " : " << name << "\n";
//	std::cerr << "Destructing " << this << "\n";
}

#if 0
struct TT
{
	std::string name;
	TT() : name("Meee") {}
};

template<typename T>
struct safe_static_object
{
	T *&ptr;
	int refcnt;

	T* operator ->() { return ptr; }
	const T* operator ->() const { return ptr; }

	safe_static_object(T *&ptr_) : ptr(ptr_)
	{
		if(ptr == NULL)
		{
			ptr = new T;
			refcnt = 0;
			std::cout << this << ": C : Creating inner object\n";
		}
		refcnt++;
		std::cout << this << ": C : refcnt=" << refcnt << "\n";
	}
	
	~safe_static_object()
	{
		if(!--refcnt)
		{
			delete ptr;
			ptr = NULL;
			std::cout << this << ": C : Deleting inner object\n";
		}
		std::cout << this << ": D : refcnt=" << refcnt << "\n";
	}
};

static TT *tt_obj = NULL;
static TT *tt_ref = 0;
safe_static_object<TT> tt(tt_obj);
#endif

Log::Log(const std::string &n, int level, bool on)
: name(n), logLevel(level), loggingOn(on), output(NULL)
{
	// global commands
	{
		EnvVar e_on("all_log_on");
		if(e_on) { loggingOn = atoi(e_on.c_str()) != 0; }
		EnvVar e_l("all_log_level");
		if(e_l) { logLevel = atoi(e_l.c_str()); }
	}

	// specific to this object
	{
		EnvVar e_file(name + "_log_file");
		if(e_file)
		{
			loggingOn = true;
			// redirect logging to a file
			output = new std::ofstream(e_file.c_str(), std::ios::app);
			if(!output->good())
			{
				std::cerr << "Cannot open log file '" << e_file << "' for writing.\n";
				abort();
			}
		}
		EnvVar e_l(name + "_log_level");
		if(e_l) { loggingOn = true; logLevel = atoi(e_l.c_str()); }
		EnvVar e_on(name + "_log_on");
		if(e_on) { loggingOn = atoi(e_on.c_str()) != 0; }
	}

	// make a sound if called for
	{
		EnvVar e_on("all_log_list");
		EnvVar e_on1(name + "_log_list");
		if(e_on || e_on1)
		{
			linestream(*this, -3).stream() << "Log '" << identify()
				<< "', log_on = " << loggingOn << ", "
				<< "log_level = " << logLevel;
		}
	}
}

int Log::m_counter = 0;
#if LOG_THREADSAFE
boost::mutex m_mutex;
#endif

int Log::counter()
{
#if LOG_THREADSAFE
	boost::mutex::scoped_lock lock(m_mutex);
#endif
	m_counter++;
	if(m_counter >= 1000) { m_counter = 0; }
	return m_counter;
}

static EnvVar le("LOGGING");

Log::linestream::linestream(Log &p, int level, const char *subsys)
: parent(p), std::ostringstream()
{
//	if(level == exception && (!le || (std::string)le == "0")) { return; }
	if(!le || (std::string)le == "0") { return; }

	time_t t = time(NULL);
	char tt[200], tstr[200];
	strftime(tt, sizeof(tstr), "%c", localtime(&t));
	sprintf(tstr, "%s %03d", tt, Log::counter());
	std::stringstream strm;
	if(subsys)
	{
		std::string ss(subsys);
		size_t p = ss.find('(');
		if(p != std::string::npos) { ss.resize(p); }
		p = ss.rfind(' ');
		if(p != std::string::npos) { ss = ss.substr(p+1); }
		strm << tstr << " " << parent.identify() << "(" << level << ") [" << ss << "]: ";
	}
	else
	{
		strm << tstr << " " << parent.identify() << "(" << level << "): ";
	}
	prefix = strm.str();
}

Log::linestream::~linestream()
{
	std::string ss = str();
	std::ostream &out = parent.output ? *parent.output : std::cerr;

	bool logging = le && (std::string)le != "0";

	// turn on fancyness if we're writing to console
	if(&out == &std::cerr && !logging)
	{
		using namespace peyton::ui::term;

		// detect ':' and make everything before it bold, if it looks like a short heading
		size_t semi = ss.find(':');
		if(semi != std::string::npos && semi < TAB1)
		{
			std::ostringstream oss;
			char padding[TAB1+1]; memset(padding, ' ', TAB1); padding[TAB1-1] = ':'; padding[TAB1] = 0;
			memcpy(padding, ss.c_str(), semi);
			oss << BOLD << padding << OFF << ss.substr(semi+1);
			ss = oss.str();
		}

		// detect the words "WARNING" or "ERROR" and print it out in reverse video
		static const char *rwords[] = { "WARNING", "ERROR" };
		FOR(0, 2)
		{
			const char *word = rwords[i];
			int len = strlen(word);

			size_t at = 0;
			while((at = ss.find(word, at)) != std::string::npos)
			{
				ss.replace(at, len, REVERSE + word + OFF);
				at += len;
			}
		}
	}

	size_t p0 = 0;
	do
	{
		size_t p1 = ss.find('\n', p0);
		if(p1 == std::string::npos)
		{
			if(p0 != ss.size()) { out << prefix << ss.substr(p0) << "\n"; }
			break;
		}

		p1++;
		out << prefix << ss.substr(p0, p1-p0);
		p0 = p1;
	} while(true);
	
	out.flush();
}

std::ostringstream &Log::linestream::stream()
{
	return *this;
}

bool peyton::system::assert_msg_abort()
{
	std::cerr << "\nStack Trace:\n";
	peyton::system::print_trace();
	std::cerr << "\n";
	std::cerr << "##############################################################################\n";
	std::cerr << "# Dump complete.\n";
	std::cerr << "##############################################################################\n";
	std::cerr << "\n";
	abort();
}

bool peyton::system::assert_msg_message(const char *assertion, const char *func, const char *file, int line)
{
	std::cerr << "\n\n\n";
	std::cerr << "############################## ASSERTION FAILED ##############################\n";
	std::cerr << "# Condition : " << assertion << "\n";
	std::cerr << "#\n";
	std::cerr << "# Function  : " << func << "\n";
	std::cerr << "# Source    : " << file << "\n";
	std::cerr << "# Line      : " << line << "\n";
	std::cerr << "##############################################################################\n";
	std::cerr << "# Possible auxilliary information to follow:\n";
	std::cerr << "##############################################################################\n";
	std::cerr << "\n";
	return true;
}

//
//	Format specification:
//	traditional - "%[flag][width][.precision][length]<conversion>"
//	object - "%[anything]@"
//

#include <astro/io/format.h>
#include <astro/exceptions.h>

#include <sstream>

static std::string conversions("diouxXeEfFgGaAcCsSpn");
static std::string flags("#0- +'I");
static std::string length("hlLqjzt");

using namespace peyton::io;
using namespace peyton::exceptions;
using namespace std;

Formatter::Formatter(const std::string &fmt, bool arw)
: formatt(fmt), out(*(new ostringstream)), sstrm(true), at(0), to(0), autorewind(arw)
{
	pop();
}

Formatter::~Formatter()
{
	if(sstrm) {
		ostringstream *s = (ostringstream *)&out;
		delete s;
	}
}

Formatter::operator std::string() const
{
	if(!sstrm)	return "";

	ostringstream *s = (ostringstream *)&out;
	return s->str();
}

void Formatter::pop()
{
	at = to;

	while(true) {
//		cerr << "<<at=" << at << ",to=" << to << ">>";
		
		while(to != formatt.size() && formatt[to] != '%') { ++to; }
		out << formatt.substr(at, to - at);
		at = to;

		if(to == formatt.size()) { break; }
		++to;
//		cerr << "----" << formatt[to] << "----";

		// %% -> %
		if(to != formatt.size() && formatt[to] == '%') { ++at; ++to; continue; }

		//
		// ectract formatt string
		//

		tf.conversion = 0;
		if(formatt[to] == '@')
		{
			++to; 
		}
		else if(formatt[to] == '(')
		{
			int brackets = 0;
			do {
				switch(formatt[to]) {
					case '(' : brackets++; break;
					case ')' : brackets--; break;
				}
				++to;
			} while(to != formatt.size() && brackets);
			if(to != formatt.size() && formatt[to] == '@') // @ is optional in () are used
			{
				++to;
			}
		}
		else
		{
			// old style format
			if(flags.find(formatt[to]) != string::npos) { tf.flag = formatt[to]; ++to; } else { tf.flag = 0; }
			if(to == formatt.size()) { continue; }

			tf.width = -1;
			if(isdigit(formatt[to]))
			{
				tf.width = formatt[to] - '0'; ++to;
				while(to != formatt.size() && isdigit(formatt[to])) { tf.width = 10*tf.width + (formatt[to] - '0'); ++to; }
				if(to == formatt.size()) { continue; }
			}

			tf.prec = -1;
			if(formatt[to] == '.')
			{
				++to;
				tf.prec = 0;
				while(to != formatt.size() && isdigit(formatt[to])) { tf.prec = 10*tf.prec + (formatt[to] - '0'); ++to; }
				if(to == formatt.size()) { continue; }
			}

			tf.length[0] = 0;
			if(length.find(formatt[to]) != string::npos)
			{
				tf.length[0] = formatt[to]; ++to;
				tf.length[1] = 0;
				switch(tf.length[0]) {
				case 'h': case 'l':
					if(to == formatt.size()) { continue; }
					if(formatt[to] == tf.length[0]) { tf.length[1] = tf.length[0]; }
					break;
				}
			}

			if(conversions.find(formatt[to]) != string::npos) { tf.conversion = formatt[to]; ++to; }
			else { continue; } // TODO: emit a warning we had a bad format string
		}

		break;
	};

	// if a good format string found - it's located in [at, to) range in 'formatt'
}

static string int_formats("di");
static string unsigned_formats("ouxX");
static string double_formats("eEfFgGaA");
static string string_formats("s");

void peyton::io::format_type(Formatter &f, const int &x)
{
	char *c; int n;
	if(int_formats.find(f.tf.conversion) != string::npos)
	{
		asprintf(&c, f.front().c_str(), x);
		f.out << c;
		free(c);
	}
	else
	{
		THROW(EIOFormat, "Error in format string - [" + f.front() + "] format requested for an integer");
	}
}

void peyton::io::format_type(Formatter &f, const unsigned &x)
{
	char *c; int n;
	if(unsigned_formats.find(f.tf.conversion) != string::npos)
	{
		asprintf(&c, f.front().c_str(), x);
		f.out << c;
		free(c);
	}
	else
	{
		THROW(EIOFormat, "Error in format string - [" + f.front() + "] format requested for an unsigned integer");
	}
}

void peyton::io::format_type(Formatter &f, const double &x)
{
	char *c; int n;
	if(double_formats.find(f.tf.conversion) != string::npos)
	{
		asprintf(&c, f.front().c_str(), x);
		f.out << c;
		free(c);
	}
	else
	{
		THROW(EIOFormat, "Error in format string - [" + f.front() + "] format requested for a double");
	}
}

void peyton::io::format_type(Formatter &f, const char *x)
{
	char *c; int n;
	if(string_formats.find(f.tf.conversion) != string::npos)
	{
		asprintf(&c, f.front().c_str(), x);
		f.out << c;
		free(c);
	}
	else
	{
		THROW(EIOFormat, "Error in format string - [" + f.front() + "] format requested for a string");
	}
}

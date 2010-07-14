#ifndef _peyton_ranges_h__
#define _peyton_ranges_h__

#include <cassert>
#include <limits>

//#include <iostream>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>

namespace peyton
{
	namespace util
	{
		extern struct interval_special { } ALL;
		extern struct interval_MAX
		{
			template<typename T> operator T() const { return std::numeric_limits<T>::max(); };
		} MAX;
		extern struct interval_MIN
		{
			template<typename T> operator T() const { return std::numeric_limits<T>::is_integer ? std::numeric_limits<T>::min() : -std::numeric_limits<T>::max(); };
		} MIN;

		template<typename T>
		struct interval
		{
			T first, last;

			interval(const T &a) : first(a), last(a) {}
			interval(const T &a, const T &b) : first(a), last(b) {}
			interval(const interval_special &r = ALL) : first(MIN), last(MAX) {}

			bool contains(const T& v) { return first <= v && v <= last; }
			operator bool() const { return first <= last; }
		};

		template<typename T>
		inline interval<T> make_interval(const T &a, const T &b)
		{
			return interval<T>(a, b);
		}

		template<typename T>
		T arg_parse(const std::string &s)
		{
			if(s == "MIN") { return MIN; }
			if(s == "MAX") { return MAX; }
			return boost::lexical_cast<T>(s);
		}

		//
		// Parse intervals of the form:
		//      <r1>..<r2>
		//      MIN..<r2>
		//      <r1>..MAX
		//      ALL
		//
		template<typename T>
		void validate(boost::any& v,
			const std::vector<std::string>& values,
			interval<T>* target_type, int)
		{
			using namespace boost::program_options;
			typedef interval<T> intervalT;

			// Make sure no previous assignment to 'a' was made.
			validators::check_first_occurrence(v);

			// Extract the first string from 'values'. If there is more than
			// one string, it's an error, and exception will be thrown.
			const std::string& s = validators::get_single_string(values);

			if(s == "ALL")
			{
				v = boost::any(intervalT(ALL));
				return;
			}

			static boost::regex r("(.+?)(?:\\.\\.(.+))?");
			boost::smatch match;
			if (boost::regex_match(s, match, r)) {
				//for(int i=0; i != match.size(); i++) { std::cerr << match[i] << "\n"; }
				if(match[2] == "")
				{
					v = boost::any(intervalT(arg_parse<T>(match[1])));
				}
				else
				{
					v = boost::any(intervalT(arg_parse<T>(match[1]), arg_parse<T>(match[2])));
				}
			} else {
				#if BOOST_VERSION  < 104200
					throw validation_error("invalid value");
				#else
					invalid_option_value iov("invalid value");
					throw validation_error(iov);
				#endif
			}
		}

		template<typename T>
		std::ostream &operator<<(std::ostream &out, const interval<T> &r)
		{
			if(r.first == T(MIN) && r.last == T(MAX)) { return out << "ALL"; }
			if(r.first == r.last) { return out << r.first; }

			if(r.first == T(MIN)) { out << "MIN"; } else { out << r.first; }
			out << "..";
			if(r.last == T(MAX)) { out << "MAX"; } else { out << r.last; }

			return out;
		}

		///////////////////////////////////////////////////////////////////////////////

		template<typename T>
		struct range
		{
			T x0, x1, dx;

			range(const T &x0_)
			{
				x0 = x0_;
				dx = T(1);
				x1 = x0 + dx;
				assert(x0 < x1 && x0 + dx >= x1);
			}

			range(const T &x0_, const T &x1_, const T &dx_)
			: x0(x0_), x1(x1_), dx(dx_)
			{ }

			bool encloses(const T &x) const
			{
				return x0 <= x && x < x1;
			}

			struct iterator
			{
				const T dx;
				T at;

				iterator(const T &at0, const T dx_) : at(at0), dx(dx_) {}

				iterator &operator ++()
				{
					at += dx;
					return *this;
				}

				bool operator <(const iterator &it)
				{
					return at < it.at;
				}

				T operator *() const
				{
					return at;
				}
			};

			iterator begin() const { return iterator(x0, dx); }
			iterator end() const { return iterator(x1, dx); }

			bool ge_end(const iterator &i) const
			{
				return i.at < x1;
			}
		};

		template<typename T>
		inline range<T> make_range(T x0, T x1, T dx)
		{
			return range<T>(x0, x1, dx);
		}

		//
		// Parse ranges of the form:
		//      <begin>..<end>..<step>
		//	<begin>
		//
		template<typename T>
		void validate(boost::any& v,
			const std::vector<std::string>& values,
			range<T>* target_type, int)
		{
			using namespace boost;
			using namespace boost::program_options;
			typedef range<T> rangeT;

			// Make sure no previous assignment to 'a' was made.
			validators::check_first_occurrence(v);

			// Extract the first string from 'values'. If there is more than
			// one string, it's an error, and exception will be thrown.
			const std::string& s = validators::get_single_string(values);

			static const std::string num("([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)");
			static regex r(num + "(?:\\.\\." + num + "\\.\\." + num + ")?");
			smatch match;
			if (regex_match(s, match, r)) {
				//for(int i=0; i != match.size(); i++) { std::cerr << i << " " << match[i] << "\n"; }
				if(match[2] == "")
				{
					v = any(rangeT(lexical_cast<T>(match[1])));
				}
				else
				{
					v = any(rangeT(lexical_cast<T>(match[1]), lexical_cast<T>(match[2]), lexical_cast<T>(match[3])));
				}
			} else {
				#if BOOST_VERSION  < 104200
					throw validation_error("invalid value");
				#else
					invalid_option_value iov("invalid value");
					throw validation_error(iov);
				#endif
			}
		}

		template<typename T>
		std::ostream &operator<<(std::ostream &out, const range<T> &r)
		{
			if(r.x0 + r.dx == r.x1 && r.dx == T(1)) { return out << r.x0; }

			return out << r.x0 << ".." << r.x1 << ".." << r.dx;
		}

		template<typename T>
		struct it_range
		{
			typedef typename T::iterator iterator;
			iterator beg, fin;

			it_range(const T &a)
				: beg(a.begin()), fin(a.end())
			{}

			it_range(const iterator &beg_, const iterator &fin_)
				: beg(beg_), fin(fin_)
			{}

			iterator begin() const { return beg; }
			iterator end() const { return fin; }

			bool ge_end(const iterator &i) const
			{
				return i != fin;
			}
		};

		template<typename T>
		inline it_range<T> make_range(const T &a)
		{
			return it_range<T>(a.begin(), a.end());
		}

		#define FORRANGEj(j, R) for(typeof((R).begin()) j = (R).begin(); (R).ge_end(j); ++j)
		#define FORRANGE(R) FORRANGEj(i, R)
	}
}

#endif

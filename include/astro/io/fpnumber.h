#ifndef fpnumber_h_
#define fpnumber_h_

#include <iosfwd>

namespace peyton {
namespace io {

	class fpnumber
	{
	protected:
		double x;
	public:
		class iostate;

		static std::ostream &mark(std::ostream &out);
		static std::ostream &binary(std::ostream &out);
		static std::ostream &text(std::ostream &out);

		static std::istream &mark(std::istream &in);
		static std::istream &binary(std::istream &in);
		static std::istream &text(std::istream &in);
	
		struct format
		{
			unsigned m, k;	// mantissa and exponent bit lengths
			unsigned l;
			format(unsigned m_, unsigned k_) : m(m_), k(k_), l(k_+m_) {}
		};
	public:
		fpnumber() {}
		fpnumber(const double _x) : x(_x) {}
		operator double() const { return x; }
		fpnumber &operator=(const double _x) { x = _x; return *this; }
	};

	std::ostream &operator <<(std::ostream &out, const fpnumber &x);
	std::ostream &operator <<(std::ostream &out, const fpnumber::format &f);
	std::istream &operator >>(std::istream &in, fpnumber &x);
	std::istream &operator >>(std::istream &in, const fpnumber::format &f);

	double bitunpack(int m, int k, unsigned v);
	unsigned bitpack(int m, int k, double x);

}
}

#endif

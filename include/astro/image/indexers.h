#ifndef __indexerss_h
#define __indexerss_h

/*

	Multidimensional array accessors. The purpose of this templates is to
	make easyer to access logically multidimentional data stored in
	physically one-dimensional valarrays.
	
	There are FFT versions of accessor routines, which provide access to
	padded FFTW arrays.
	
	A note about indices. Indices for operator() are in column-maior order
	(that is, FORTRAN style, first index varies fastest when moving through
	linear valarray). The rationale is the following: the arrays describe
	usually something in spatial domain, where we're are used to specify
	indices as x, y, z. So we want to keep that convention. However, when
	traversing the underlying 1D array, I want my x coordinate to vary fastest
	(that is, first traverse row (y=0,x) and then (y=1,x), and so on...). So
	while the operator() indices are in column-major format, the underlying
	array is C-style.

	When dealing with FFTW arrays, x dimension is the one that is padded
	(that is, d_x = 2*(n/2 + 1)).
*/

#include <valarray>
#include <complex>

namespace peyton {
namespace image {

/// access a valarray as 3D image in format suitable for FFTW
template <typename T, typename A = std::valarray<T> >
class indf3 {
public:
	A &v;
	int n[3];
public:
	indf3(A &v_, int x, int y, int z) : v(v_) { n[0] = z; n[1] = y; n[2] = 2*(x/2+1); }
	indf3(A &v_, int N)               : v(v_) { n[0] = n[1] = N;    n[2] = 2*(N/2+1); }

	T &operator ()(int x, int y, int z)       { return v[x + y*n[2] + z*n[2]*n[1]]; }
	
	int rank() { return 3; }
};


/// access a valarray as FFTW padded array of complex numbers
template <typename T, typename A = std::valarray<T> >
class indfc3 {
protected:
	int index(int x, int y, int z) const 	{ return 2*x + y*n[2] + z*n[2]*n[1]; }
public:
	A &v;
	int n[3];
public:
	indfc3(A &v_, int x, int y, int z) : v(v_) { n[0] = z; n[1] = y; n[2] = 2*(x/2+1); }
	indfc3(A &v_, int N)               : v(v_) { n[0] = n[1] = N;    n[2] = 2*(N/2+1); }

	std::complex<T> operator ()(int x, int y, int z) const { const int i = index(x, y, z); return std::complex<T>(v[i], v[i+1]); }
	void set(int x, int y, int z, std::complex<T> c) { const int i = index(x, y, z); v[i] = std::real(c); v[i+1] = std::imag(c); }

	int rank() { return 3; }
};


/// access a valarray as 3D image
template <typename T, typename A = std::valarray<T> >
class ind3 {
public:
	A &v;
	int n[3];
public:
	ind3(A &v_, int x, int y, int z) : v(v_) { n[0] = z; n[1] = y; n[2] = x; }
	ind3(A &v_, int N)               : v(v_) { n[0] = n[1] = n[2] = N; }

	T &operator ()(int x, int y, int z)       { return v[x + y*n[2] + z*n[2]*n[1]]; }

	int rank() { return 3; }
};

/// access a valarray as 2D image
template <typename T, typename A = std::valarray<T> >
class ind2 {
public:
	typedef T value_type;
	typedef A array_type;

	A &v;
	int n[2];

	class iterator {
	public:
		ind2 &img;
		int x, y;
	public:
		iterator(ind2 &img_, int x_ = 0, int y_ = 0) : img(img_), x(x_), y(y_) {}

		iterator &operator ++() { if(++x == img.x()) { x = 0; y++; }; return *this; }
		iterator &operator ++(int) { return ++(*this); }

		T &operator *() { return img(x, y); }
		T operator *() const { return img(x, y); }

		int index() const { return img.index(x, y); }

		bool operator==(const iterator &b) const { return x == b.x && y == b.y; }
		bool operator!=(const iterator &b) const { return !(*this == b); }
	};
public:
	ind2(A &v_, int x, int y) : v(v_) { n[0] = y; n[1] = x; }
	ind2(A &v_, int N)        : v(v_) { n[0] = n[1] = N; }

	int index(int x, int y) const { return x + y*n[1]; }

	// Image element accessors
	T &operator ()(int x, int y)             { return v[index(x, y)]; }
	T &operator ()(int x, int y) const       { return v[index(x, y)]; }
	T &operator ()(const iterator &i)        { return v[index(i.x, i.y)]; }
	T &operator ()(const iterator &i) const  { return v[index(i.x, i.y)]; }
	T &operator [](const int i)              { return v[i]; }
	T  operator [](const int i) const        { return v[i]; }

	// Image dimension accessors 
	int x() const { return n[1]; }
	int y() const { return n[0]; }
	int &x() { return n[1]; }	/// (note: these DO NOT resize the underlying array)
	int &y() { return n[0]; }	/// (note: these DO NOT resize the underlying array)

	int rows() const { return y(); }
	int cols() const { return x(); }

	int rank() { return 2; }

	// STL iterator interface
	iterator begin() { return iterator(*this); }
	iterator end() { return iterator(*this, 0, y()); }
	int size() { return v.size(); }

	operator array_type &() { return v; }
};

#if 0

THIS IS STILL UNFINISHED

template <typename Ind, typename Trans>
class phys_ind2 : public Ind
{
protected:
	typedef Trans transform;
	transform trans;
public:
	phys_ind2(A &v_, int x, int y) : Ind(v, x, y) {}
	phys_ind2(A &v_, int N)        : Ind(v, N)    {}

	T &transformed(V2)       { return v[index(x, y)]; }
};

#endif

/**
	\brief Copy real valarray to fft (padded) format valarray
	\note src and dest arrays _must not_ be the same
*/
template<typename A>
A&
copy_real_to_fft(A &f, A &r, const int n, const int size)
{
	f[0] = r[0];
	for(int i = 1, j = 1; i != size; i++, j++) {
		if(i % n == 0) { j += 2; }
		f[j] = r[i];
	}
	return f;
}

//
// FFT helper declarations and functions
//

extern "C" {
	void fftw3d(int n, float *scr, int iopt, char *wisdomfile);	
}

/**
	\brief Do an inplace FFT on an N-dimensional image.
	
	Input image \a scr is assumed to be real, in padded FFTW array format (use indfX accessors
	for accessing such arrays). On return, the image will contain complex numbers (eg., use
	indfcX accessors for accessing the array).
	
	\todo Document what each of the parameters mean. Write example code snippet.
	\note Requires linking with FFTW library.
*/
inline void fftw3d(int n, std::valarray<float> &scr, int iopt, char *wisdomfile = NULL)
{
	fftw3d(n, &scr[0], iopt, wisdomfile);
}

} // namespace image
} // namespace peyton

#define __peyton__image peyton::image

#endif

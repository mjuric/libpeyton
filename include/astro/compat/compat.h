#ifndef __compat_h
#define __compat_h

#ifndef _MSC_VER
#define stricmp strcasecmp
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#if __GNUC__ < 3 || defined _MSC_VER
template<typename T>
inline T round(T x) {
	if(x > 0) {
		return (int(x - .5) == int(x)) ? T(int(x+1)) : T(int(x));
	} else {
		x = -x;
		return (int(x - .5) == int(x)) ? T(-int(x+1)) : T(-int(x));
	}
}
#endif

#endif

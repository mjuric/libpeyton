#ifndef __photometry_h
#define __photometry_h

namespace peyton {
namespace sdss {
namespace phot {

	void johnson(double &V, double &B, double r, double g) throw();

	const int uband = 0;
	const int gband = 1;
	const int rband = 2;
	const int iband = 3;
	const int zband = 4;
	
	/// convert flux f, with zero point f0, error fluxErr and in band band to
	/// a corresponding luptitude
	void luptitude(float f, float fluxErr, int band, float &lupt, float &luptErr, float f0) throw();
	void fluxfromluptitude(float lupt, float luptErr, int band, float &f, float &fErr, float f0) throw();

	void pogson(float f, float fErr, float &mag, float &magErr, float f0) throw();
	void fluxfrompogson(float mag, float magErr, float &f, float &fErr, float f0) throw();
}
}
}

#define __peyton_sdss peyton::sdss

#endif

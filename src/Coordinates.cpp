#include <iostream>

#include <astro/coordinates.h>
#include <astro/constants.h>

#include <math.h>
#include <string.h>
#include <stdio.h>

using namespace peyton;

/*
################################################################
### given RA and Dec ($1,$2), vRA and vDec in ($3,$4), and
### lambda and beta ($5,$6), stuff ($7, $8) with vLambda & vBeta
### (formulae from Mueler)
eqvel2ecvel2 8

     define d2r (asin(1.0)/90.0) # degree to radian
     define eps ($d2r*23.439291)
     define sineps (sin($eps))
     define coseps (cos($eps))

     set sinalpha = sin($d2r*$1)
     set cosalpha = cos($d2r*$1)
     set sindelta = sin($d2r*$2)
     set cosdelta = cos($d2r*$2)
     set tandelta = sindelta/cosdelta
     set secdelta = 1.0/cosdelta

     set secbeta = 1.0/cos($d2r*$6)
     set tanbeta = sin($d2r*$6)/cos($d2r*$6)
     set coslambda = cos($d2r*$5)

     set sinr = coslambda*secdelta*$sineps
     set cosr = secdelta*secbeta*$coseps - tandelta*tanbeta

     # vLambda
     set $7 = ($3*cosr*cosdelta+$4*sinr)*secbeta
     # vBeta
     set $8 = -$3*sinr*cosdelta+$4*cosr

     unset sinalpha unset cosalpha
     unset sindelta unset cosdelta unset tandelta unset secdelta
     unset secbeta unset tanbeta unset coslambda
     unset sinr unset cosr
################################################################
*/

#define Power pow
#define Cos cos
#define Sin sin
#define Sqrt sqrt

// transforms from GCS to equatorial coordinate system
void coordinates::rot_vel(
	Radians node, Radians inc,
	Radians muX, Radians nuX,
	Radians vmuX, Radians vnuX,
	Radians &vra, Radians &vdec
)
// ################################################################
// ### given mu and nu ($1,$2), vmu and vnu in ($3,$4), and
// ### ra and dec ($5,$6), stuff ($7, $8) with vra & vdec
// ### (formulae from Mueler)
// eqvel2ecvel2 8
{
#if 0
	double raX, decX;
	gcsequ(node, inc, muX, nuX, raX, decX);

	muX -= node;
	inc = -inc;

     double sininc = sin(inc);
     double cosinc = cos(inc);

     double sinalpha = sin(muX);
     double cosalpha = cos(muX);
     double sindelta = sin(nuX);
     double cosdelta = cos(nuX);
     double tandelta = sindelta/cosdelta;
     double secdelta = 1.0/cosdelta;

     double secbeta = 1.0/cos(decX);
     double tanbeta = sin(decX)/cos(decX);
     double coslambda = cos(raX);

     double sinr = coslambda*secdelta*sininc;
     double cosr = secdelta*secbeta*cosinc - tandelta*tanbeta;

     // vLambda
     vra = (vmuX*cosr*cosdelta+vnuX*sinr)*secbeta;
     // vBeta
     vdec = -vmuX*sinr*cosdelta+vnuX*cosr;
#else
	vra = 
    (-2*vnuX*Cos(muX - node)*Sin(inc) +
      vmuX*(2*Cos(inc)*Power(Cos(nuX),2) -
         Sin(inc)*Sin(muX - node)*Sin(2*nuX)))/
    (2.*(Power(Cos(muX - node),2)*Power(Cos(nuX),2) +
        Power(-(Cos(inc)*Cos(nuX)*Sin(muX - node)) + Sin(inc)*Sin(nuX),2)));

	vdec = 
   (vmuX*Cos(muX - node)*Cos(nuX)*Sin(inc) +
      vnuX*(Cos(inc)*Cos(nuX) - Sin(inc)*Sin(muX - node)*Sin(nuX)))/
    Sqrt(1 - Power(-(Cos(nuX)*Sin(inc)*Sin(muX - node)) - Cos(inc)*Sin(nuX),
       2));
#endif
}

// ################################################################

void // equgcs - Equatorial to Great Circles System
coordinates::equgcs(Radians node, Radians inc, Radians ra, Radians dec, Radians &mu, Radians &nu)
{
	Radians x1, y1, z1, x2, y2, z2, cdec, sinc, cinc;

	// Rotation
	ra -= node;
	
	cdec = cos(dec); sinc = sin(inc); cinc = cos(inc);

	x1 = cos(ra)*cdec;
	y1 = sin(ra)*cdec;
	z1 = sin(dec);
  
	x2 = x1;
	y2 = y1*cinc + z1*sinc;
	z2 =-y1*sinc + z1*cinc;

	mu = atan2(y2, x2) + node;
	nu = asin(z2);

	while(mu < 0) mu += ctn::pi2;
	while(mu > ctn::pi2) mu -= ctn::pi2;
}

void // gcsequ - Great Circles System to Equatorial
coordinates::gcsequ(Radians node, Radians inc, Radians mu, Radians nu, Radians &ra, Radians &dec)
{
	double x1, y1, z1, x2, y2, z2, sinc, cinc, cnu;

	// Rotation
	mu -= node;
	
	cnu = cos(nu); sinc = sin(inc); cinc = cos(inc);
	
	x2 = cos(mu)*cnu;
	y2 = sin(mu)*cnu;
	z2 = sin(nu);

	x1 = x2;
	y1 = y2*cinc - z2*sinc;
	z1 = y2*sinc + z2*cinc;

	ra = atan2 (y1, x1) + node;
	dec = asin(z1);

	while(ra < 0) ra += ctn::pi2;
	while(ra > ctn::pi2) ra -= ctn::pi2;
}

void
coordinates::eclequ(Radians lambda, Radians betha, Radians &ra, Radians &dec)
{
	const double epsilon = 23.4392911*ctn::d2r;
	ra = atan2((sin(lambda) * cos(epsilon) - tan(betha) * sin(epsilon)), cos(lambda));
	dec = asin(sin(betha) * cos(epsilon) + cos(betha) * sin(epsilon) * sin(lambda));

	// normalize
	if(ra < 0) ra += ctn::pi2;
}

void
coordinates::equecl(Radians ra, Radians dec, Radians &lambda, Radians &betha)
{
	const double epsilon = 23.4392911*ctn::d2r;
	lambda = atan2((sin(ra) * cos(epsilon) + tan(dec) * sin(epsilon)), cos(ra));
	betha = asin(sin(dec) * cos(epsilon) - cos(dec) * sin(epsilon) * sin(ra));

	// normalize
	if(lambda < 0) lambda += ctn::pi2;
}

// galactic north pole (alpha, delta)
#if 0
// Binney-Merrifield, pg 30 (note that BM has a typo in l0 -- 123.932 instead of 122.932)
static const double angp = ctn::d2r * 192.85948;
static const double dngp = ctn::d2r * 27.12825;
static const double l0 = ctn::d2r * 32.932;
#else
// Appendix of Reid et al. (http://adsabs.harvard.edu/cgi-bin/bib_query?2004ApJ...616..872R)
// This convention is also used by LAMBDA/WMAP (http://lambda.gsfc.nasa.gov/toolbox/tb_coordconv.cfm)
static const double angp = ctn::d2r * 192.859508333; //  12h 51m 26.282s (J2000)
static const double dngp = ctn::d2r * 27.128336111;  // +27d 07' 42.01" (J2000)
static const double l0 = ctn::d2r * 32.932;
#endif
static const double ce = cos(dngp);
static const double se = sin(dngp);

void // equgal - Equatorial to Galactic coordinates
coordinates::equgal(Radians ra, Radians dec, Radians &l, Radians &b)
{
	const double cd = cos(dec);
	const double sd = sin(dec);
	const double ca = cos(ra-angp);
	const double sa = sin(ra-angp);

	double sb = cd*ce*ca + sd*se;
	double tly = sd - sb*se;
	double tlx = cd*sa*ce;
	
	l = atan2(tly, tlx) + l0;
	b = asin(sb);

	while(l < 0) { l += ctn::pi2; }
}

void // equgal - Equatorial to Galactic coordinates
coordinates::galequ(Radians l, Radians b, Radians &ra, Radians &dec)
{
	const double cb = cos(b);
	const double sb = sin(b);
	const double cl = cos(l-l0);
	const double sl = sin(l-l0);

	ra = atan2(
			cb*cl,
			sb*ce-cb*se*sl
		) + angp;
	dec = asin(cb*ce*sl + sb*se);

	while(ra < 0) { ra += ctn::pi2; }
}

void // gcsgal - SDSS GCS to Galactic coordinates
coordinates::gcsgal(Radians node, Radians inc, Radians mu, Radians nu, Radians &l, Radians &b)
{
	Radians ra, dec;
	coordinates::gcsequ(node, inc, mu, nu, ra, dec);
	coordinates::equgal(ra, dec, l, b);
}

void // galgcs - Galactic to SDSS GCS coordinates
coordinates::galgcs(Radians node, Radians inc, Radians l, Radians b, Radians &mu, Radians &nu)
{
	Radians ra, dec;
	coordinates::galequ(l, b, ra, dec);
	coordinates::equgcs(node, inc, ra, dec, mu, nu);
}

void coordinates::identity(double lon, double lat, double &lon1, double &lat1)
{
	lon1 = lon;
	lat1 = lat;
}

coordinates::transform coordinates::get_transform(const std::string &from, const std::string &to)
{
	if(from == "gal")
	{
		if(to == "equ") { return galequ; }
		if(to == "gal") { return identity; }
	}
	if(from == "equ")
	{
		if(to == "gal") { return equgal; }
		if(to == "ecl") { return equecl; }
		if(to == "equ") { return identity; }
	}
	if(from == "ecl")
	{
		if(to == "equ") { return eclequ; }
		if(to == "ecl") { return identity; }
	}
	return NULL;
}



Radians coordinates::distance(Radians xr1, Radians yr1, Radians xr2, Radians yr2)
{
	double pos1[3], pos2[3], w, cosb;
	Radians diff;
	int i;

	/* Convert two vectors to direction cosines */
	cosb = cos (yr1);
	pos1[0] = cos (xr1) * cosb;
	pos1[1] = sin (xr1) * cosb;
	pos1[2] = sin (yr1);

	cosb = cos (yr2);
	pos2[0] = cos (xr2) * cosb;
	pos2[1] = sin (xr2) * cosb;
	pos2[2] = sin (yr2);

	/* Modulus squared of half the difference vector */
	w = 0.0;
	for (i = 0; i < 3; i++) {
	w = w + (pos1[i] - pos2[i]) * (pos1[i] - pos2[i]);
	}
	w = w / 4.0;
	if (w > 1.0) w = 1.0;

	/* Angle beween the vectors */
	diff = 2.0 * atan2 (sqrt (w), sqrt (1.0 - w));

	return diff;
}

bool coordinates::inBox(double ra, double dec, double ralo, double declo, double rahi, double dechi)
{
	if(rahi < ralo) {
		if(inBox(ra, dec, ralo, declo, ctn::pi2, dechi)) return true;
		return inBox(ra, dec, 0, declo, rahi, dechi);
	}

	if(ralo < ra && ra < rahi &&
		declo < dec && dec < dechi) { return true; }
	
	return false;
}

inline int str_find(const char *where, const char what)
{
	const char *c = strchr(where, what);
	if(c == NULL) return -1;
	return c - where;
}

void coordinates::formatCoord(const char *fmt, char *out, const double _v)
{
	double v;
	double H, M, S;
	v = _v;
	H = v / 15; v -= int(H) * 15;
	M = v * 4; v -= floor(M) / 4;
	S = v * 240;

	v = fabs(_v);
	double d, m, s;
	d = v; v -= floor(d);
	m = v * 60; v -= floor(m) / 60;
	s = v * 3600;

	char sign = _v > 0 ? '+' : '-';

	double asrc[] = {floor(H), floor(M), S, H, M, floor(d), floor(m), s, d, m};
	double arr[50] = {0};
	int num = 0, i = 0, id;
	const char *valid = "HMSINdmsing";
	const char *c =fmt;

	char buf[1000];
	while(*c != 0) {
		if(*c == '%') {
			const char *start = c;
			while(*c != 0 && (id = str_find(valid, *c)) == -1) {
				c++;
			}
			if(*c == 0) break;
			if(*c == 'g') {
				buf[i] = sign; i++;
			} else {
				for(; start != c; start++, i++) {
					buf[i] = *start;
				}
				buf[i] = 'f'; i++;
				arr[num] = asrc[id]; num++;
			}
		} else {
			buf[i] = *c; i++;
		}
		c++;
	}
	buf[i] = 0;

	sprintf(out, buf, arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6], arr[7], arr[8], arr[9],
		arr[10], arr[11], arr[12], arr[13], arr[14], arr[15], arr[16], arr[17], arr[18], arr[19]
		);
}

void coordinates::formatCoord(char *ras, char *decs, const double ra_, const double dec_, const int fmt)
{
	double ra = ra_, dec = dec_;

	char neg = dec > 0 ? ' ' : '-';
	if(neg == '-') dec = -dec;

	int a, b; double c;
	a = int(ra / 15); ra -= a * 15;
	b = int(ra * 4); ra -= (float)b / 4;
	c = ra * 240;
	if(fmt & FC_MPEC_FORMAT)
		sprintf(ras, "%02.0f %02.0f %05.2f", (float)a, (float)b, c);
	else if(fmt & FC_NO_FRAC_SECONDS)
		sprintf(ras, "%02.0f:%02.0f:%02.0f", (float)a, (float)b, c);
	else
		sprintf(ras, "%02.0f:%02.0f:%05.2f", (float)a, (float)b, c);

	a = int(dec); dec -= a; dec = fabs(dec);
	b = int(dec * 60); dec -= (float)b / 60;
	c = dec * 3600;
	// check for roundoffs, to single digit
	if(c - (int)c >= .95) {
		c = ceil(c);
		if(c == 60) {
			c = 0;
			b++;
			if(b == 60) {
				b = 0;
				a++;
			}
		}
	}

	a = (int)fabs(a);
	b = (int)fabs(b);
	c = fabs(c);

	if(fmt & FC_MPEC_FORMAT)
		sprintf(decs, "%c%02.0f %02.0f %04.1f", neg, (float)a, (float)b, c);
	else if(fmt & FC_NO_FRAC_SECONDS)
		sprintf(decs, "%c%02.0f:%02.0f:%02.0f", neg, (float)a, (float)b, c);
	else
		sprintf(decs, "%c%02.0f:%02.0f:%05.2f", neg, (float)a, (float)b, c);

}


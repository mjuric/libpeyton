#include <astro/util.h>
#include <astro/system/exceptions.h>

#include <cmath>

// void (*derivs)(float, float [], float []))

namespace peyton {
namespace math {
/// numerical methods related classes, functions and namespaces
namespace numeric {
/**
	\brief various, mostly Runge-Kutta, integrators. Mostly adapted/stolen from Num.Rec.
	\todo Document this namespace better, provide code samples
*/
namespace integrators {

	/*
		\brief 4th order Runge-Kutta integration
		
		Given values for the variables \a y[1..n] and their derivatives \a dydx[1..n] known at \a x, use the
		fourth-order Runge-Kutta method to advance the solution over an interval \a h and return the
		incremented variables as \a yout[1..n], which need not be a distinct array from \a y. The user
		supplies the routine \a derivs(x,y,dydx), which returns derivatives \a dydx at \a x.
	*/
	template<typename D, class Array, typename Derivs>
	void rk4(
		Array &out,
		const Array &y, const Array &dydx, const int n,
		const D x, const D h, Derivs derivs)
	{
		int i;
		D xh,hh,h6;

		Array dym(n), dyt(n), yt(n);

		hh=h*0.5;
		h6=h/6.0;
		xh=x+hh;

		for (i=0; i<n; i++) yt[i]=y[i]+hh*dydx[i];	// First step.
		derivs(xh,yt,dyt);				// Second step.

		for (i=0; i<n; i++) yt[i]=y[i]+hh*dyt[i];
		derivs(xh,yt,dym);				// Third step.

		for (i=0; i<n; i++) {
			yt[i]=y[i]+h*dym[i];
			dym[i] += dyt[i];
		}

		derivs(x+h,yt,dyt);				// Fourth step.
		for (i=0; i<n; i++) {				// Accumulate increments with proper weights.
			yout[i]=y[i]+h6*(dydx[i]+dyt[i]+2.0*dym[i]);
		}
	}

	///////////////////////////////////////////////////////////////////

	/**
		\brief 5th order Cash-Karp RK integrator
		
		Given values for \a n variables \a y[1..n] and their derivatives \a dydx[1..n] known at \a x, use
		the 5th-order Cash-Karp Runge-Kutta method to advance the solution over an interval \a h
		and return the incremented variables as \a yout[1..n]. Also return an estimate of the local
		truncation error in yout using the embedded fourth-order method. The user supplies the routine
		\a derivs(x,y,dydx), which returns derivatives \a dydx at \a x.
	*/

	template<typename D, class Array, typename Derivs>
	void rkck(
		Array &yout, Array &yerr,
		const Array y, const Array dydx, const int n, const D x, const D h,
		Derivs derivs)
	{
		int i;
		static D a2=0.2,a3=0.3,a4=0.6,a5=1.0,a6=0.875,b21=0.2,
			b31=3.0/40.0,b32=9.0/40.0,b41=0.3,b42 = -0.9,b43=1.2,
			b51 = -11.0/54.0, b52=2.5,b53 = -70.0/27.0,b54=35.0/27.0,
			b61=1631.0/55296.0,b62=175.0/512.0,b63=575.0/13824.0,
			b64=44275.0/110592.0,b65=253.0/4096.0,c1=37.0/378.0,
			c3=250.0/621.0,c4=125.0/594.0,c6=512.0/1771.0,
			dc5 = -277.00/14336.0;
		D dc1=c1-2825.0/27648.0,dc3=c3-18575.0/48384.0, dc4=c4-13525.0/55296.0,dc6=c6-0.25;

		Array ak2(n), ak3(n), ak4(n), ak5(n), ak6(n), ytemp(n);

	 	// First step.
	 	FOR(0, n) ytemp[i]=y[i]+b21*h*dydx[i];

		derivs(x+a2*h,ytemp,ak2); 	// Second step.
		FOR(0, n) ytemp[i]=y[i]+h*(b31*dydx[i]+b32*ak2[i]);

		derivs(x+a3*h,ytemp,ak3); 	// Third step.
		FOR(0, n) ytemp[i]=y[i]+h*(b41*dydx[i]+b42*ak2[i]+b43*ak3[i]);

		derivs(x+a4*h,ytemp,ak4); 	// Fourth step.
		FOR(0, n) ytemp[i]=y[i]+h*(b51*dydx[i]+b52*ak2[i]+b53*ak3[i]+b54*ak4[i]);

		derivs(x+a5*h,ytemp,ak5); 	// Fifth step.
		FOR(0, n) ytemp[i]=y[i]+h*(b61*dydx[i]+b62*ak2[i]+b63*ak3[i]+b64*ak4[i]+b65*ak5[i]);

		derivs(x+a6*h,ytemp,ak6); 	// Sixth step.

		// Accumulate increments with proper weights.
		FOR(0, n) yout[i]=y[i]+h*(c1*dydx[i]+c3*ak3[i]+c4*ak4[i]+c6*ak6[i]);

		// Estimate error as difference between fourth and fth order methods.
		FOR(0, n) yerr[i]=h*(dc1*dydx[i]+dc3*ak3[i]+dc4*ak4[i]+dc5*ak5[i]+dc6*ak6[i]);
	}

	/////////////////////////////////////////////////

	#define SAFETY	 0.9
	#define PGROW	-0.2
	#define PSHRNK	-0.25
	#define ERRCON	 1.89e-4	// The value ERRCON equals (5/SAFETY) raised to the power (1/PGROW), see use below.

	/**
		\brief 5th-order Runge-Kutta step with monitoring of local truncation error
		
		Fifth-order Runge-Kutta step with monitoring of local truncation error to ensure accuracy and
		adjust stepsize. Input are the dependent variable vector y[1..n] and its derivative dydx[1..n]
		at the starting value of the independent variable x. Also input are the stepsize to be attempted
		htry, the required accuracy eps, and the vector yscal[1..n] against which the error is
		scaled. On output, y and x are replaced by their new values, hdid is the stepsize that was
		actually accomplished, and hnext is the estimated next stepsize. derivs is the user-supplied
		routine that computes the right-hand side derivatives.
	*/
	template<typename D, class Array, typename Derivs>
	void rkqs(
		D &x, Array &y, D &hdid, D &hnext,		// input/output x, y[], actual step size, predicted next s. size
		const Array dydx, const int n, const D htry,	// input derivatives, array size, initial step size to try
		const D eps, const Array yscal,		// accuracy constraints
		Derivs derivs)

	{
		int i;
		D errmax,h,htemp,xnew;

		Array yerr(n), ytemp(n);
		h=htry;			// Set stepsize to the initial trial value.

		for (;;) {
			rkck(ytemp, yerr, y, dydx, n, x, h, derivs);	// Take a step.

			errmax=0.0;				// Evaluate accuracy.
			for (i=0;i<n;i++) errmax = std::max(errmax, std::abs(yerr[i]/yscal[i]));

			errmax /= eps;				// Scale relative to required tolerance.
			if (errmax <= 1.0) break;		// Step succeeded. Compute size of next step.
			htemp = SAFETY*h*pow(errmax,PSHRNK);

			// Truncation error too large, reduce stepsize.
			// No more than a factor of 10.
			h=(h >= 0.0 ? std::max(htemp,0.1*h) : std::min(htemp,0.1*h));

			xnew=x+h;
			if (xnew == x) { THROW(EAny, "stepsize underflow in rkqs"); }
		}

		if (errmax > ERRCON) hnext = SAFETY*h*pow(errmax,PGROW);
		else hnext=5.0*h; // No more than a factor of 5 increase.

		x += (hdid=h);

		y=ytemp;
	}

	///////////////////////////////////////////

	#define MAXSTP 10000000
	#define TINY 1.0e-30
	// extern int kmax,kount;
	// extern float *xp,**yp,dxsav;

	/*
		User storage for intermediate results. Preset kmax and dxsav in the calling program. If kmax !=
		0 results are stored at approximate intervals dxsav in the arrays xp[1..kount], yp[1..nvar]
		[1..kount], where kount is output by odeint. Defining declarations for these variables, with
		memory allocations xp[1..kmax] and yp[1..nvar][1..kmax] for the arrays, should be in
		the calling program.
	*/
	template<typename D, class Array>
	struct StdProgressCallback {
		void operator()(D x, Array y, int n) {
			std::cout << x;
			FOR(0, n) std::cout << " " << y[i];
			std::cout << "\n";
		}
	};

	/**
		\brief Runge-Kutta driver with adaptive stepsize control

		Runge-Kutta driver with adaptive stepsize control. Integrate starting values ystart[1..nvar]
		from x1 to x2 with accuracy eps, storing intermediate results in global variables. h1 should
		be set as a guessed rst stepsize, hmin as the minimum allowed stepsize (can be zero). On
		output nok and nbad are the number of good and bad (but retried and fixed) steps taken, and
		ystart is replaced by values at the end of the integration interval. derivs is the user-supplied
		routine for calculating the right-hand side derivative, while rkqs is the name of the stepper
		routine to be used.
	*/
	template<typename D, class Array, typename Derivs, typename ProgressCallback>
	void odeint(Array &ystart,	///< y(x1) on input and y(x2) on output
		int &nok,			///< number of correct step guesses
		int &nbad,			///< number of step guess misses
		Derivs derivs,
		const int n,		///< y[] size
		const D x1,			///< start x
		const D x2,			///< finish x
		const D eps,		///< maximum allowed error
		const D h1,			///< initial step guess
		const D hmin,		///< minimum allowed step size
		const float dxsav,
		ProgressCallback &progress	///< progress callback routine
	)
	{
		int nstp,i;
		D xsav,x,hnext,hdid,h;

		Array yscal(n), y(n), dydx(n);

		x=x1;
		h=sign(h1,x2-x1);
		nok = nbad = 0;

		y = ystart;
		xsav=x-dxsav*2.0;	// Assures storage of first step.

		FORj(nstp, 0, MAXSTP) {	// Take at most MAXSTP steps.
			derivs(x, y, dydx);

			// Scaling used to monitor accuracy. This general-purpose choice can be modied if need be.
			FOR(0, n) { yscal[i]=std::abs(y[i])+std::abs(dydx[i]*h)+TINY; }

			if (std::abs(x-xsav) > std::abs(dxsav)) {
				progress(x, y, n);	// Store intermediate results
				xsav=x;
			}

			if ((x+h-x2)*(x+h-x1) > 0.0) h=x2-x;	// If stepsize can overshoot, decrease.
		
			rkqs(x, y, hdid, hnext, dydx, n, h, eps, yscal, derivs);

			if (hdid == h) ++nok; else ++nbad;
			if ((x-x2)*(x2-x1) >= 0.0) {		// Are we done?
				ystart = y;
				progress(x, y, n);		// Save final step
				return;				// Normal exit.
			}

			if (std::abs(hnext) <= hmin) THROW(EAny, "Step size too small in odeint");
			h=hnext;
		}
		THROW(EAny, "Too many steps in routine odeint");
	}

	/// Specialized version of integrator driver, with \a h1 = 1 year
	template<typename D, class Array, typename Derivs, typename ProgressCallback>
	void odeint(Array &ystart,			///< y(x1) on input & y(x2) on output
		Derivs derivs,
		const D x1,					///< start x
		const D x2,					///< finish x,
		const D eps,				///< maximum allowed error
		const D h1,					///< initial step guess
		ProgressCallback &progress
	) {
		int nok, nbad;
		odeint(ystart, nok, nbad, derivs,
			ystart.size(), x1, x2, eps,
			h1, 0.,
			1.*3600.*24.*365., progress);
	}

	/// Specialized version of integrator driver
	template<typename D, class Array, typename Derivs>
	void odeint(Array &ystart,			///< y(x1) on input & y(x2) on output
		Derivs derivs,
		const D x1,					///< start x
		const D x2,					///< finish x,
		const D eps,				///< maximum allowed error
		const D h1					///< initial step guess
	) {
		int nok, nbad;
		odeint(ystart, nok, nbad, derivs,
			ystart.size(), x1, x2, eps,
			h1, 0.,
			0, StdProgressCallback<D, Array>());
	}

} // namespace integrators
} // namespace numeric
} // namespace math
} // namespace peyton

#define __peyton_math_numeric_integrators peyton::math::numeric::integrators

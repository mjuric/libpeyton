#include <Magick++.h>
#include <astro/math/vector.h>

namespace peyton {
namespace io {
namespace magick {

	typedef peyton::math::V3 RGB;

	template<typename Image>
	class Transfer
	{
	public:
		virtual RGB operator[](const typename Image::value_type &v) = 0;
	};

	template<class Image>
	class IdentityTransfer : public Transfer<Image>
	{
	public:
		RGB operator[](const typename Image::value_type &v) { return RGB(v); }
	};

	template<typename T>
	class IteratorAdapter
	{
	protected:
		T &t;
		typedef typename T::iterator iterator;
	public:
		IteratorAdapter(T &t_) : t(t_) {}
		iterator begin() { return t.begin(); }
		iterator end() { return t.end(); }
	};

	template<typename T>
	class ScalarIteratorAdapter
	{
	protected:
		const T &t;
		typedef const T *iterator;
	public:
		ScalarIteratorAdapter(const T &t_) : t(t_) {}
		iterator begin() { return &t; }
		iterator end() { return (&t) + 1; }
	};

	// specializations
	template<> class IteratorAdapter<int>    : public ScalarIteratorAdapter<int>    { public: IteratorAdapter(const int &t_)    : ScalarIteratorAdapter<int>(t_)    {} };
	template<> class IteratorAdapter<bool>   : public ScalarIteratorAdapter<bool>   { public: IteratorAdapter(const bool &t_)   : ScalarIteratorAdapter<bool>(t_)   {} };
	template<> class IteratorAdapter<double> : public ScalarIteratorAdapter<double> { public: IteratorAdapter(const double &t_) : ScalarIteratorAdapter<double>(t_) {} };
	template<> class IteratorAdapter<float>  : public ScalarIteratorAdapter<float>  { public: IteratorAdapter(const float &t_)  : ScalarIteratorAdapter<float>(t_)  {} };

	// for scalar values
	template<class Image>
	class NormalizedTransfer : public Transfer<Image>
	{
	protected:
		double range;	 ///< range of colors in this image
		double minn, maxx; ///< zero point of this image, maximum value of the image
		
		typedef typename Image::value_type V;
	public:
		// these functions return the minimum/maximum of a (possibly) vectorial quantity given in p and a double given in a
		// the IteratorAdapter class makes it possible for the same code to work on scalars as well
		// Note that this is incredibly convoluted and was shown to be a bad idea (tm), and should be completely redesigned
		//
		double minimum(double a, const V &p) { IteratorAdapter<V> ia(p); FOREACH(ia) { if(*i < a) { a = *i; } }; return a; }
		double maximum(double a, const V &p) { IteratorAdapter<V> ia(p); FOREACH(ia) { if(*i > a) { a = *i; } }; return a; }

		NormalizedTransfer(Image &img)
		{
			// find color range
			minn = maxx = *(IteratorAdapter<V>(*img.begin()).begin());
			FOREACH(img)
			{
				minn = minimum(minn, *i);
				maxx = maximum(maxx, *i);
			}
			range = maxx - minn;
		}

		RGB operator[](const typename Image::value_type &v) { return RGB((v - minn) / range); }
	};

	template<class Image>
	class LogTransfer : public Transfer<Image>
	{
	public:
		typedef typename Image::value_type S;
		S minn, maxx, range;
	public:
		LogTransfer(Image &img)
		{
			bool first = true;
			FOREACH(img)
			{
				if(*i <= 0) continue;
				if(first)
				{
					minn  = maxx = *i;
					first = false;
				}
				else
				{
					minn = std::min(minn, *i);
					maxx = std::max(maxx, *i);
				}
			}
			if(!first) {
				maxx = log(maxx);
				minn = log(minn);
				range = maxx - minn;
			}
		}

		RGB operator[](const typename Image::value_type &v)
		{
			if(v <= 0.) { return RGB(0.); }

			double lv = log(double(v));

			if( v > 10) {
			cerr << v << " " << lv << " " << (lv - minn) / range << " " << minn << " " << maxx << " " << range << "\n";
			}

			return RGB((lv - minn) / range);
		}
	};

	class Filter {
	public:
		virtual RGB operator[](const RGB &v) = 0;
	};

	class IdentityFilter : public Filter {
	public:
		static IdentityFilter I;
	public:
		virtual RGB operator[](const RGB &v) { return v; }
	};

#if 0
	// TODO: Finish implementing this filter
	class ColormapFilter : public Filter
	{
	protected:
		Filter &in;
		static auto_ptr<Config> conf;
	protected:
		typedef std::pair<double, double> cmap;
		struct cmap_less { bool operator(const cmap &a, const cmap &b) { return a.first < b.first; } }
		typedef std::vector<cmap> cmap_set;
		
		cmap_set colormaps[3];
	public:
		ColormapFilter(std::string name, Filter &in_ = IdentityFilter::I)
			: in(in_)
		{
			// locate colormap_name in some kind of colormap names database
			// load the map and sort it (it MUST be sorted)
			if(conf == NULL)
			{
				// load the configuration
			}

			if(!conf.count(name)) THROW(... something ...)
			
			... open colormap file ...
			... file format: R1 -> R2 lines .... one empty line ... G1 -> G2 lines ... one empty line ... etc.
		}

		RGB operator[](const RGB &v_)
		{
			RGB v = in[v_];
			FOR(0, 3)
			{
				cmap_set::iterator x2 = lower_bound(colormaps[i].begin(), colormaps[i].end(), v[i]);
				cmap_set::iterator x1 = x2-1;
				
				v[i] = (*x1).second + ((*x2).second - (*x1).second)/((*x2).first - (*x1).first) * (v[i] - (*x1).first);
			}
			return v[i];
		}
	};
#endif
	// this is just as a demonstration
	class RedFilter : public Filter {
	protected:
		Filter &in;
	public:
		RedFilter(Filter &in_ = IdentityFilter::I) : in(in_) {}

		RGB operator[](const RGB &v_) { RGB v = in[v_]; return RGB(v.x, 0., 0.); }
	};

	// Sine map
	class SineFilter : public Filter {
	protected:
		Filter &in;
	public:
		SineFilter(Filter &in_ = IdentityFilter::I) : in(in_) {}

		RGB operator[](const RGB &v_) { RGB v = in[v_]; if(v.x > .9) { std::cerr << v.x << "\n"; } return RGB((cos(v.x*ctn::pi2*10.+ctn::pi)+1)/2., 0., 0.); }
	};

	template<typename Image>
	void write(Image &img, const std::string &filename, Transfer<Image> *transfer = NULL, Filter *filter = &IdentityFilter::I)
	{
		Magick::Image image(Magick::Geometry(img.x(), img.y()), "black");
		image.classType(Magick::DirectClass);
		image.modifyImage();

		NormalizedTransfer<Image> idt(img);
		IdentityFilter idf;
		if(transfer == NULL) { transfer = &idt; }

		DEBUG(verbose, "Writing output image to [ " << filename << " ]");

		Magick::Pixels view(image);
		int cols = image.columns();
		int rows = image.rows();

		Magick::PixelPacket *pp, *pp0 = view.get(0, 0, cols, rows);

		pp = pp0; FOREACHj(p, img) {
			RGB v = (*filter)[(*transfer)[*p]] * (1 << 16);
			(*pp).red = (Magick::Quantum)v[0];
			(*pp).green = (Magick::Quantum)v[1];
			(*pp).blue = (Magick::Quantum)v[2];
			pp++;
		}

		view.sync();
		image.flip();
		image.write(filename);
	}

} // namespace magick
} // namespace io
} // namespace peyton

#define __peyton_io_magick peyton::io::magick

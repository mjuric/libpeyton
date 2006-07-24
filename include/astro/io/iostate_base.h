#ifndef iostate_base_h_
#define iostate_base_h_

#include <ios>

//////////////////////////////////////////////////////////////////////////////////////////////

namespace peyton {
namespace io {

	/**
		\brief Helper template for easy creation of statefull manipulators
		
		This template allows you to simply bind complex state information to any iostream.
		Keeping states is useful if you want to add special formatting to you own classes.
		For example:
		
			vector v;
			cout << vector::pretty << v << "\n";
			cout << vector::column << v << "\n";
		
		(in simple cases like this, state information can be kept in ios::pword(), this
		class is primarely for more complex cases - for a prime example of the use of 
		this class, see \c peyton::io::fpnumber class.)

		To make use of your manipulator, do something like:
		
			class vector {
				class iostate;
				...
			};
		
			class vector::iostate : public iostate_base<vector::iostate>
			{
				iostate(iostate &);
				...
			}

		To obtain a unique copy of vector::iostate, which is automatically bound to an
		opened iostream:
		
			vector::iostate &ios = vector::iostate::get_iostate(stream);
			
		And that's it. Creation and deletion of the class is automatic.
	*/
	template <typename Derived>
	class iostate_base
	{
	protected:
		typedef Derived * PDerived;
	public:
		/**
			\brief Returns the instance of the derived class, bound to the given stream
			
			If no instances are bound, one shall be autocreated and bound.
		*/
		static Derived &get_iostate(std::ios &stream);	// main interface function - use this in op<< and op>> to get an instance of Derived

	public:
		static int xalloc_idx;
		/// Internal function. This callback will be registered with the ostream to which this iostate is attached.
		static void callback(std::ios_base::event ev, std::ios_base& iosobj, int index);	

	public:
		/**
			\brief Called when ios::copyfmt() is called on the underlying stream.
			
			By default, the copy-constructor is called to create a new copy of the
			Derived object. Override to specify a more complex behavior.
		*/
		virtual Derived *copyfmt();
	};

//////////////////////////////////////////////////////////////////////////////////////////////

template <typename Derived>
int iostate_base<Derived>::xalloc_idx;

template <typename Derived>
void iostate_base<Derived>::callback(std::ios_base::event ev, std::ios_base& obj, int index)
{
	Derived *s = static_cast<Derived *>(obj.pword(index));

	switch(ev)
	{
	case std::ios_base::copyfmt_event:
		s = s->copyfmt();
		break;
	case std::ios_base::erase_event:
		delete s;
		s = NULL;
		break;
	}
}

template <typename Derived>
Derived &iostate_base<Derived>::get_iostate(std::ios &stream)
{
	if(xalloc_idx == -1) { xalloc_idx = std::ios_base::xalloc(); }

	PDerived &s = (PDerived &)(stream.pword(xalloc_idx));
	if(s != NULL) { return *s; }

	s = new Derived;
	stream.register_callback(Derived::callback, xalloc_idx);

	return *s;
}

template <typename Derived>
Derived *iostate_base<Derived>::copyfmt()
{
	// by default, just call the copy-constructor
	return new Derived(static_cast<Derived &>(*this));
}

}
}

#endif

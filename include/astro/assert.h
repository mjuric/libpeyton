#ifndef _astro_assert_h
#define _astro_assert_h

namespace peyton {
namespace system {

	/**
		\brief		This function is only to be used through ASSERT() macro
	*/
	bool assert_msg_message(const char *assertion, const char *func, const char *file, int line);
	bool assert_msg_abort();

} // namespace system
} // namespace peyton

//
// VERIFY(x) -- abort if condition (x) is not true.
//
#if defined(__GNUG__) || defined (__INTEL_COMPILER)
	#define VERIFY(cond) for(bool __nuke = !(cond); __nuke && peyton::system::assert_msg_message(#cond, __PRETTY_FUNCTION__, __FILE__, __LINE__); peyton::system::assert_msg_abort())
#else
	#define VERIFY(cond) for(bool __nuke = !(cond); __nuke && peyton::system::assert_msg_message(#cond, "<function_name_unsupported>"_, __FILE__, __LINE__); peyton::system::assert_msg_abort())
#endif

//
// ASSERT(x) -- abort if condition (x) is not true, disabled if NDEBUG is set
//
#ifndef NDEBUG
	#define ASSERT(cond)	VERIFY(cond)
#else
	#define ASSERT(cond)	if(0)
#endif

#endif // _astro_assert_h

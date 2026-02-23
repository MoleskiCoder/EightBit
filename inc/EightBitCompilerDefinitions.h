#pragma once

#include <cassert>

#ifdef _MSC_VER

#	define ASSUME(x)	__assume(x);

#	define LIKELY(x)	(x)
#	define UNLIKELY(x)	(x)

#	define UNREACHABLE	{ ASSUME(0); assert(false && "unreachable"); }

#elif defined(__GNUG__)

#	define ASSUME(x)	{ if (!x) __builtin_unreachable(); }

#	define LIKELY(x)	__builtin_expect(!!(x), 1)
#	define UNLIKELY(x)	__builtin_expect(!!(x), 0)

#	define UNREACHABLE	__builtin_unreachable();

#else

#	define ASSUME(x)	assert(x);

#	define LIKELY(x)	(x)
#	define UNLIKELY(x)	(x)

#	define UNREACHABLE	ASSUME(0)

#endif

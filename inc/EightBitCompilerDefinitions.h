#pragma once

#ifdef _MSC_VER

#	include <intrin.h>

#	define LIKELY(x)	(x)
#	define UNLIKELY(x)	(x)

#	define EIGHTBIT_PARITY(x)	(__popcnt(value) % 2)

#	define UNREACHABLE __assume(0)

#elif defined(__GNUG__)

#	include <x86intrin.h>

#	define LIKELY(x)	__builtin_expect(!!(x), 1)
#	define UNLIKELY(x)	__builtin_expect(!!(x), 0)

#	define EIGHTBIT_PARITY(x)	__builtin_parity(value)

#	define UNREACHABLE __builtin_unreachable();

#else
#	error Unknown compiler
#endif

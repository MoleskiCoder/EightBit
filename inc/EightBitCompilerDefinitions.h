#pragma once

#ifdef _MSC_VER
#	include <intrin.h>
#endif

#ifdef __GNUG__
#	include <x86intrin.h>
#endif

namespace EightBit {
	int countBits(uint8_t value);
	bool oddParity(uint8_t value);
	int findFirstSet(int value);
}

/*
Published in 1988, the C Programming Language 2nd Ed.
(by Brian W.Kernighan and Dennis M.Ritchie) mentions
this in exercise 2 - 9. On April 19, 2006 Don Knuth pointed
out to me that this method "was first published by Peter
Wegner in CACM 3 (1960), 322.
(Also discovered independently by Derrick Lehmer and published
in 1964 in a book edited by Beckenbach.)"
*/
inline int EightBit::countBits(uint8_t value) {
	int count; // c accumulates the total bits set in value
	for (count = 0; value; ++count)
		value &= value - 1; // clear the least significant bit set
	return count;
}

inline bool EightBit::oddParity(uint8_t value) {
	return countBits(value) % 2;
}

inline int EightBit::findFirstSet(int value) {
#ifdef _MSC_VER
	unsigned long index;
	if (_BitScanForward(&index, value))
		return index + 1;
	return 0;
#elif defined(__GNUG__)
	return __builtin_ffs(value);
#else
#	error Find first set not implemented
#endif
}

#ifdef _MSC_VER

#	define LIKELY(x)	(x)
#	define UNLIKELY(x)	(x)

#	define PARITY(x)	(__popcnt(x) % 2)

#	define UNREACHABLE __assume(0)

#elif defined(__GNUG__)

#	define LIKELY(x)	__builtin_expect(!!(x), 1)
#	define UNLIKELY(x)	__builtin_expect(!!(x), 0)

#	define PARITY(x)	__builtin_parity(x)

#	define UNREACHABLE __builtin_unreachable();

#else

#	define LIKELY(x)	(x)
#	define UNLIKELY(x)	(x)

#	define PARITY(x)	EightBit::oddParity(x)

#	define UNREACHABLE

#endif

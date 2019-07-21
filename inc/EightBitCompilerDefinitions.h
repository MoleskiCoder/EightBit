#pragma once

#include <cassert>
#include <bitset>

#ifdef _MSC_VER
#	include <intrin.h>
#endif

#ifdef __GNUG__
#	include <x86intrin.h>
#endif

namespace EightBit {
	[[nodiscard]] int countBits(uint8_t value) noexcept;
	[[nodiscard]] bool oddParity(uint8_t value) noexcept;
	[[nodiscard]] int findFirstSet(unsigned long value) noexcept;
	constexpr void assume(int expression);
}

inline int EightBit::countBits(uint8_t value) noexcept {
#ifdef _MSC_VER
	return __popcnt(value);
#else
	/*
	Published in 1988, the C Programming Language 2nd Ed.
	(by Brian W.Kernighan and Dennis M.Ritchie) mentions
	this in exercise 2 - 9. On April 19, 2006 Don Knuth pointed
	out to me that this method "was first published by Peter
	Wegner in CACM 3 (1960), 322.
	(Also discovered independently by Derrick Lehmer and published
	in 1964 in a book edited by Beckenbach.)"
	*/
	int count; // c accumulates the total bits set in value
	for (count = 0; value; ++count)
		value &= value - 1; // clear the least significant bit set
	return count;
#endif
}

inline bool EightBit::oddParity(const uint8_t value) noexcept {
	return countBits(value) % 2;
}

inline int EightBit::findFirstSet(const unsigned long value) noexcept {
#ifdef _MSC_VER
	unsigned long index;
	if (_BitScanForward(&index, value))
		return index + 1;
	return 0;
#elif defined(__GNUG__)
	return __builtin_ffs(value);
#else
	std::bitset<sizeof(unsigned long) * 8> bits(value);
	for (size_t i = bits.size() - 1; i >= 0; --i)
		if (bits.test(i))
			return i + 1;
	return 0;
#endif
}

inline constexpr void EightBit::assume(const int expression) {
#ifdef _MSC_VER
	__assume(expression);
#elif defined(__GNUG__)
	if (!expression)
		__builtin_unreachable();
#else
	assert(expression);
#endif
}

#define ASSUME(x)	EightBit::assume(x)

#ifdef _MSC_VER

#	define LIKELY(x)	(x)
#	define UNLIKELY(x)	(x)

#	define PARITY(x)	EightBit::oddParity(x)

#	define UNREACHABLE	{ ASSUME(0); throw std::exception("unreachable"); }

#elif defined(__GNUG__)

#	define LIKELY(x)	__builtin_expect(!!(x), 1)
#	define UNLIKELY(x)	__builtin_expect(!!(x), 0)

#	define PARITY(x)	__builtin_parity(x)

#	define UNREACHABLE	__builtin_unreachable();

#else

#	define LIKELY(x)	(x)
#	define UNLIKELY(x)	(x)

#	define PARITY(x)	EightBit::oddParity(x)

#	define UNREACHABLE	ASSUME(0)

#endif

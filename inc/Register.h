#pragma once

#include <cstdint>

#ifdef __BYTE_ORDER__
#	if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#		define HOST_LITTLE_ENDIAN
#	endif
#	if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#		define HOST_BIG_ENDIAN
#	endif
#else
#	if defined(_M_X64) || defined(_M_IX86)
#		define HOST_LITTLE_ENDIAN
#	else
#		define HOST_BIG_ENDIAN
#	endif
#endif

namespace EightBit {
	typedef union {
		struct {
#ifdef HOST_LITTLE_ENDIAN
			uint8_t low;
			uint8_t high;
#endif
#ifdef HOST_BIG_ENDIAN
			uint8_t high;
			uint8_t low;
#endif
		};
		uint16_t word;
	} register16_t;
}
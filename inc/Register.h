#pragma once

#include <cstdint>
#include <iostream>  
#include <iomanip>

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
	union register16_t {
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
		register16_t() noexcept : word(0) {}
		register16_t(const uint16_t w) noexcept : word(w) {}
		register16_t(const uint8_t l, const uint8_t h) noexcept : low(l), high(h) {}

		register16_t& operator++() noexcept {
			++word;
			return *this;
		}

		register16_t& operator--() noexcept {
			--word;
			return *this;
		}

		register16_t operator++(int) noexcept {
			register16_t temporary(*this);
			operator++();
			return temporary;
		}

		register16_t operator--(int) noexcept {
			register16_t temporary(*this);
			operator--();
			return temporary;
		}

		register16_t& operator+=(const register16_t rhs) noexcept {
			this->word += rhs.word;
			return *this;
		}

		register16_t& operator-=(const register16_t rhs) noexcept {
			this->word -= rhs.word;
			return *this;
		}
	};

	inline bool operator==(const register16_t lhs, const register16_t rhs) noexcept {
		return lhs.word == rhs.word;
	}

	inline bool operator!=(const register16_t lhs, const register16_t rhs) noexcept {
		return !(lhs == rhs);
	}

	inline register16_t operator+(register16_t lhs, const register16_t rhs) noexcept {
		lhs += rhs;
		return lhs;
	}

	inline register16_t operator-(register16_t lhs, const register16_t rhs) noexcept {
		lhs -= rhs;
		return lhs;
	}

	inline std::ostream& operator<<(std::ostream& output, const register16_t value) {  
		return output << std::hex << std::setw(4) << std::setfill('0') << value.word;
	}
}

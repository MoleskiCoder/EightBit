#pragma once

#include <cstdint>

#ifdef _MSC_VER
#	include <intrin.h>
#endif

#ifdef __GNUG__
#	include <x86intrin.h>
#endif

#if !(defined(_MSC_VER) || defined(__GNUG__))
#	include <bitset>
#endif

#include "Device.h"

namespace EightBit {
	class Chip : public Device {
	public:
		enum Bits {
			Bit0 = 1,
			Bit1 = Bit0 << 1,
			Bit2 = Bit1 << 1,
			Bit3 = Bit2 << 1,
			Bit4 = Bit3 << 1,
			Bit5 = Bit4 << 1,
			Bit6 = Bit5 << 1,
			Bit7 = Bit6 << 1,
			Bit8 = Bit7 << 1,
			Bit9 = Bit8 << 1,
			Bit10 = Bit9 << 1,
			Bit11 = Bit10 << 1,
			Bit12 = Bit11 << 1,
			Bit13 = Bit12 << 1,
			Bit14 = Bit13 << 1,
			Bit15 = Bit14 << 1,
			Bit16 = Bit15 << 1
		};

		enum Mask {
			Mask1 = Bit1 - 1,
			Mask2 = Bit2 - 1,
			Mask3 = Bit3 - 1,
			Mask4 = Bit4 - 1,
			Mask5 = Bit5 - 1,
			Mask6 = Bit6 - 1,
			Mask7 = Bit7 - 1,
			Mask8 = Bit8 - 1,
			Mask9 = Bit9 - 1,
			Mask10 = Bit10 - 1,
			Mask11 = Bit11 - 1,
			Mask12 = Bit12 - 1,
			Mask13 = Bit13 - 1,
			Mask14 = Bit14 - 1,
			Mask15 = Bit15 - 1,
			Mask16 = Bit16 - 1
		};

		[[nodiscard]] static constexpr uint8_t bit(const int which) noexcept { return 1 << which; }

		[[nodiscard]] static constexpr uint8_t setBit(uint8_t input, const int which) noexcept { return input | which; }
		[[nodiscard]] static constexpr uint8_t setBit(uint8_t input, const int which, const int condition) noexcept { return setBit(input, which, !!condition); }
		[[nodiscard]] static constexpr uint8_t setBit(uint8_t input, const int which, const uint32_t condition) noexcept { return setBit(input, which, !!condition); }
		[[nodiscard]] static constexpr uint8_t setBit(uint8_t input, const int which, const bool condition) noexcept { return condition ? setBit(input, which) : clearBit(input, which); }

		[[nodiscard]] static constexpr uint8_t clearBit(uint8_t input, const int which) noexcept { return input & ~which; }
		[[nodiscard]] static constexpr uint8_t clearBit(uint8_t input, const int which, const int condition) noexcept { return clearBit(input, which, !!condition); }
		[[nodiscard]] static constexpr uint8_t clearBit(uint8_t input, const int which, const uint32_t condition) noexcept { return clearBit(input, which, !!condition); }
		[[nodiscard]] static constexpr uint8_t clearBit(uint8_t input, const int which, const bool condition) noexcept { return setBit(input, which, !condition); }

		[[nodiscard]] static constexpr uint8_t highByte(int value) noexcept { return value >> 8; }
		[[nodiscard]] static constexpr uint8_t highByte(uint16_t value) noexcept { return highByte(static_cast<int>(value)); }

		[[nodiscard]] static constexpr uint8_t lowByte(int value) noexcept { return value & Mask::Mask8; }
		[[nodiscard]] static constexpr uint8_t lowByte(uint16_t value) noexcept { return lowByte(static_cast<int>(value)); }

		[[nodiscard]] static constexpr uint16_t promoteByte(uint8_t value) noexcept { return value << 8; }
		[[nodiscard]] static constexpr uint8_t demoteByte(uint16_t value) noexcept { return highByte(value); }

		[[nodiscard]] static uint16_t constexpr higherPart(uint16_t value) noexcept { return value & 0xff00; }
		[[nodiscard]] static uint8_t constexpr lowerPart(uint16_t value) noexcept { return lowByte(value); }

		[[nodiscard]] static uint16_t constexpr makeWord(uint8_t low, uint8_t high) noexcept { return promoteByte(high) | low; }

		[[nodiscard]] static constexpr auto highNibble(const int value) noexcept { return value >> 4; }
		[[nodiscard]] static constexpr auto lowNibble(const int value) noexcept { return value & Mask4; }

		[[nodiscard]] static constexpr auto higherNibble(const int value) noexcept { return value & 0xf0; }
		[[nodiscard]] static constexpr auto lowerNibble(const int value) noexcept { return lowNibble(value); }

		[[nodiscard]] static constexpr auto promoteNibble(const int value) noexcept { return value << 4; }
		[[nodiscard]] static constexpr auto demoteNibble(const int value) noexcept { return highNibble(value); }

		[[nodiscard]] static auto countBits(unsigned value) noexcept {
#if defined(_MSC_VER)
			return __popcnt(value);
#elif defined(__GNUG__)
			return __builtin_popcount(value);
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

		[[nodiscard]] static auto oddParity(const unsigned value) noexcept {
#ifdef __GNUG__
			return __builtin_parity(value);
#else
			return countBits(value) % 2;
#endif
		}

		[[nodiscard]] static auto evenParity(const unsigned value) noexcept { return !oddParity(value); }

		[[nodiscard]] static int findFirstSet(const unsigned long value) noexcept {
#if defined(_MSC_VER)
			unsigned long index = 0;
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

		Chip(const Chip& rhs) noexcept
		: Device(rhs) {}

	protected:
		Chip() noexcept = default;
	};
}

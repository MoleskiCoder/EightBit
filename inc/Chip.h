#pragma once

#include <cstdint>
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

		enum Masks {
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

		static constexpr uint8_t bit(const int which) noexcept { return 1 << which; }

		static void clearFlag(uint8_t& f, const int flag) noexcept { f &= ~flag; }
		static void setFlag(uint8_t& f, const int flag) noexcept { f |= flag; }

		static void setFlag(uint8_t& f, const int flag, const int condition) noexcept { setFlag(f, flag, !!condition); }
		static void setFlag(uint8_t& f, const int flag, const uint32_t condition) noexcept { setFlag(f, flag, !!condition); }
		static void setFlag(uint8_t& f, const int flag, const bool condition) noexcept { condition ? setFlag(f, flag) : clearFlag(f, flag); }

		static void clearFlag(uint8_t& f, const int flag, const int condition) noexcept { clearFlag(f, flag, !!condition); }
		static void clearFlag(uint8_t& f, const int flag, const uint32_t condition) noexcept { clearFlag(f, flag, !!condition); }
		static void clearFlag(uint8_t& f, const int flag, const bool condition) noexcept { setFlag(f, flag, !condition); }

		static constexpr auto highNibble(const int value) { return value >> 4; }
		static constexpr auto lowNibble(const int value) { return value & Mask4; }

		static constexpr auto higherNibble(const int value) { return value & 0xf0; }
		static constexpr auto lowerNibble(const int value) { return lowNibble(value); }

		static constexpr auto promoteNibble(const int value) { return value << 4; }
		static constexpr auto demoteNibble(const int value) { return highNibble(value); }

		virtual ~Chip();

	protected:
		Chip();
	};
}

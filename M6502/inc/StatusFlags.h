#pragma once

#include <string>
#include <cstdint>

struct StatusFlags {

	bool negative;
	bool overflow;
	bool reserved;
	bool brk;
	bool decimal;
	bool interrupt;
	bool zero;
	bool carry;

	enum StatusBits {
		Negative = 0x80,    // N
		Overflow = 0x40,    // V
		Reserved = 0x20,    // ignored
		Break = 0x10,       // B
		Decimal = 0x08,     // D (use BCD for arithmetic)
		Interrupt = 0x04,   // I (IRQ disable)
		Zero = 0x02,        // Z
		Carry = 0x01,       // C
	};

	StatusFlags(uint8_t value) {
		negative = (value & StatusBits::Negative) != 0;
		overflow = (value & StatusBits::Overflow) != 0;
		reserved = (value & StatusBits::Reserved) != 0;
		brk = (value & StatusBits::Break) != 0;
		decimal = (value & StatusBits::Decimal) != 0;
		interrupt = (value & StatusBits::Interrupt) != 0;
		zero = (value & StatusBits::Zero) != 0;
		carry = (value & StatusBits::Carry) != 0;
	}

	operator uint8_t() const {

		uint8_t flags = 0;

		if (negative)
			flags |= StatusBits::Negative;

		if (overflow)
			flags |= StatusBits::Overflow;

		if (reserved)
			flags |= StatusBits::Reserved;

		if (brk)
			flags |= StatusBits::Break;

		if (decimal)
			flags |= StatusBits::Decimal;

		if (interrupt)
			flags |= StatusBits::Interrupt;

		if (zero)
			flags |= StatusBits::Zero;

		if (carry)
			flags |= StatusBits::Carry;

		return flags;
	}

	operator std::string() const {
		std::string returned;
		returned += negative ? "N" : "-";
		returned += overflow ? "O" : "-";
		returned += reserved ? "R" : "-";
		returned += brk ? "B" : "-";
		returned += decimal ? "D" : "-";
		returned += interrupt ? "I" : "-";
		returned += zero ? "Z" : "-";
		returned += carry ? "C" : "-";
		return returned;
	}
};

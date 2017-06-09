#pragma once

#include <string>
#include <cstdint>

struct StatusFlags {

	bool SF;
	bool ZF;
	bool YF;
	bool HF;
	bool XF;
	bool PF;
	bool NF;
	bool CF;

	enum StatusBits {
		Sign = 0x80,				// SF
		Zero = 0x40,				// ZF
		YFlag = 0x20,				// YF
		HalfCarry = 0x10,			// HC
		XFlag= 0x8,					// XF
		Parity = 0x4,				// PF
		Overflow = 0x4,				// PF
		Subtract = 0x2,				// NF
		Carry = 0x1,				// CF
	};

	StatusFlags(uint8_t value = 0) {
		SF = (value & StatusBits::Sign) != 0;
		ZF = (value & StatusBits::Zero) != 0;
		YF = (value & StatusBits::YFlag) != 0;
		HF = (value & StatusBits::HalfCarry) != 0;
		XF = (value & StatusBits::XFlag) != 0;
		PF = (value & StatusBits::Parity) != 0;		// parity/overflow
		NF = (value & StatusBits::Subtract) != 0;
		CF = (value & StatusBits::Carry) != 0;
	}

	operator uint8_t() const {

		uint8_t flags = 0;

		if (SF)
			flags |= StatusBits::Sign;

		if (ZF)
			flags |= StatusBits::Zero;

		if (YF)
			flags |= StatusBits::YFlag;

		if (HF)
			flags |= StatusBits::HalfCarry;

		if (XF)
			flags |= StatusBits::XFlag;

		if (PF)
			flags |= StatusBits::Parity;

		if (NF)
			flags |= StatusBits::Subtract;

		if (CF)
			flags |= StatusBits::Carry;

		return flags;
	}

	operator std::string() const {
		std::string returned;
		returned += SF ? "S" : "-";
		returned += ZF ? "Z" : "-";
		returned += YF ? "Y" : "-";
		returned += HF ? "H" : "-";
		returned += XF ? "X" : "-";
		returned += PF ? "P" : "-";
		returned += NF ? "N" : "-";
		returned += CF ? "C" : "-";
		return returned;
	}
};

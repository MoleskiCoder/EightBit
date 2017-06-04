#pragma once

#include <string>
#include <cstdint>

namespace EightBit {
	struct StatusFlags {

		bool S;
		bool Z;
		bool AC;
		bool P;
		bool C;

		enum StatusBits {
			Sign = 0x80,				// S
			Zero = 0x40,				// Z
			AuxiliaryCarry = 0x10,		// AC
			Parity = 0x4,				// Z
			Carry = 0x1,				// S
		};

		StatusFlags(uint8_t value) {
			S = (value & StatusBits::Sign) != 0;
			Z = (value & StatusBits::Zero) != 0;
			AC = (value & StatusBits::AuxiliaryCarry) != 0;
			P = (value & StatusBits::Parity) != 0;
			C = (value & StatusBits::Carry) != 0;
		}

		operator uint8_t() const {

			uint8_t flags = 0;

			if (S)
				flags |= StatusBits::Sign;

			if (Z)
				flags |= StatusBits::Zero;

			flags &= ~0x20;		// Reserved off

			if (AC)
				flags |= StatusBits::AuxiliaryCarry;

			flags &= ~0x8;		// Reserved off

			if (P)
				flags |= StatusBits::Parity;

			flags |= 0x2;		// Reserved on

			if (C)
				flags |= StatusBits::Carry;

			return flags;
		}

		operator std::string() const {
			std::string returned;
			returned += S ? "S" : "-";
			returned += Z ? "Z" : "-";
			returned += "0";
			returned += AC ? "A" : "-";
			returned += "0";
			returned += P ? "P" : "-";
			returned += "1";
			returned += C ? "C" : "-";
			return returned;
		}
	};
}
#pragma once

#include <vector>
#include <fstream>

#include <Register.h>

namespace EightBit {
	class Z80;
}

namespace Fuse {
	class RegisterState {
	public:
		enum {
			AF, BC, DE, HL, AF_, BC_, DE_, HL_, IX, IY, SP, PC, MEMPTR, NUMBER_OF_REGISTERS
		};
		std::vector<EightBit::register16_t> registers;
		int i = -1, r = -1;
		bool iff1 = false, iff2 = false;
		int im = -1;
		bool halted = false;
		int tstates = -1;

	public:
		RegisterState();

		void read(std::ifstream& file);
		void readInternal(std::ifstream& file);
		void readExternal(std::ifstream& file);

		void transfer(EightBit::Z80& cpu) const;
	};
}
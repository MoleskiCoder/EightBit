#pragma once

#include <vector>
#include <string>
#include <fstream>

#include <Register.h>

namespace Fuse {
	class RegisterState {
	public:
		enum {
			AF, BC, DE, HL, AF_, BC_, DE_, HL_, IX, IY, SP, PC, MEMPTR, NUMBER_OF_REGISTERS
		};
		std::vector<EightBit::register16_t> registers;
		int i, r;
		bool iff1, iff2;
		int im;
		bool halted;
		int tstates;

	public:
		RegisterState();

		void read(std::ifstream& file);
		void readInternal(std::ifstream& file);
		void readExternal(std::ifstream& file);
	};
}
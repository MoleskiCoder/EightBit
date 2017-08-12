#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "Memory.h"

namespace Fuse {
	class RegisterState {
	public:
		enum {
			AF, BC, DE, HL, SP, PC, NUMBER_OF_REGISTERS
		};
		std::vector<EightBit::register16_t> registers;
		bool halted;
		int tstates;

	public:
		RegisterState();

		void read(std::ifstream& file);

	private:
		void readInternal(std::ifstream& file);
		void readExternal(std::ifstream& file);

		static std::string hex(int value);
	};
}
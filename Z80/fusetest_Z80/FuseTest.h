#pragma once

#include <string>
#include <vector>
#include <fstream>

#include "FuseMemoryDatum.h"
#include "FuseRegisterState.h"

#include <Memory.h>
#include <Z80.h>

namespace Fuse {
	class Test {
	public:
		std::string description;
		RegisterState registerState;
		std::vector<MemoryDatum> memoryData;
		bool finish = false;

		void read(std::ifstream& file);

		void transferMemory(EightBit::Memory& memory) const;
		void transferRegisters(EightBit::Z80& cpu) const;
	};
}
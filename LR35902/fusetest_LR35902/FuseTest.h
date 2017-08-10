#pragma once

#include <string>
#include <vector>
#include <fstream>

#include "FuseMemoryDatum.h"
#include "FuseRegisterState.h"

namespace Fuse {
	class Test {
	public:
		std::string description;
		RegisterState registerState;
		std::vector<MemoryDatum> memoryData;
		bool finish = false;

		void read(std::ifstream& file);
		void write(std::ofstream& file);
	};
}
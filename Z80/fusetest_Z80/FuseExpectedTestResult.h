#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <tuple>

#include "FuseTestEvents.h"
#include "FuseRegisterState.h"
#include "FuseMemoryDatum.h"

namespace Fuse {
	class ExpectedTestResult
	{
	public:
		std::string description;
		TestEvents events;
		RegisterState registerState;
		std::vector<MemoryDatum> memoryData;
		
		bool finish;

		ExpectedTestResult()
		: finish(false) {}

		void read(std::ifstream& file);

		// returns a vector of: address, expected, actual
		std::vector<std::tuple<int, int, int>> findDifferences(const EightBit::Memory& memory) const;
	};
}
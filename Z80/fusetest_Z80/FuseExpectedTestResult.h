#pragma once

#include <fstream>
#include <string>
#include <vector>

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
	};
}
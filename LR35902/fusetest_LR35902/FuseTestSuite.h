#pragma once

#include <string>

#include "FuseTests.h"
#include "FuseExpectedTestResults.h"

namespace Fuse {
	class TestSuite {
	private:
		Tests m_tests;
		ExpectedTestResults m_results;

	public:
		TestSuite(std::string path);

		void run();
	};
}
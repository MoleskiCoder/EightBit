#pragma once

#include <map>
#include <string>
#include <fstream>

#include "FuseExpectedTestResult.h"

namespace Fuse {
	class ExpectedTestResults {
	private:
		std::map<std::string, ExpectedTestResult> results;

		void read(std::ifstream& file);
		void write(std::ofstream& file);

	public:
		void read(std::string path);
		void write(std::string path);
		const std::map<std::string, ExpectedTestResult>& container() const {
			return results;
		}
	};
}
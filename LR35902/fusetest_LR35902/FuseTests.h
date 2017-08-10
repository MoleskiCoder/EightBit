#pragma once

#include <map>
#include <string>
#include <fstream>

#include "FuseTest.h"

namespace Fuse {
	class Tests {
	private:
		std::map<std::string, Test> tests;

		void read(std::ifstream& file);

	public:
		void read(std::string path);
		const std::map<std::string, Test>& container() const { return tests;  }
	};
}

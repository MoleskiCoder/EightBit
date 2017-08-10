#pragma once

#include <string>

namespace Fuse {
	class TestEvent
	{
	public:
		bool valid;
		std::string specifier;
		int cycles, address, value;

		TestEvent() 
		: valid(false),
		  cycles(-1), address(-1), value(-1) {
		}

		void read(std::ifstream& file);
		void write(std::ofstream& file);
	};
}
#pragma once

#include <cinttypes>
#include <vector>
#include <fstream>

namespace Fuse {
	class MemoryDatum {
	private:
		bool finish;

	public:
		int address;
		std::vector<uint8_t> bytes;

		MemoryDatum()
		: address(-1),
		  finish(false) {}

		bool finished() const { return finish; }
		void read(std::ifstream& file);
		void write(std::ofstream& file);
	};
}
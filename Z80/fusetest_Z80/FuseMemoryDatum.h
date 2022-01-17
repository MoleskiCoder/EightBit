#pragma once

#include <cinttypes>
#include <vector>
#include <fstream>
#include <tuple>

#include <Memory.h>

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

		void transfer(EightBit::Memory& memory) const;

		// returns a vector of: address, expected, actual
		std::vector<std::tuple<int, int, int>> findDifferences(const EightBit::Memory& memory) const;
	};
}
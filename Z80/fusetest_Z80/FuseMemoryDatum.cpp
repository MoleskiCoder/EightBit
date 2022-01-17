#include "stdafx.h"
#include "FuseMemoryDatum.h"

void Fuse::MemoryDatum::read(std::ifstream& file) {
	int begin;
	file >> begin;
	finish = (begin == -1);
	if (finish)
		return;

	address = begin;
	int byte;
	bool completed = false;
	do {
		file >> byte;
		completed = (byte == -1);
		if (!completed)
			bytes.push_back(byte);
	} while (!completed);
}

void Fuse::MemoryDatum::transfer(EightBit::Memory& memory) const {
	memory.load(bytes, address);
}

// returns a vector of: address, expected, actual
std::vector<std::tuple<int, int, int>> Fuse::MemoryDatum::findDifferences(const EightBit::Memory& memory) const {
	std::vector<std::tuple<int, int, int>> returned;
	for (int i = 0; i < bytes.size(); ++i) {
		const auto expected = bytes[i];
		int address = this->address + i;
		const auto actual = memory.peek(address);
		if (expected != actual)
			returned.push_back({ address, expected, actual });
	}
	return returned;;
}

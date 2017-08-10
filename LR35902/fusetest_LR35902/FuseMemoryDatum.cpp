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

void Fuse::MemoryDatum::write(std::ofstream& file) {
	file
		<< std::hex
		<< std::setfill('0');

	file << std::setw(4) << address << " ";
	for (auto byte : bytes) {
		file << std::setw(2) << (int)byte << " ";
	}
	file << std::dec << -1;
}
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
#include "stdafx.h"
#include "FuseTest.h"

void Fuse::Test::read(std::ifstream& file) {

	finish = false;
	do {
		std::getline(file, description);
		finish = file.eof();
	} while (description.empty() && !finish);

	if (finish)
		return;

	registerState.read(file);

	bool complete = false;
	do {
		MemoryDatum memoryDatum;
		memoryDatum.read(file);
		complete = memoryDatum.finished();
		if (!complete)
			memoryData.push_back(memoryDatum);
	} while (!complete);
}

void Fuse::Test::transferMemory(EightBit::Memory& memory) const {
	for (const auto& memoryDatum : memoryData)
		memoryDatum.transfer(memory);
}

void Fuse::Test::transferRegisters(EightBit::Z80 & cpu) const {
	registerState.transfer(cpu);
}

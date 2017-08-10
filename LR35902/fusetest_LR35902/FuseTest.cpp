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

void Fuse::Test::write(std::ofstream& file) {

	file << description << std::endl;

	registerState.write(file);
	file << std::endl;

	for (auto memoryDatum : memoryData) {
		memoryDatum.write(file);
		file << std::endl;
	}

	file << -1 << std::endl;
}

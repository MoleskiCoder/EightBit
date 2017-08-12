#include "stdafx.h"
#include "FuseTest.h"

void Fuse::Test::read(std::ifstream& file) {

	finish = false;
	do {
		std::getline(file, description);
		finish = file.eof();
		if (!description.empty() && (description[0] == ';'))	// ignore comments
			description.clear();
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
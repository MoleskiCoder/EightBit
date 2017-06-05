#include "stdafx.h"
#include "FuseExpectedTestResult.h"

#include <sstream>

/*
75
	0 MC 0000
	4 MR 0000 75
	4 MC a169
	7 MW a169 69
0200 cf98 90d8 a169 0000 0000 0000 0000 0000 0000 0000 0001 0000
00 01 0 0 0 0 7
a169 69 -1
*/
void Fuse::ExpectedTestResult::read(std::ifstream& file) {

	finish = false;
	do {
		std::getline(file, description);
		finish = file.eof();
	} while (description.empty() && !finish);

	if (finish)
		return;

	events.read(file);
	registerState.read(file);

	std::string line;
	std::getline(file, line);

	if (!line.empty())
		throw std::logic_error("EOL swallow failure!!");

	do {
		auto before = file.tellg();
		std::getline(file, line);
		if (!line.empty()) {

			file.seekg(before);

			MemoryDatum datum;
			datum.read(file);
			memoryData.push_back(datum);
		}
	} while (!line.empty());
}
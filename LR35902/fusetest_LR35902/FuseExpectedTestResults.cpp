#include "stdafx.h"
#include "FuseExpectedTestResults.h"

void Fuse::ExpectedTestResults::read(std::ifstream& file) {
	bool finished = false;
	while (!file.eof()) {
		ExpectedTestResult result;
		result.read(file);
		finished = result.finish;
		if (!finished)
			results[result.description] = result;
	}
}

void Fuse::ExpectedTestResults::read(std::string path) {
	std::ifstream file;
	file >> std::hex;
	file.open(path);
	read(file);
}
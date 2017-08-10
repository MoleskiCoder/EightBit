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

void Fuse::ExpectedTestResults::write(std::ofstream& file) {
	for (auto result : results) {
		result.second.write(file);
		file << std::endl << std::endl;
	}
}

void Fuse::ExpectedTestResults::read(std::string path) {
	std::ifstream file;
	file >> std::hex;
	file.open(path);
	read(file);
}

void Fuse::ExpectedTestResults::write(std::string path) {
	std::ofstream file;
	file << std::hex;
	file.open(path);
	write(file);
}

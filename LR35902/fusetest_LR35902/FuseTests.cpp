#include "stdafx.h"
#include "FuseTests.h"

void Fuse::Tests::read(std::ifstream& file) {
	bool finished = false;
	while (!file.eof()) {
		Test test;
		test.read(file);
		finished = test.finish;
		if (!finished)
			tests[test.description] = test;
	}
}

void Fuse::Tests::read(std::string path) {
	std::ifstream file;
	file >> std::hex;
	file.open(path);
	read(file);
}
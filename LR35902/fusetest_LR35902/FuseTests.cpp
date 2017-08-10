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

void Fuse::Tests::write(std::ofstream& file) {
	for (auto test : tests) {
		test.second.write(file);
		file << std::endl;
	}
}

void Fuse::Tests::read(std::string path) {
	std::ifstream file;
	file >> std::hex;
	file.open(path);
	read(file);
}

void Fuse::Tests::write(std::string path) {
	std::ofstream file;
	file.open(path);
	write(file);
}

#include "stdafx.h"
#include "FuseTestSuite.h"
#include "FuseTestRunner.h"

Fuse::TestSuite::TestSuite(std::string path) {
	m_tests.read(path + ".in");
	m_results.read(path + ".expected");
}

void Fuse::TestSuite::run() {
	auto failedCount = 0;
	auto unimplementedCount = 0;
	for (auto test : m_tests.container()) {

		auto key = test.first;
		std::cout << "** Checking: " << key << std::endl;

		auto input = test.second;
		auto result = m_results.container().find(key)->second;

		Fuse::TestRunner runner(input, result);

		runner.run();
		if (runner.failed())
			++failedCount;
		if (runner.unimplemented())
			++unimplementedCount;
	}
	std::cout << "+++ Failed test count: " << failedCount << std::endl;
	std::cout << "+++ Unimplemented test count: " << unimplementedCount << std::endl;
}
#include "stdafx.h"

#include <iostream>
#include <filesystem>

#include "TestRunner.h"
#include "test_t.h"
#include "opcode_test_suite_t.h"


int main() {
	std::filesystem::path location = "C:\\github\\spectrum\\libraries\\EightBit\\modules\\ProcessorTests\\6502\\v1";

    for (const auto& entry : std::filesystem::directory_iterator{ location }) {

        const auto path = entry.path();
        std::cout << "** path: " << path << std::endl;

        opcode_test_suite_t opcode(path.string());
        opcode.load();
        const auto& opcode_test_array = opcode.get_array();
        for (const auto& opcode_test_element : opcode_test_array) {

            const auto opcode_test = test_t(opcode_test_element);

            TestRunner runner(opcode_test);
            runner.run();
        }

        std::cout << "\n";
    }
}
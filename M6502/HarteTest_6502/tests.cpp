#include "stdafx.h"

#include <chrono>
#include <iostream>
#include <filesystem>

#include "TestRunner.h"
#include "test_t.h"
#include "opcode_test_suite_t.h"

int main() {

	std::filesystem::path location = "C:\\github\\spectrum\\libraries\\EightBit\\modules\\ProcessorTests\\6502\\v1";

    const auto start_time = std::chrono::steady_clock::now();

    int bad_opcode_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator{ location }) {

        const auto path = entry.path();

        std::cout << "Processing: " << path.filename() << "\n";
        opcode_test_suite_t opcode(path.string());
        opcode.load();
        opcode.parse();

        const auto opcode_test_array = opcode.raw().get_array();

        bool opcode_bad = false;
        for (const auto& opcode_test_element : opcode_test_array) {

            const auto opcode_test = test_t(opcode_test_element);

            TestRunner runner(opcode_test);
            const auto good = runner.check();
            if (!good) {
                if (!opcode_bad) {
                    std::cout << "** Failed: " << opcode_test.name() << "\n";
                    for (const auto& message : runner.messages())
                        std::cout << "**** " << message << "\n";
                    opcode_bad = true;
                }
            }
        }
        if (opcode_bad)
            ++bad_opcode_count;
    }

    const auto finish_time = std::chrono::steady_clock::now();
    const auto elapsed_time = finish_time - start_time;
    const auto seconds = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed_time).count();
    std::cout
        << "Elapsed time: " << seconds << " seconds"
        << ", bad opcode count: " << bad_opcode_count
        << std::endl;
}

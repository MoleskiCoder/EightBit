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

    int undocumented_opcode_count = 0;
    int unimplemented_opcode_count = 0;
    int invalid_opcode_count = 0;

    for (const auto& entry : std::filesystem::directory_iterator{ location }) {

        const auto path = entry.path();

        std::cout << "Processing: " << path.filename() << "\n";
        opcode_test_suite_t opcode(path.string());
        opcode.load();

        for (auto opcode_test_element : opcode) {

            const auto opcode_test = test_t(opcode_test_element);
            TestRunner runner(opcode_test);
            runner.check();

            auto invalid = runner.invalid();
            if (invalid) {
                ++invalid_opcode_count;
                if (runner.unimplemented())
                    ++unimplemented_opcode_count;
                if (runner.undocumented())
                    ++undocumented_opcode_count;
                std::cout << "** Failed: " << opcode_test.name() << "\n";
                for (const auto& message : runner.messages())
                    std::cout << "**** " << message << "\n";
                break;
            }
        }
    }

    const auto finish_time = std::chrono::steady_clock::now();
    const auto elapsed_time = finish_time - start_time;
    const auto seconds = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed_time).count();
    std::cout
        << "Elapsed time: " << seconds << " seconds"
        << ", undocumented opcode count: " << undocumented_opcode_count
        << ", unimplemented opcode count: " << unimplemented_opcode_count
        << ", invalid opcode count: " << (invalid_opcode_count - unimplemented_opcode_count)
        << std::endl;
}

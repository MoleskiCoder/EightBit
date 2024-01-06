#include "stdafx.h"

#include <chrono>
#include <iostream>
#include <filesystem>

#include "TestRunner.h"
#include "checker_t.h"
#include "test_t.h"
#include "opcode_test_suite_t.h"
#include "processor_test_suite_t.h"

int main() {

    auto directory = std::string("C:\\github\\spectrum\\libraries\\EightBit\\modules\\ProcessorTests\\6502\\v1");

    const auto start_time = std::chrono::steady_clock::now();

    int unimplemented_opcode_count = 0;
    int invalid_opcode_count = 0;

    TestRunner runner;
    runner.initialise();

    checker_t checker(runner);
    checker.initialise();

    processor_test_suite_t m6502_tests(directory);
    auto opcode_generator = m6502_tests.generator();
    while (opcode_generator) {

        auto opcode = opcode_generator();

        const auto path = std::filesystem::path(opcode.path());
        std::cout << "Processing: " << path.filename() << "\n";
        opcode.load();

        auto test_generator = opcode.generator();
        std::vector<std::string> test_names;
        while (test_generator) {

            const auto test = test_generator();
            checker.check(test);
            if (checker.invalid()) {

                std::cout << "** Failed: " << test.name() << "\n";

                ++invalid_opcode_count;

                // Was it just unimplemented?
                if (checker.unimplemented())
                    ++unimplemented_opcode_count;

                // Let's see if we had any successes!
                if (!test_names.empty()) {
                    std::cout << "**** The follow test variations succeeeded\n";
                    for (const auto& test_name : test_names)
                        std::cout << "****** " << test_name << "\n";
                }

                // OK, we've attempted an implementation, how did it fail?
                for (const auto& message : checker.messages())
                    std::cout << "**** " << message << "\n";

                // I'm not really interested in the remaining tests for this opcode
                break;
            }

            test_names.push_back(std::string(std::string_view(test.name())));
        }
    }

   const auto finish_time = std::chrono::steady_clock::now();
   const auto elapsed_time = finish_time - start_time;
   const auto seconds = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed_time).count();
   std::cout
       << "Elapsed time: " << seconds << " seconds"
       << ", unimplemented opcode count: " << unimplemented_opcode_count
       << ", invalid opcode count: " << (invalid_opcode_count - unimplemented_opcode_count)
       << std::endl;
}

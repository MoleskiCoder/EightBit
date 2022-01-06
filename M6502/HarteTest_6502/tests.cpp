#include "stdafx.h"

#include <chrono>
#include <iostream>
#include <filesystem>

#if __cplusplus < 202002L
#   include <boost/bind.hpp>
#endif

#include "TestRunner.h"
#include "checker_t.h"
#include "test_t.h"
#include "opcode_test_suite_t.h"
#include "processor_test_suite_t.h"

int main() {

    auto directory = std::string("C:\\github\\spectrum\\libraries\\EightBit\\modules\\ProcessorTests\\6502\\v1");

    const auto start_time = std::chrono::steady_clock::now();

    int undocumented_opcode_count = 0;
    int unimplemented_opcode_count = 0;
    int invalid_opcode_count = 0;

    TestRunner runner;
    runner.initialise();

    checker_t checker(runner);
    checker.initialise();

#ifdef USE_COROUTINES

#if __cplusplus >= 202002L

    processor_test_suite_t m6502_tests(directory);
    auto opcode_generator = m6502_tests.generator();
    while (opcode_generator) {

        auto opcode = opcode_generator();

        const auto path = std::filesystem::path(opcode.path());
        std::cout << "Processing: " << path.filename() << "\n";
        opcode.load();

        auto test_generator = opcode.generator();
        while (test_generator) {

            const auto test = test_generator();
            checker.check(test);

            if (checker.invalid()) {
                ++invalid_opcode_count;
                if (checker.unimplemented())
                    ++unimplemented_opcode_count;
                if (checker.undocumented())
                    ++undocumented_opcode_count;
                std::cout << "** Failed: " << test.name() << "\n";
                for (const auto& message : checker.messages())
                    std::cout << "**** " << message << "\n";
                break;
            }
        }
    }

#else

    const processor_test_suite_t m6502_tests(directory);
    boost::coroutines2::coroutine<opcode_test_suite_t>::pull_type opcodes(boost::bind(&processor_test_suite_t::generator, &m6502_tests, _1));
    for (auto& opcode : opcodes) {

        const auto path = std::filesystem::path(opcode.path());
        std::cout << "Processing: " << path.filename() << "\n";
        opcode.load();

        boost::coroutines2::coroutine<test_t>::pull_type tests(boost::bind(&opcode_test_suite_t::generator, &opcode, _1));
        for (const auto& test : tests) {

            checker.check(test);

            if (checker.invalid()) {
                ++invalid_opcode_count;
                if (checker.unimplemented())
                    ++unimplemented_opcode_count;
                if (checker.undocumented())
                    ++undocumented_opcode_count;
                std::cout << "** Failed: " << test.name() << "\n";
                for (const auto& message : checker.messages())
                    std::cout << "**** " << message << "\n";
                break;
            }
        }
    }

#endif

#else

    const processor_test_suite_t m6502_tests(directory);
    auto opcodes = m6502_tests.generate();
    for (auto& opcode : opcodes) {

        const auto path = std::filesystem::path(opcode.path());
        std::cout << "Processing: " << path.filename() << "\n";
        opcode.load();

        const auto tests = opcode.generate();
        for (const auto& test : tests) {

            checker.check(test);

            if (checker.invalid()) {
                ++invalid_opcode_count;
                if (checker.unimplemented())
                    ++unimplemented_opcode_count;
                if (checker.undocumented())
                    ++undocumented_opcode_count;
                std::cout << "** Failed: " << test.name() << "\n";
                for (const auto& message : checker.messages())
                    std::cout << "**** " << message << "\n";
                break;
            }
        }
    }

#endif

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

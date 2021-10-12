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

    for (const auto& entry : std::filesystem::directory_iterator{ location }) {

        const auto path = entry.path();

        const auto filename = path.filename();
        std::cout << "Processing: " << filename << "\n";

        opcode_test_suite_t opcode(path.string());
        opcode.load();

#ifdef USE_BOOST_JSON
        const auto& opcode_test_array = opcode.get_array();
#endif
#ifdef USE_NLOHMANN_JSON
        const auto& opcode_test_array = opcode.raw();
#endif
#ifdef USE_JSONCPP_JSON
        const auto& opcode_test_array = opcode.raw();
#endif
#ifdef USE_SIMDJSON_JSON
        assert(opcode.raw().is_array());
        const auto opcode_test_array = opcode.raw().get_array();
#endif

        bool opcode_bad = false;
        for (const auto& opcode_test_element : opcode_test_array) {

            const auto opcode_test = test_t(opcode_test_element);

            TestRunner runner(opcode_test);
#ifndef TEST_JSON_PERFORMANCE
            const auto good = runner.check();
            if (!good) {
                if (!opcode_bad) {
                    std::cout << "** Failed: " << opcode_test.name() << "\n";
                    for (const auto& message : runner.messages())
                        std::cout << "**** " << message << "\n";
                    opcode_bad = true;
                }
            }
#endif
        }
    }

    const auto finish_time = std::chrono::steady_clock::now();

    const auto elapsed_time = finish_time - start_time;

    const auto seconds = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed_time).count();

    std::cout << "Elapsed time: " << seconds << " seconds" << std::endl;
}
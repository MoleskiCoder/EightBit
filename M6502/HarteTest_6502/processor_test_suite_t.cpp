#include "stdafx.h"
#include "processor_test_suite_t.h"

#include <filesystem>

processor_test_suite_t::processor_test_suite_t(std::string location) noexcept
: m_location(location) {
}

EightBit::co_generator_t<opcode_test_suite_t> processor_test_suite_t::generator() {
    std::filesystem::path directory = location();
    for (const auto& entry : std::filesystem::directory_iterator{ directory })
        co_yield opcode_test_suite_t(entry.path().string());
}

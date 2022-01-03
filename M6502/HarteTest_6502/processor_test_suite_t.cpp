#include "stdafx.h"
#include "processor_test_suite_t.h"

#include <filesystem>

processor_test_suite_t::processor_test_suite_t(std::string location) noexcept
: m_location(location) {
}

#ifdef USE_COROUTINES
#if __cplusplus >= 202002L
EightBit::co_generator_t<opcode_test_suite_t> processor_test_suite_t::generator() {
    std::filesystem::path directory = location();
    for (const auto& entry : std::filesystem::directory_iterator{ directory })
        co_yield opcode_test_suite_t(entry.path().string());
}
#else
void processor_test_suite_t::generator(boost::coroutines2::coroutine<opcode_test_suite_t>::push_type& sink) {
    std::filesystem::path directory = location();
    for (const auto& entry : std::filesystem::directory_iterator{ directory })
        sink(opcode_test_suite_t(entry.path().string()));
}
#endif
#else
std::vector<opcode_test_suite_t> processor_test_suite_t::generate() {
    std::vector<opcode_test_suite_t> returned;
    std::filesystem::path directory = location();
    for (const auto& entry : std::filesystem::directory_iterator{ directory })
        returned.push_back(opcode_test_suite_t(entry.path().string()));
    return returned;
}
#endif

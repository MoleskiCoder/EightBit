#include "stdafx.h"
#include "opcode_test_suite_t.h"

#include <cassert>
#include <fstream>
#include <filesystem>

std::string opcode_test_suite_t::read(std::string path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    const auto size = std::filesystem::file_size(path);
    std::string result(size, '\0');
    file.read(result.data(), size);
    return result;
}

opcode_test_suite_t::opcode_test_suite_t(std::string path)
: m_path(path) {}

const boost::json::array& opcode_test_suite_t::get_array() const noexcept {
    assert(raw().is_array());
    return raw().get_array();
}

void opcode_test_suite_t::load() {
    const auto contents = read(path());
    m_raw = boost::json::parse(contents);
}

#include "stdafx.h"
#include "opcode_test_suite_t.h"

#include <cassert>
#include <exception>
#include <fstream>
#include <filesystem>

simdjson::dom::parser opcode_test_suite_t::m_parser;

std::string opcode_test_suite_t::read(std::string path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    const auto size = std::filesystem::file_size(path);
    std::string result(size, '\0');
    file.read(result.data(), size);
    return result;
}

opcode_test_suite_t::opcode_test_suite_t(std::string path)
: m_path(path) {}

void opcode_test_suite_t::load() {
    m_contents = read(path());
}

void opcode_test_suite_t::parse() {
    m_raw = m_parser.parse(m_contents);
    m_contents.clear();
    m_contents.shrink_to_fit();
}

#include "stdafx.h"
#include "opcode_test_suite_t.h"

#include <cassert>
#include <exception>
#include <fstream>
#include <filesystem>

simdjson::dom::parser opcode_test_suite_t::m_parser;

opcode_test_suite_t::opcode_test_suite_t(std::string path)
: m_path(path) {}

void opcode_test_suite_t::load() {
    m_raw = m_parser.load(path());
}

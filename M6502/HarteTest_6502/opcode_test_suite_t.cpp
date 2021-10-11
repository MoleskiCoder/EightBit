#include "stdafx.h"
#include "opcode_test_suite_t.h"

#include <cassert>
#include <exception>
#include <fstream>
#include <filesystem>

#ifdef USE_JSONCPP_JSON
std::unique_ptr<Json::CharReader> opcode_test_suite_t::m_reader;
#endif
#ifdef USE_SIMDJSON_JSON
std::unique_ptr<simdjson::dom::parser> opcode_test_suite_t::m_parser;
#endif

std::string opcode_test_suite_t::read(std::string path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    const auto size = std::filesystem::file_size(path);
    std::string result(size, '\0');
    file.read(result.data(), size);
    return result;
}

opcode_test_suite_t::opcode_test_suite_t(std::string path)
: m_path(path) {}

#ifdef USE_BOOST_JSON

const boost::json::array& opcode_test_suite_t::get_array() const noexcept {
    assert(raw().is_array());
    return raw().get_array();
}

void opcode_test_suite_t::load() {
    const auto contents = read(path());
    m_raw = boost::json::parse(contents);
}

#endif

#ifdef USE_NLOHMANN_JSON

void opcode_test_suite_t::load() {
    const auto contents = read(path());
    m_raw = nlohmann::json::parse(contents);
}

#endif

#ifdef USE_JSONCPP_JSON

void opcode_test_suite_t::load() {
    if (m_reader == nullptr) {
        Json::CharReaderBuilder builder;
        m_reader.reset(builder.newCharReader());
    }
    const auto contents = read(path());
    if (!m_reader->parse(contents.data(), contents.data() + contents.size(), &m_raw, nullptr))
        throw std::runtime_error("Unable to parse tests");
}

#endif

#ifdef USE_SIMDJSON_JSON

void opcode_test_suite_t::load() {
    if (m_parser == nullptr)
        m_parser = std::make_unique<simdjson::dom::parser>();
    const auto contents = read(path());
    m_raw = m_parser->parse(contents);
}

#endif

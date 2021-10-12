#include "stdafx.h"
#include "opcode_test_suite_t.h"

#include <cassert>
#include <exception>
#include <fstream>
#include <filesystem>

#ifdef JSON_PREFER_REUSE_OF_PARSER
#ifdef USE_JSONCPP_JSON
std::unique_ptr<Json::CharReader> opcode_test_suite_t::m_parser;
#endif
#ifdef USE_SIMDJSON_JSON
simdjson::dom::parser opcode_test_suite_t::m_parser;
#endif
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

void opcode_test_suite_t::load() {
    m_contents = read(path());
}

#ifdef USE_BOOST_JSON

const boost::json::array& opcode_test_suite_t::get_array() const noexcept {
    assert(raw().is_array());
    return raw().get_array();
}

void opcode_test_suite_t::parse() {
    m_raw = boost::json::parse(m_contents);
    m_contents.clear();
    m_contents.shrink_to_fit();}

#endif

#ifdef USE_NLOHMANN_JSON

void opcode_test_suite_t::parse() {
    m_raw = nlohmann::json::parse(m_contents);
    m_contents.clear();
    m_contents.shrink_to_fit();}

#endif

#ifdef USE_JSONCPP_JSON

void opcode_test_suite_t::parse() {
#ifdef JSON_PREFER_REUSE_OF_PARSER
    if (m_parser == nullptr)
        m_parser.reset(Json::CharReaderBuilder().newCharReader());
    if (!m_parser->parse(m_contents.data(), m_contents.data() + m_contents.size(), &m_raw, nullptr))
        throw std::runtime_error("Unable to parse tests");
#else
    std::unique_ptr<Json::CharReader> parser(Json::CharReaderBuilder().newCharReader());
    if (!parser->parse(m_contents.data(), m_contents.data() + m_contents.size(), &m_raw, nullptr))
        throw std::runtime_error("Unable to parse tests");
#endif
    m_contents.clear();
    m_contents.shrink_to_fit();
}

#endif

#ifdef USE_SIMDJSON_JSON

void opcode_test_suite_t::parse() {
    m_raw = m_parser.parse(m_contents);
    m_contents.clear();
    m_contents.shrink_to_fit();
}

#endif

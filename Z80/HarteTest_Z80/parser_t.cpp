#include "stdafx.h"
#include "parser_t.h"

simdjson::dom::parser parser_t::m_parser;

parser_t::parser_t(const std::string path) noexcept
: m_path(path) {}

void parser_t::load() {
    m_raw = m_parser.load(m_path);
}

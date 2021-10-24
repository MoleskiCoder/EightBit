#pragma once

#include <string>

#include "simdjson/simdjson.h"

class parser_t {
private:
    // N.B.
    // The parser must be kept for the lifetime of any parsed data.
    // Therefore, it can only be used for one document at a time.
    static simdjson::dom::parser m_parser;

    std::string m_path;
    simdjson::dom::element m_raw;

public:
    parser_t(std::string path) noexcept;

    [[nodiscard]] constexpr const auto& path() const noexcept { return m_path; }
    [[nodiscard]] const auto raw() const noexcept { return m_raw; }

    virtual void load();
};

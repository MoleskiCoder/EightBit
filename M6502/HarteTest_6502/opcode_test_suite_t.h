#pragma once

#include <string>

#include "simdjson/simdjson.h"

class opcode_test_suite_t final {
private:
    // N.B.
    // The parser must be kept for the lifetime of any parsed data.
    // Therefore, it can only be used for one document at a time.
    static simdjson::dom::parser m_parser;

    std::string m_path;
    simdjson::dom::array m_raw;;

public:
    opcode_test_suite_t(std::string path);

    [[nodiscard]] constexpr const auto& path() const noexcept { return m_path; }

    void load();

    [[nodiscard]] auto begin() const noexcept { return m_raw.begin(); }
    [[nodiscard]] auto end() const noexcept { return m_raw.end(); }
};

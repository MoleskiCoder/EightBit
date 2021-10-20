#pragma once

#include <string>

#include "simdjson/simdjson.h"

class opcode_test_suite_t final {
private:
    // N.B.
    // The parser must be kept for the lifetime of any parsed data.
    // Therefore, it can only be used for one document at a time.
    static simdjson::dom::parser m_parser;

    [[nodiscard]] static std::string read(std::string path);

    std::string m_path;
    std::string m_contents;
    simdjson::dom::element m_raw;

public:
    opcode_test_suite_t(std::string path);

    [[nodiscard]] constexpr const auto& path() const noexcept { return m_path; }
    [[nodiscard]] const auto raw() const noexcept { return m_raw; }

    void load();    // Reads into contents
    void parse();   // Parse the contents
};

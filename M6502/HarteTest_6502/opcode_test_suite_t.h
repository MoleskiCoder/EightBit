#pragma once

#include <string>
#include <boost/json.hpp>

class opcode_test_suite_t final {
private:
    static std::string read(std::string path);

    std::string m_path;
    boost::json::value m_raw;

public:
    opcode_test_suite_t(std::string path);

    constexpr const auto& path() const noexcept { return m_path; }
    constexpr const auto& raw() const noexcept { return m_raw; }
    const boost::json::array& get_array() const noexcept;

    void load();
};

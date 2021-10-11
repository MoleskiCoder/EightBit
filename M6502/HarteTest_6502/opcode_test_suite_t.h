#pragma once

#include <string>
#include <boost/json.hpp>

class opcode_test_suite_t final {
private:
    [[nodiscard]] static std::string read(std::string path);

    std::string m_path;
    boost::json::value m_raw;

public:
    opcode_test_suite_t(std::string path);

    [[nodiscard]] constexpr const auto& path() const noexcept { return m_path; }
    [[nodiscard]] constexpr const auto& raw() const noexcept { return m_raw; }
    [[nodiscard]] const boost::json::array& get_array() const noexcept;

    void load();
};

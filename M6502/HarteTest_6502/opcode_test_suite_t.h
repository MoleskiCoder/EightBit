#pragma once

#include <string>

#ifdef USE_BOOST_JSON
#	include <boost/json.hpp>
#endif

#ifdef USE_NLOHMANN_JSON
#   include "nlohmann/json.hpp"
#endif

class opcode_test_suite_t final {
private:
    [[nodiscard]] static std::string read(std::string path);

    std::string m_path;
#ifdef USE_BOOST_JSON
    boost::json::value m_raw;
#endif
#ifdef USE_NLOHMANN_JSON
    nlohmann::json m_raw;
#endif

public:
    opcode_test_suite_t(std::string path);

    [[nodiscard]] constexpr const auto& path() const noexcept { return m_path; }
    [[nodiscard]] constexpr const auto& raw() const noexcept { return m_raw; }

#ifdef USE_BOOST_JSON
    [[nodiscard]] const boost::json::array& get_array() const noexcept;
#endif

    void load();
};

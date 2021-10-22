#pragma once

#include <cstdint>
#include <string>

#include "simdjson/simdjson.h"

class element_t {
private:
    simdjson::dom::element m_raw;

protected:
    element_t(simdjson::dom::element input) noexcept;

    auto raw() const noexcept { return m_raw; }

    [[nodiscard]] auto at(std::string key) const noexcept { return raw()[key]; }
    [[nodiscard]] auto array_at(std::string key) const noexcept { return at(key).get_array(); }
    [[nodiscard]] auto integer_at(std::string key) const noexcept { return (int64_t)at(key); }
};

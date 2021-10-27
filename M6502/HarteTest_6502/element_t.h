#pragma once

#include <string_view>

#include "simdjson/simdjson.h"

class element_t {
private:
    simdjson::dom::element m_raw;

protected:
    element_t() noexcept;
    element_t(simdjson::dom::element input) noexcept;

    [[nodiscard]] auto raw() const noexcept { return m_raw; }

    [[nodiscard]] auto at(std::string_view key) const noexcept { return raw()[key]; }
    [[nodiscard]] auto operator[](std::string_view key) const noexcept { return at(key); }
    [[nodiscard]] auto array_at(std::string_view key) const noexcept { return at(key).get_array(); }
    [[nodiscard]] auto integer_at(std::string_view key) const noexcept { return at(key).get_int64(); }
};

#pragma once

#include "simdjson/simdjson.h"

#include "byte_t.h"

class ram_t final {
private:
    simdjson::dom::array m_raw;;

public:
    ram_t(simdjson::dom::array input);

    [[nodiscard]] auto begin() const noexcept { return m_raw.begin(); }
    [[nodiscard]] auto end() const noexcept { return m_raw.end(); }
};

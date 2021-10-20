#pragma once

#include <cstdint>
#include <vector>

#include "simdjson/simdjson.h"

#include "byte_t.h"

class ram_t final {
private:
    std::vector<byte_t> m_bytes;

public:
    ram_t(simdjson::dom::array input);

    void add(byte_t byte);

    [[nodiscard]] auto begin() const noexcept { return m_bytes.begin(); }
    [[nodiscard]] auto end() const noexcept { return m_bytes.end(); }

    [[nodiscard]] auto size() const noexcept { return m_bytes.size(); }

    void clear() noexcept { m_bytes.clear(); }

    [[nodiscard]] auto& operator[](size_t idx) noexcept { return m_bytes[idx]; }
    [[nodiscard]] auto operator[](size_t idx) const noexcept { return m_bytes[idx]; }
};

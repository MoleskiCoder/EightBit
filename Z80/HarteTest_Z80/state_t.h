#pragma once

#include <cstdint>
#include <string_view>

#include "simdjson/simdjson.h"

#include "element_t.h"
#include "ram_t.h"

class state_t final : public element_t {
public:
    state_t(const simdjson::dom::element input) noexcept
    : element_t(input) {}

    [[nodiscard]] auto address_at(std::string_view key) const noexcept { return uint16_t(integer_at(key)); }
    [[nodiscard]] auto byte_at(std::string_view key) const noexcept { return uint8_t(integer_at(key)); }

    [[nodiscard]] auto pc() const noexcept { return address_at("pc"); }
    [[nodiscard]] auto sp() const noexcept { return address_at("sp"); }

    [[nodiscard]] auto a() const noexcept { return byte_at("a"); }
    [[nodiscard]] auto f() const noexcept { return byte_at("f"); }
    [[nodiscard]] auto b() const noexcept { return byte_at("b"); }
    [[nodiscard]] auto c() const noexcept { return byte_at("c"); }
    [[nodiscard]] auto d() const noexcept { return byte_at("d"); }
    [[nodiscard]] auto e() const noexcept { return byte_at("e"); }
    [[nodiscard]] auto h() const noexcept { return byte_at("h"); }
    [[nodiscard]] auto l() const noexcept { return byte_at("l"); }

    [[nodiscard]] auto af_() const noexcept { return address_at("af_"); }
    [[nodiscard]] auto bc_() const noexcept { return address_at("bc_"); }
    [[nodiscard]] auto de_() const noexcept { return address_at("de_"); }
    [[nodiscard]] auto hl_() const noexcept { return address_at("hl_"); }

    [[nodiscard]] auto i() const noexcept { return byte_at("i"); }
    [[nodiscard]] auto r() const noexcept { return byte_at("r"); }

    [[nodiscard]] auto im() const noexcept { return byte_at("im"); }

    [[nodiscard]] auto ei() const noexcept { return byte_at("ei"); }

    [[nodiscard]] auto p() const noexcept { return byte_at("p"); }

    [[nodiscard]] auto q() const noexcept { return byte_at("q"); }

    [[nodiscard]] auto iff1() const noexcept { return byte_at("iff1"); }
    [[nodiscard]] auto iff2() const noexcept { return byte_at("iff2"); }

    [[nodiscard]] auto wz() const noexcept { return address_at("wz"); }

    [[nodiscard]] auto ix() const noexcept { return address_at("ix"); }
    [[nodiscard]] auto iy() const noexcept { return address_at("iy"); }

    [[nodiscard]] auto ram() const noexcept { return ram_t(array_at("ram")); }
};

#pragma once

#include <string>

#include <co_generator_t.h>

#include "parser_t.h"
#include "test_t.h"

class opcode_test_suite_t final : public parser_t {
private:
    [[nodiscard]] auto array() const noexcept { return raw().get_array(); }

public:
    opcode_test_suite_t() noexcept {}
    opcode_test_suite_t(std::string path) noexcept;

    [[nodiscard]] auto begin() const noexcept { return array().begin(); }
    [[nodiscard]] auto end() const noexcept { return array().end(); }
    [[nodiscard]] auto size() const noexcept { return array().size(); }

    [[nodiscard]] EightBit::co_generator_t<test_t> generator() const;
};

#pragma once

#include <string>

#include "parser_t.h"
#include "test_t.h"

#ifdef USE_COROUTINES
#include "co_generator_t.h"
#endif

class opcode_test_suite_t final : public parser_t {
private:
    [[nodiscard]] auto array() const noexcept { return raw().get_array(); }

public:
    opcode_test_suite_t() noexcept {}
    opcode_test_suite_t(std::string path) noexcept;

    [[nodiscard]] auto begin() const noexcept { return array().begin(); }
    [[nodiscard]] auto end() const noexcept { return array().end(); }

#ifdef USE_COROUTINES
    co_generator_t<test_t> generator();
#endif
};

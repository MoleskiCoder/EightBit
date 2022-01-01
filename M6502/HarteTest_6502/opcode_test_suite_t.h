#pragma once

#include <string>

#if __cplusplus >= 202002L
#   include <co_generator_t.h>
#else
#	include <boost/coroutine2/all.hpp>
#endif

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

#if __cplusplus >= 202002L
    [[nodiscard]] EightBit::co_generator_t<test_t> generator();
#else
    void generator(boost::coroutines2::coroutine<test_t>::push_type& sink);
#endif
};

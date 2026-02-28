#include "stdafx.h"
#include "opcode_test_suite_t.h"

opcode_test_suite_t::opcode_test_suite_t(const std::string path) noexcept
: parser_t(path) {}

EightBit::co_generator_t<test_t> opcode_test_suite_t::generator() const {
	for (const auto element : *this)
		co_yield test_t(element);
}
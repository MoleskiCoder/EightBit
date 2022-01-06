#include "stdafx.h"
#include "opcode_test_suite_t.h"

opcode_test_suite_t::opcode_test_suite_t(const std::string path) noexcept
: parser_t(path) {}

#ifdef USE_COROUTINES
#if __cplusplus >= 202002L
EightBit::co_generator_t<test_t> opcode_test_suite_t::generator() const {
	for (const auto element : *this)
		co_yield test_t(element);
}
#else
void opcode_test_suite_t::generator(boost::coroutines2::coroutine<test_t>::push_type& sink) const {
	for (const auto element : *this)
		sink(test_t(element));
}
#endif
#else
std::vector<test_t> opcode_test_suite_t::generate() const {
	std::vector<test_t> returned;
	returned.reserve(size());
	for (const auto element : *this)
		returned.push_back(test_t(element));
	return returned;
}
#endif
#pragma once

#include <string>
#include <string_view>

#ifdef USE_COROUTINES
#if __cplusplus >= 202002L
#	include <co_generator_t.h>
#else
#	include <boost/coroutine2/all.hpp>
#endif
#else
#	include <vector>
#endif

#include "opcode_test_suite_t.h"

class processor_test_suite_t final {
private:
	std::string m_location;

public:
	processor_test_suite_t(std::string location) noexcept;

#if __cplusplus >= 202002L
	[[nodiscard]] constexpr std::string_view location() const noexcept { return m_location; }
#else
	[[nodiscard]] std::string_view location() const noexcept { return m_location; }
#endif

#ifdef USE_COROUTINES
#if __cplusplus >= 202002L
	[[nodiscard]] EightBit::co_generator_t<opcode_test_suite_t> generator() const;
#else
	void generator(boost::coroutines2::coroutine<opcode_test_suite_t>::push_type& sink) const;
#endif
#else
	[[nodiscard]] std::vector<opcode_test_suite_t> generate() const;
#endif
};

#pragma once

#include <string>
#include <string_view>

#if __cplusplus >= 202002L
#	include <co_generator_t.h>
#else
#	include <boost/coroutine2/all.hpp>
#endif

#include "opcode_test_suite_t.h"

class processor_test_suite_t final {
private:
	std::string m_location;

public:
	processor_test_suite_t(std::string location) noexcept;

	std::string_view location() const noexcept { return m_location; }

#if __cplusplus >= 202002L
	[[nodiscard]] EightBit::co_generator_t<opcode_test_suite_t> generator();
#else
	void generator(boost::coroutines2::coroutine<opcode_test_suite_t>::push_type& sink);
#endif
};

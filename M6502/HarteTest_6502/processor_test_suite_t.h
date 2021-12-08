#pragma once

#include <string>
#include <string_view>

#ifdef USE_COROUTINES
#include "co_generator_t.h"
#endif

#include "opcode_test_suite_t.h"

class processor_test_suite_t final {
private:
	std::string m_location;

public:
	processor_test_suite_t(std::string location) noexcept;

	std::string_view location() const noexcept { return m_location; }

#ifdef USE_COROUTINES
	co_generator_t<opcode_test_suite_t> generator();
#endif
};

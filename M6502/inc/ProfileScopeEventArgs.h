#pragma once

#include <string>
#include <cstdint>

class ProfileScopeEventArgs
{
private:
	std::string m_scope;
	uint64_t m_cycles;
	uint64_t m_count;

public:
	ProfileScopeEventArgs(std::string scope, uint64_t cycles, uint64_t count)
	: m_scope(scope), m_cycles(cycles), m_count(count) {}

	const std::string& getScope() const	{ return m_scope;	}
	uint64_t getCycles() const			{ return m_cycles;	}
	uint64_t getCount() const			{ return m_count;	}
};


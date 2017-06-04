#pragma once

#include <string>
#include <cstdint>

class ProfileLineEventArgs
{
private:
	std::string m_source;
	uint64_t m_cycles;

public:
	ProfileLineEventArgs(std::string source, uint64_t cycles)
	: m_source(source), m_cycles(cycles) {}

	const std::string& getSource() const	{ return m_source;	}
	uint64_t getCycles() const				{ return m_cycles;	}
};
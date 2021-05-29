#pragma once

#include <string>
#include <cstdint>

namespace EightBit {
	class ProfileLineEventArgs
	{
	private:
		int m_address;
		std::string m_source;
		uint64_t m_cycles;

	public:
		ProfileLineEventArgs(int address, std::string source, uint64_t cycles)
		: m_address(address), m_source(source), m_cycles(cycles) {}

		int address() const { return m_address; }
		const std::string& source() const { return m_source; }
		uint64_t cycles() const { return m_cycles; }
	};
}
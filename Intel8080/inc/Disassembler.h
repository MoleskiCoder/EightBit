#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <boost/format.hpp>

#include <Bus.h>

#include "Intel8080.h"

namespace EightBit {
	class Disassembler {
	public:
		Disassembler(Bus& bus) noexcept;

		static std::string state(Intel8080& cpu);
		std::string disassemble(Intel8080& cpu);

		static std::string flag(uint8_t value, int flag, std::string represents, std::string off = "-");
		static std::string flags(uint8_t value);
		static std::string hex(uint8_t value);
		static std::string hex(uint16_t value);
		static std::string binary(uint8_t value);

	private:
		mutable boost::format m_formatter;
		Bus& m_bus;

		std::string disassemble(Intel8080& cpu, uint16_t pc);

		static std::string disassemble(
			int& dumpCount,
			int x, int y, int z,
			int p, int q);

		static std::string RP(int rp);
		static std::string RP2(int rp);
		static std::string R(int r);
		static std::string cc(int flag);
		static std::string alu(int which);
		static std::string alu2(int which);

		Bus& BUS() { return m_bus; }
	};
}
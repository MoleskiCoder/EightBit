#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <boost/format.hpp>

#include <Intel8080.h>

namespace EightBit {
	class Disassembler {
	public:
		Disassembler();

		static std::string state(const Intel8080& cpu);
		std::string disassemble(const Intel8080& cpu);

		static std::string flag(uint8_t value, int flag, std::string represents, std::string off = "-");
		static std::string flags(uint8_t value);
		static std::string hex(uint8_t value);
		static std::string hex(uint16_t value);
		static std::string binary(uint8_t value);

		static std::string invalid(uint8_t value);

	private:
		mutable boost::format m_formatter;

		void disassemble(std::ostringstream& output, const Intel8080& cpu, uint16_t pc);

		void disassemble(
			std::ostringstream& output,
			const Intel8080& cpu,
			uint16_t pc,
			std::string& specification,
			int& dumpCount,
			int x, int y, int z,
			int p, int q);

		std::string RP(int rp) const;
		std::string RP2(int rp) const;
		std::string R(int r) const;
		static std::string cc(int flag);
		static std::string alu(int which);
		static std::string alu2(int which);
	};
}
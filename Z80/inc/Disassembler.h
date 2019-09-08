#pragma once

#include <string>
#include <boost/format.hpp>
#include <Bus.h>

namespace EightBit {

	class Z80;

	class Disassembler final {
	public:
		Disassembler(Bus& bus) noexcept;

		static std::string state(Z80& cpu);
		std::string disassemble(Z80& cpu);

		static std::string flag(uint8_t value, int flag, const std::string& represents);
		static std::string flags(uint8_t value);
		static std::string hex(uint8_t value);
		static std::string hex(uint16_t value);
		static std::string binary(uint8_t value);
		static std::string decimal(uint8_t value);

		static std::string invalid(uint8_t value);

	private:
		mutable boost::format m_formatter;
		bool m_prefixCB = false;
		bool m_prefixDD = false;
		bool m_prefixED = false;
		bool m_prefixFD = false;
		bool m_displaced = false;
		uint8_t m_opcode = 0xff;
		Bus& m_bus;

		void disassemble(std::ostringstream& output, Z80& cpu, uint16_t pc);

		void disassembleCB(
			std::ostringstream& output,
			const Z80& cpu,
			uint16_t pc,
			std::string& specification,
			int& dumpCount,
			int x, int y, int z,
			int p, int q) const;

		void disassembleED(
			std::ostringstream& output,
			const Z80& cpu,
			uint16_t pc,
			std::string& specification,
			int& dumpCount,
			int x, int y, int z,
			int p, int q) const;

		void disassembleOther(
			std::ostringstream& output,
			Z80& cpu,
			uint16_t pc,
			std::string& specification,
			int& dumpCount,
			int x, int y, int z,
			int p, int q);

		std::string HL2() const;
		std::string RP(int rp) const;
		std::string RP2(int rp) const;
		std::string R(int r) const;
		static std::string cc(int flag);
		static std::string alu(int which);

		Bus& BUS() { return m_bus; }
	};
}
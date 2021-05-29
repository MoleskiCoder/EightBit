#pragma once

#include <cstdint>
#include <string>

#include <Bus.h>

#include "mc6809.h"

namespace EightBit {
	class Disassembly final {
	public:
		Disassembly(Bus& bus, mc6809& processor);

		[[nodiscard]] bool ignore();

		[[nodiscard]] std::string trace(uint16_t current);
		[[nodiscard]] std::string disassemble(uint16_t current);

		[[nodiscard]] std::string trace(register16_t current);
		[[nodiscard]] std::string disassemble(register16_t current);

		[[nodiscard]] std::string trace();
		[[nodiscard]] std::string disassemble();

		static [[nodiscard]] std::string dump_Flags(uint8_t value);
		static [[nodiscard]] std::string dump_ByteValue(uint8_t value);
		static [[nodiscard]] std::string dump_RelativeValue(int8_t value);
		static [[nodiscard]] std::string dump_WordValue(uint16_t value);
		static [[nodiscard]] std::string dump_WordValue(register16_t value);
		static [[nodiscard]] std::string dump_RelativeValue(int16_t value);
		static [[nodiscard]] std::string dump_RelativeValue(register16_t value);

	private:
		Bus& m_bus;
		mc6809& m_cpu;

		mutable uint16_t m_address = 0xffff;

		bool m_prefix10 = false;
		bool m_prefix11 = false;

		static void dump(std::ostream& out, int value, int width);

		[[nodiscard]] mc6809& CPU() { return m_cpu; }
		[[nodiscard]] Bus& BUS() { return m_bus; }

		[[nodiscard]] uint8_t getByte(uint16_t address);
		[[nodiscard]] uint16_t getWord(uint16_t address);

		[[nodiscard]] std::string disassembleUnprefixed();
		[[nodiscard]] std::string disassemble10();
		[[nodiscard]] std::string disassemble11();

		//

		[[nodiscard]] std::string RR(int which);
		[[nodiscard]] std::string wrapIndirect(std::string what, bool indirect);

		[[nodiscard]] std::string Address_direct(std::string mnemomic);
		[[nodiscard]] std::string Address_indexed(std::string mnemomic);
		[[nodiscard]] std::string Address_extended(std::string mnemomic);
		[[nodiscard]] std::string Address_relative_byte(std::string mnemomic);
		[[nodiscard]] std::string Address_relative_word(std::string mnemomic);

		[[nodiscard]] std::string AM_immediate_byte(std::string mnemomic);
		[[nodiscard]] std::string AM_immediate_word(std::string mnemomic);

		//

		[[nodiscard]] std::string branchShort(std::string mnemomic);
		[[nodiscard]] std::string branchLong(std::string mnemomic);

		//

		[[nodiscard]] std::string referenceTransfer8(int specifier);
		[[nodiscard]] std::string referenceTransfer16(int specifier);
		[[nodiscard]] std::string tfr(std::string mnemomic);

		//

		[[nodiscard]] std::string pulS();
		[[nodiscard]] std::string pulU();
		[[nodiscard]] std::string pshS();
		[[nodiscard]] std::string pshU();
		[[nodiscard]] std::string pulX(std::string mnemomic, std::string upon);
		[[nodiscard]] std::string pshX(std::string mnemomic, std::string upon);
	};
}
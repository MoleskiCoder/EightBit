#pragma once

#include <cstdint>
#include <string>

#include "mc6809.h"

namespace EightBit {
	class Disassembly final {
	public:
		Disassembly(mc6809& processor);

		bool ignore();

		std::string trace(uint16_t current);
		std::string trace(register16_t current);
		std::string trace();

	private:
		mc6809& m_cpu;

		mutable uint16_t m_address = 0xffff;

		bool m_prefix10 = false;
		bool m_prefix11 = false;

		static std::string dump_Flags(uint8_t value);
		static std::string dump_ByteValue(uint8_t value);
		static std::string dump_RelativeValue(int8_t value);
		static std::string dump_WordValue(uint16_t value);
		static std::string dump_WordValue(register16_t value);
		static std::string dump_RelativeValue(int16_t value);
		static std::string dump_RelativeValue(register16_t value);

		std::string disassemble(uint16_t current);
		std::string disassemble(register16_t current);
		std::string disassemble();

		static void dump(std::ostream& out, int value, int width);

		mc6809& CPU() { return m_cpu; }

		uint8_t getByte(uint16_t address);
		uint16_t getWord(uint16_t address);

		std::string disassembleUnprefixed();
		std::string disassemble10();
		std::string disassemble11();

		//

		std::string RR(int which);
		std::string wrapIndirect(std::string what, bool indirect);

		std::string Address_direct(std::string mnemomic);
		std::string Address_indexed(std::string mnemomic);
		std::string Address_extended(std::string mnemomic);
		std::string Address_relative_byte(std::string mnemomic);
		std::string Address_relative_word(std::string mnemomic);

		std::string AM_immediate_byte(std::string mnemomic);
		std::string AM_immediate_word(std::string mnemomic);

		//

		std::string branchShort(std::string mnemomic);
		std::string branchLong(std::string mnemomic);

		//

		std::string referenceTransfer8(int specifier);
		std::string referenceTransfer16(int specifier);
		std::string tfr(std::string mnemomic);

		//

		std::string pulS();
		std::string pulU();
		std::string pshS();
		std::string pshU();
		std::string pulX(std::string mnemomic, std::string upon);
		std::string pshX(std::string mnemomic, std::string upon);
	};
}
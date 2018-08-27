#pragma once

#include <cstdint>
#include <string>

#include "mc6809.h"

namespace EightBit {
	class Disassembly {
	public:
		Disassembly(mc6809& processor);

		std::string disassemble(uint16_t current);
		std::string disassemble(register16_t current);
		std::string disassemble();

		std::string dumpState();

		static std::string dump_Flags(uint8_t value);
		static std::string dump_ByteValue(uint8_t value);
		static std::string dump_RelativeValue(int8_t value);
		static std::string dump_WordValue(uint16_t value);
		static std::string dump_WordValue(register16_t value);
		static std::string dump_RelativeValue(int16_t value);
		static std::string dump_RelativeValue(register16_t value);

	private:
		mc6809& m_cpu;

		mutable uint16_t m_address = 0xffff;

		bool m_prefix10 = false;
		bool m_prefix11 = false;

		static void dump(std::ostream& out, int value, int width);

		mc6809& CPU() { return m_cpu; }

		uint8_t getByte(uint16_t address);
		uint16_t getWord(uint16_t address);

		std::string disassembleUnprefixed();
		std::string disassemble10();
		std::string disassemble11();

		//
		std::string RR(int which);

		std::string Address_indexed(std::string mnemomic);

		std::string AM_immediate_byte(std::string mnemomic);
		std::string AM_immediate_word(std::string mnemomic);

		std::string dump_Byte(uint16_t address);
		std::string dump_DByte(uint16_t address);
		std::string dump_Word(uint16_t address);
	};
}
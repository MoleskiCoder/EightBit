#pragma once

#include <cstdint>
#include <string>

#include "mc6809.h"

namespace EightBit {
	class Disassembly {
	public:
		Disassembly(mc6809& processor);

		std::string disassemble(uint16_t current) const;

		static std::string dump_Flags(uint8_t value);
		static std::string dump_ByteValue(uint8_t value);
		static std::string dump_WordValue(uint16_t value);

	private:
		mc6809& processor;

		mutable uint16_t m_address = 0xffff;

		static void dump(std::ostream& out, int value, int width);

		uint8_t getByte(uint16_t address) const;
		uint16_t getWord(uint16_t address) const;

		std::string dump_Byte(uint16_t address) const;
		std::string dump_DByte(uint16_t address) const;
		std::string dump_Word(uint16_t address) const;
	};
}
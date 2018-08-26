#include "stdafx.h"
#include "Disassembly.h"

#include <sstream>
#include <iomanip>
#include <functional>

using namespace std::placeholders;

EightBit::Disassembly::Disassembly(mc6809& targetProcessor)
:	processor(targetProcessor) {
}

std::string EightBit::Disassembly::dump_Flags(uint8_t value) {
	std::string returned;
	returned += (value & mc6809::EF) ? "E" : "-";
	returned += (value & mc6809::FF) ? "F" : "-";
	returned += (value & mc6809::HF) ? "H" : "-";
	returned += (value & mc6809::IF) ? "I" : "-";
	returned += (value & mc6809::NF) ? "N" : "-";
	returned += (value & mc6809::ZF) ? "Z" : "-";
	returned += (value & mc6809::VF) ? "V" : "-";
	returned += (value & mc6809::CF) ? "C" : "-";
	return returned;
}

void EightBit::Disassembly::dump(std::ostream& out, int value, int width) {
	out << std::hex << std::uppercase << std::setw(width) << std::setfill('0') << value;
}

std::string EightBit::Disassembly::dump_ByteValue(uint8_t value) {
	std::ostringstream output;
	dump(output, value, 2);
	return output.str();
}

std::string EightBit::Disassembly::dump_WordValue(uint16_t value) {
	std::ostringstream output;
	dump(output, value, 4);
	return output.str();
}

std::string EightBit::Disassembly::disassemble(uint16_t current) const {

	m_address = current;

	std::ostringstream output;

	auto& bus = processor.BUS();

	auto cell = bus.peek(current);

	output << dump_ByteValue(cell) << " ";

	return output.str();
}

////

uint8_t EightBit::Disassembly::getByte(uint16_t address) const {
	return processor.BUS().peek(address);
}

uint16_t EightBit::Disassembly::getWord(uint16_t address) const {
	return processor.BUS().peekWord(address);
}

////

std::string EightBit::Disassembly::dump_Byte(uint16_t address) const {
	return dump_ByteValue(getByte(address));
}

std::string EightBit::Disassembly::dump_DByte(uint16_t address) const {
	return dump_Byte(address) + " " + dump_Byte(address + 1);
}

std::string EightBit::Disassembly::dump_Word(uint16_t address) const {
	return dump_WordValue(getWord(address));
}
 
#include "stdafx.h"
#include "Disassembly.h"

#include <sstream>
#include <iomanip>
#include <functional>

using namespace std::placeholders;

Disassembly::Disassembly(MOS6502& targetProcessor, const Symbols& targetSymbols)
:	processor(targetProcessor),
	symbols(targetSymbols)
{
	dumpers = {
		{ AddressingMode::Illegal,				{ std::bind(&Disassembly::Dump_Nothing, this, std::placeholders::_1),	std::bind(&Disassembly::Dump_Nothing, this, std::placeholders::_1)	} },
		{ AddressingMode::Implied,				{ std::bind(&Disassembly::Dump_Nothing, this, std::placeholders::_1),	std::bind(&Disassembly::Dump_Nothing, this, std::placeholders::_1)	} },
		{ AddressingMode::Accumulator,			{ std::bind(&Disassembly::Dump_Nothing, this, std::placeholders::_1),	std::bind(&Disassembly::Dump_A, this, std::placeholders::_1)		} },
		{ AddressingMode::Immediate,			{ std::bind(&Disassembly::Dump_Byte, this, std::placeholders::_1),		std::bind(&Disassembly::Dump_imm, this, std::placeholders::_1)		} },
		{ AddressingMode::Relative,				{ std::bind(&Disassembly::Dump_Byte, this, std::placeholders::_1),		std::bind(&Disassembly::Dump_rel, this, std::placeholders::_1)		} },
		{ AddressingMode::XIndexed,				{ std::bind(&Disassembly::Dump_Byte, this, std::placeholders::_1),		std::bind(&Disassembly::Dump_xind, this, std::placeholders::_1)		} },
		{ AddressingMode::IndexedY,				{ std::bind(&Disassembly::Dump_Byte, this, std::placeholders::_1),		std::bind(&Disassembly::Dump_indy, this, std::placeholders::_1)		} },
		{ AddressingMode::ZeroPageIndirect,		{ std::bind(&Disassembly::Dump_Byte, this, std::placeholders::_1),		std::bind(&Disassembly::Dump_zpind, this, std::placeholders::_1)	} },
		{ AddressingMode::ZeroPage,				{ std::bind(&Disassembly::Dump_Byte, this, std::placeholders::_1),		std::bind(&Disassembly::Dump_zp, this, std::placeholders::_1)		} },
		{ AddressingMode::ZeroPageX,			{ std::bind(&Disassembly::Dump_Byte, this, std::placeholders::_1),		std::bind(&Disassembly::Dump_zpx, this, std::placeholders::_1)		} },
		{ AddressingMode::ZeroPageY,			{ std::bind(&Disassembly::Dump_Byte, this, std::placeholders::_1),		std::bind(&Disassembly::Dump_zpy, this, std::placeholders::_1)		} },
		{ AddressingMode::Absolute,				{ std::bind(&Disassembly::Dump_DByte, this, std::placeholders::_1),		std::bind(&Disassembly::Dump_abs, this, std::placeholders::_1)		} },
		{ AddressingMode::AbsoluteX,			{ std::bind(&Disassembly::Dump_DByte, this, std::placeholders::_1),		std::bind(&Disassembly::Dump_absx, this, std::placeholders::_1)		} },
		{ AddressingMode::AbsoluteY,			{ std::bind(&Disassembly::Dump_DByte, this, std::placeholders::_1),		std::bind(&Disassembly::Dump_absy, this, std::placeholders::_1)		} },
		{ AddressingMode::AbsoluteXIndirect,	{ std::bind(&Disassembly::Dump_DByte, this, std::placeholders::_1),		std::bind(&Disassembly::Dump_absxind, this, std::placeholders::_1)	} },
		{ AddressingMode::Indirect,				{ std::bind(&Disassembly::Dump_DByte, this, std::placeholders::_1),		std::bind(&Disassembly::Dump_ind, this, std::placeholders::_1)		} },
		{ AddressingMode::ZeroPageRelative,		{ std::bind(&Disassembly::Dump_DByte, this, std::placeholders::_1),		std::bind(&Disassembly::Dump_zprel, this, std::placeholders::_1)	} },
	};
}

std::string Disassembly::Dump_ByteValue(uint8_t value) const {
	std::ostringstream output;
	output << std::hex << std::setw(2) << std::setfill('0') << (int)value;
	return output.str();
}

std::string Disassembly::Dump_WordValue(uint16_t value) const {
	std::ostringstream output;
	output << std::hex << std::setw(4) << std::setfill('0') << (int)value;
	return output.str();
}

std::string Disassembly::DumpBytes(AddressingMode mode, uint16_t current) const {
	return getDumper(mode).byteDumper(current);
}

std::string Disassembly::Disassemble(uint16_t current) const {

	std::ostringstream output;

	auto content = processor.GetByte(current);
	const auto& instruction = processor.getInstruction(content);

	auto mode = instruction.mode;
	auto mnemomic = instruction.display;

	auto operand = DumpOperand(mode, current + 1);

	auto label = symbols.getLabels().find(current);
	if (label != symbols.getLabels().end())
		output << label->second << ": ";
	output << mnemomic << " " << operand;

	return output.str();
}

std::string Disassembly::DumpOperand(AddressingMode mode, uint16_t current) const {
	return getDumper(mode).disassemblyDumper(current);
}

////

uint8_t Disassembly::GetByte(uint16_t address) const {
	return processor.GetByte(address);
}

uint16_t Disassembly::GetWord(uint16_t address) const {
	return processor.GetWord(address);
}

////

std::string Disassembly::Dump_Nothing(uint16_t) const {
	return "";
}

std::string Disassembly::Dump_Byte(uint16_t address) const {
	return Dump_ByteValue(GetByte(address));
}

std::string Disassembly::Dump_DByte(uint16_t address) const {
	return Dump_Byte(address) + Dump_Byte(address + 1);
}

////

std::string Disassembly::ConvertAddress(uint16_t address) const {
	auto label = symbols.getLabels().find(address);
	if (label != symbols.getLabels().end())
		return label->second;
	std::ostringstream output;
	output << "$" << Dump_WordValue(address);
	return output.str();
}

std::string Disassembly::ConvertAddress(uint8_t address) const {
	auto label = symbols.getLabels().find(address);
	if (label != symbols.getLabels().end())
		return label->second;
	std::ostringstream output;
	output << "$" << Dump_ByteValue(address);
	return output.str();
}

std::string Disassembly::ConvertConstant(uint16_t constant) const {
	auto label = symbols.getConstants().find(constant);
	if (label != symbols.getConstants().end())
		return label->second;
	return Dump_DByte(constant);
}

std::string Disassembly::ConvertConstant(uint8_t constant) const {
	auto label = symbols.getConstants().find(constant);
	if (label != symbols.getConstants().end())
		return label->second;
	return Dump_ByteValue(constant);
}

////

std::string Disassembly::Dump_A(uint16_t) const {
	return "A";
}

std::string Disassembly::Dump_imm(uint16_t current) const {
	std::ostringstream output;
	auto immediate = GetByte(current);
	output << "#" << ConvertConstant(immediate);
	return output.str();
}

std::string Disassembly::Dump_abs(uint16_t current) const {
	auto address = GetWord(current);
	return ConvertAddress(address);
}

std::string Disassembly::Dump_zp(uint16_t current) const {
	auto zp = GetByte(current);
	return ConvertAddress(zp);
}

std::string Disassembly::Dump_zpx(uint16_t current) const {
	std::ostringstream output;
	auto zp = GetByte(current);
	output << ConvertAddress(zp) << ",X";
	return output.str();
}

std::string Disassembly::Dump_zpy(uint16_t current) const {
	std::ostringstream output;
	auto zp = GetByte(current);
	output << ConvertAddress(zp) << ",Y";
	return output.str();
}

std::string Disassembly::Dump_absx(uint16_t current) const {
	std::ostringstream output;
	auto address = GetWord(current);
	output << ConvertAddress(address) << ",X";
	return output.str();
}

std::string Disassembly::Dump_absy(uint16_t current) const {
	std::ostringstream output;
	auto address = GetWord(current);
	output << ConvertAddress(address) << ",Y";
	return output.str();
}

std::string Disassembly::Dump_absxind(uint16_t current) const {
	std::ostringstream output;
	auto address = GetWord(current);
	output << "(" << ConvertAddress(address) << ",X)";
	return output.str();
}

std::string Disassembly::Dump_xind(uint16_t current) const {
	std::ostringstream output;
	auto zp = GetByte(current);
	output << "(" << ConvertAddress(zp) << ",X)";
	return output.str();
}

std::string Disassembly::Dump_indy(uint16_t current) const {
	std::ostringstream output;
	auto zp = GetByte(current);
	output << "(" << ConvertAddress(zp) << "),Y";
	return output.str();
}

std::string Disassembly::Dump_ind(uint16_t current) const {
	std::ostringstream output;
	auto address = GetWord(current);
	output << "(" << ConvertAddress(address) << ")";
	return output.str();
}

std::string Disassembly::Dump_zpind(uint16_t current) const {
	std::ostringstream output;
	auto zp = GetByte(current);
	output << "(" << ConvertAddress(zp) << ")";
	return output.str();
}

std::string Disassembly::Dump_rel(uint16_t current) const {
	uint16_t relative = 1 + current + (int8_t)GetByte(current);
	return ConvertAddress(relative);
}

std::string Disassembly::Dump_zprel(uint16_t current) const {
	std::ostringstream output;
	auto zp = GetByte(current);
	int8_t displacement = GetByte(current + 1);
	uint16_t address = 1 + current + displacement;
	output << ConvertAddress(zp) << "," << ConvertAddress(address);
	return output.str();
}
 
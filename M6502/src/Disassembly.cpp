#include "stdafx.h"
#include "Disassembly.h"

#include <sstream>
#include <iomanip>
#include <functional>

using namespace std::placeholders;

EightBit::Disassembly::Disassembly(MOS6502& targetProcessor, const Symbols& targetSymbols)
:	processor(targetProcessor),
	symbols(targetSymbols) {
}

std::string EightBit::Disassembly::Dump_Flags(uint8_t value) const {
	std::string returned;
	returned += (value & MOS6502::NF) ? "N" : "-";
	returned += (value & MOS6502::VF) ? "O" : "-";
	returned += (value & MOS6502::RF) ? "R" : "-";
	returned += (value & MOS6502::BF) ? "B" : "-";
	returned += (value & MOS6502::DF) ? "D" : "-";
	returned += (value & MOS6502::IF) ? "I" : "-";
	returned += (value & MOS6502::ZF) ? "Z" : "-";
	returned += (value & MOS6502::CF) ? "C" : "-";
	return returned;
}

std::string EightBit::Disassembly::Dump_ByteValue(uint8_t value) const {
	std::ostringstream output;
	output << std::hex << std::setw(2) << std::setfill('0') << (int)value;
	return output.str();
}

std::string EightBit::Disassembly::Dump_WordValue(uint16_t value) const {
	std::ostringstream output;
	output << std::hex << std::setw(4) << std::setfill('0') << (int)value;
	return output.str();
}

std::string EightBit::Disassembly::Disassemble(uint16_t current) const {

	m_address = current;

	std::ostringstream output;

	auto memory = processor.getMemory();

	auto cell = memory.peek(current);

	output << Dump_ByteValue(cell);

	auto byte = memory.peek(current + 1);
	uint16_t relative = processor.PC().word + 2 + (int8_t)byte;

	auto aaa = (cell & 0b11100000) >> 5;
	auto bbb = (cell & 0b00011100) >> 2;
	auto cc = (cell & 0b00000011);

	switch (cc) {
	case 0b00:
		switch (aaa) {
		case 0b000:
			switch (bbb) {
			case 0b000:	// BRK
				output << disassemble_Implied("BRK");
				break;
			case 0b010:	// PHP
				output << disassemble_Implied("PHP");
				break;
			case 0b100:	// BPL
				output << disassemble_Relative("BPL", relative);
				break;
			case 0b110:	// CLC
				output << disassemble_Implied("CLC");
				break;
			default:
				throw std::domain_error("Illegal instruction");
			}
			break;
		case 0b001:
			switch (bbb) {
			case 0b000:	// JSR
				output << disassemble_Absolute("JSR");
				break;
			case 0b010:	// PLP
				output << disassemble_Implied("PLP");
				break;
			case 0b100:	// BMI
				output << disassemble_Relative("BMI", relative);
				break;
			case 0b110:	// SEC
				output << disassemble_Implied("SEC");
				break;
			default:	// BIT
				output << disassemble_AM_00(bbb, "BIT");
				break;
			}
			break;
		case 0b010:
			switch (bbb) {
			case 0b000:	// RTI
				output << disassemble_Implied("RTI");
				break;
			case 0b010:	// PHA
				output << disassemble_Implied("PHA");
				break;
			case 0b011:	// JMP
				output << disassemble_Absolute("JMP");
				break;
			case 0b100:	// BVC
				output << disassemble_Relative("BVC", relative);
				break;
			case 0b110:	// CLI
				output << disassemble_Implied("CLI");
				break;
			default:
				throw std::domain_error("Illegal addressing mode");
			}
			break;
		case 0b011:
			switch (bbb) {
			case 0b000:	// RTS
				output << disassemble_Implied("RTS");
				break;
			case 0b010:	// PLA
				output << disassemble_Implied("PLA");
				break;
			case 0b011:	// JMP (abs)
				output << disassemble_Indirect("JMP");
				break;
			case 0b100:	// BVS
				output << disassemble_Relative("BVS", relative);
				break;
			case 0b110:	// SEI
				output << disassemble_Implied("SEI");
				break;
			default:
				throw std::domain_error("Illegal addressing mode");
			}
			break;
		case 0b100:
			switch (bbb) {
			case 0b010:	// DEY
				output << disassemble_Implied("DEY");
				break;
			case 0b100:	// BCC
				output << disassemble_Relative("BCC", relative);
				break;
			case 0b110:	// TYA
				output << disassemble_Implied("TYA");
				break;
			default:	// STY
				output << disassemble_AM_00(bbb, "STY");
				break;
			}
			break;
		case 0b101:
			switch (bbb) {
			case 0b010:	// TAY
				output << disassemble_Implied("TAY");
				break;
			case 0b100:	// BCS
				output << disassemble_Relative("BCS", relative);
				break;
			case 0b110:	// CLV
				output << disassemble_Implied("CLV");
				break;
			default:	// LDY
				output << disassemble_AM_00(bbb, "LDY");
				break;
			}
			break;
		case 0b110:
			switch (bbb) {
			case 0b010:	// INY
				output << disassemble_Implied("INY");
				break;
			case 0b100:	// BNE
				output << disassemble_Relative("BNE", relative);
				break;
			case 0b110:	// CLD
				output << disassemble_Implied("CLD");
				break;
			default:	// CPY
				output << disassemble_AM_00(bbb, "CPY");
				break;
			}
			break;
		case 0b111:
			switch (bbb) {
			case 0b010:	// INX
				output << disassemble_Implied("INX");
				break;
			case 0b100:	// BEQ
				output << disassemble_Relative("BEQ", relative);
				break;
			case 0b110:	// SED
				output << disassemble_Implied("SED");
				break;
			default:	// CPX
				output << disassemble_AM_00(bbb, "CPX");
				break;
			}
			break;
		}
		break;
	case 0b01:
		switch (aaa) {
		case 0b000:		// ORA
			output << disassemble_AM_01(bbb, "ORA");
			break;
		case 0b001:		// AND
			output << disassemble_AM_01(bbb, "AND");
			break;
		case 0b010:		// EOR
			output << disassemble_AM_01(bbb, "EOR");
			break;
		case 0b011:		// ADC
			output << disassemble_AM_01(bbb, "ADC");
			break;
		case 0b100:		// STA
			output << disassemble_AM_01(bbb, "STA");
			break;
		case 0b101:		// LDA
			output << disassemble_AM_01(bbb, "LDA");
			break;
		case 0b110:		// CMP
			output << disassemble_AM_01(bbb, "CMP");
			break;
		case 0b111:		// SBC
			output << disassemble_AM_01(bbb, "SBC");
			break;
		default:
			__assume(0);
		}
		break;
	case 0b10:
		switch (aaa) {
		case 0b000:		// ASL
			output << disassemble_AM_10(bbb, "ASL");
			break;
		case 0b001:		// ROL
			output << disassemble_AM_10(bbb, "ROL");
			break;
		case 0b010:		// LSR
			output << disassemble_AM_10(bbb, "LSR");
			break;
		case 0b011:		// ROR
			output << disassemble_AM_10(bbb, "ROR");
			break;
		case 0b100:
			switch (bbb) {
			case 0b010:	// TXA
				output << disassemble_Implied("TXA");
				break;
			case 0b110:	// TXS
				output << disassemble_Implied("TXS");
				break;
			default:	// STX
				output << disassemble_AM_10_x(bbb, "STX");
				break;
			}
			break;
		case 0b101:
			switch (bbb) {
			case 0b010:	// TAX
				output << disassemble_Implied("TAX");
				break;
			case 0b110:	// TSX
				output << disassemble_Implied("TSX");
				break;
			default:	// LDX
				output << disassemble_AM_10_x(bbb, "LDX");
				break;
			}
			break;
		case 0b110:
			switch (bbb) {
			case 0b010:	// DEX
				output << disassemble_Implied("DEX");
				break;
			default:	// DEC
				output << disassemble_AM_10(bbb, "DEC");
				break;
			}
			break;
		case 0b111:
			switch (bbb) {
			case 0b010:	// NOP
				output << disassemble_Implied("NOP");
				break;
			default:	// INC
				output << disassemble_AM_10(bbb, "INC");
				break;
			}
			break;
		default:
			__assume(0);
		}
		break;
	case 0b11:
		throw std::domain_error("Illegal instruction group");
	default:
		__assume(0);
	}

	//const auto& instruction = processor.getInstruction(content);

	//auto mode = instruction.mode;
	//auto mnemomic = instruction.display;

	//auto operand = DumpOperand(mode, current + 1);

	//auto label = symbols.getLabels().find(current);
	//if (label != symbols.getLabels().end())
	//	output << label->second << ": ";
	//output << mnemomic << " " << operand;

	return output.str();
}

////

uint8_t EightBit::Disassembly::GetByte(uint16_t address) const {
	return processor.getMemory().peek(address);
}

////

std::string EightBit::Disassembly::Dump_Byte(uint16_t address) const {
	return Dump_ByteValue(GetByte(address));
}

std::string EightBit::Disassembly::Dump_DByte(uint16_t address) const {
	return Dump_Byte(address) + Dump_Byte(address + 1);
}

////

std::string EightBit::Disassembly::ConvertAddress(uint16_t address) const {
	auto label = symbols.getLabels().find(address);
	if (label != symbols.getLabels().end())
		return label->second;
	std::ostringstream output;
	output << "$" << Dump_WordValue(address);
	return output.str();
}

std::string EightBit::Disassembly::ConvertAddress(uint8_t address) const {
	auto label = symbols.getLabels().find(address);
	if (label != symbols.getLabels().end())
		return label->second;
	std::ostringstream output;
	output << "$" << Dump_ByteValue(address);
	return output.str();
}

std::string EightBit::Disassembly::ConvertConstant(uint16_t constant) const {
	auto label = symbols.getConstants().find(constant);
	if (label != symbols.getConstants().end())
		return label->second;
	return Dump_DByte(constant);
}

std::string EightBit::Disassembly::ConvertConstant(uint8_t constant) const {
	auto label = symbols.getConstants().find(constant);
	if (label != symbols.getConstants().end())
		return label->second;
	return Dump_ByteValue(constant);
}
 
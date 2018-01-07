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

std::string EightBit::Disassembly::dump_Flags(uint8_t value) const {
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

void EightBit::Disassembly::dump(std::ostream& out, int value, int width) {
	out << std::hex << std::uppercase << std::setw(width) << std::setfill('0') << value;
}

std::string EightBit::Disassembly::dump_ByteValue(uint8_t value) const {
	std::ostringstream output;
	dump(output, value, 2);
	return output.str();
}

std::string EightBit::Disassembly::dump_WordValue(uint16_t value) const {
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

	auto byte = bus.peek(current + 1);
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
			case 0b001:	// DOP/NOP (0x04)
				output << disassemble_AM_00(bbb, "*NOP");
				break;
			case 0b010:	// PHP
				output << disassemble_Implied("PHP");
				break;
			case 0b011:	// TOP/NOP (0b00001100, 0x0c)
				output << disassemble_AM_00(bbb, "*NOP");
				break;
			case 0b100:	// BPL
				output << disassemble_Relative("BPL", relative);
				break;
			case 0b101:	// DOP/NOP (0x14)
				output << disassemble_AM_00(bbb, "*NOP");
				break;
			case 0b110:	// CLC
				output << disassemble_Implied("CLC");
				break;
			case 0b111:	// TOP/NOP (0b00011100, 0x1c)
				output << disassemble_AM_00(bbb, "*NOP");
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
			case 0b101:	// DOP/NOP (0x34)
				output << disassemble_AM_00(bbb, "*NOP");
				break;
			case 0b110:	// SEC
				output << disassemble_Implied("SEC");
				break;
			case 0b111:	// TOP/NOP (0b00111100, 0x3c)
				output << disassemble_AM_00(bbb, "*NOP");
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
			case 0b001:	// DOP/NOP (0x44)
				output << disassemble_AM_00(bbb, "*NOP");
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
			case 0b101:	// DOP/NOP (0x54)
				output << disassemble_AM_00(bbb, "*NOP");
				break;
			case 0b110:	// CLI
				output << disassemble_Implied("CLI");
				break;
			case 0b111:	// TOP/NOP (0b01011100, 0x5c)
				output << disassemble_AM_00(bbb, "*NOP");
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
			case 0b001:	// DOP/NOP (0x64)
				output << disassemble_AM_00(bbb, "*NOP");
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
			case 0b101:	// DOP/NOP (0x74)
				output << disassemble_AM_00(bbb, "*NOP");
				break;
			case 0b110:	// SEI
				output << disassemble_Implied("SEI");
				break;
			case 0b111:	// TOP/NOP (0b01111100, 0x7c)
				output << disassemble_AM_00(bbb, "*NOP");
				break;
			default:
				throw std::domain_error("Illegal addressing mode");
			}
			break;
		case 0b100:
			switch (bbb) {
			case 0b000:	// DOP/NOP (0x80)
				output << disassemble_AM_00(bbb, "*NOP");
				break;
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
			case 0b101:	// DOP/NOP (0xd4)
				output << disassemble_AM_00(bbb, "*NOP");
				break;
			case 0b110:	// CLD
				output << disassemble_Implied("CLD");
				break;
			case 0b111:	// TOP/NOP (0b11011100, 0xdc)
				output << disassemble_AM_00(bbb, "*NOP");
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
			case 0b101:	// DOP/NOP (0xf4)
				output << disassemble_AM_00(bbb, "*NOP");
				break;
			case 0b110:	// SED
				output << disassemble_Implied("SED");
				break;
			case 0b111:	// TOP/NOP (0b11111100, 0xfc)
				output << disassemble_AM_00(bbb, "*NOP");
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
			UNREACHABLE;
		}
		break;
	case 0b10:
		switch (aaa) {
		case 0b000:		// ASL
			switch (bbb) {
			case 0b110:	// 0x1a
				output << disassemble_Implied("*NOP");
				break;
			default:
				output << disassemble_AM_10(bbb, "ASL");
				break;
			}
			break;
		case 0b001:		// ROL
			switch (bbb) {
			case 0b110:	// 0x3a
				output << disassemble_Implied("*NOP");
				break;
			default:
				output << disassemble_AM_10(bbb, "ROL");
				break;
			}
			break;
		case 0b010:		// LSR
			switch (bbb) {
			case 0b110:	// 0x5a
				output << disassemble_Implied("*NOP");
				break;
			default:
				output << disassemble_AM_10(bbb, "LSR");
				break;
			}
			break;
		case 0b011:		// ROR
			switch (bbb) {
			case 0b110:	// 0x7a
				output << disassemble_Implied("*NOP");
				break;
			default:
				output << disassemble_AM_10(bbb, "ROR");
				break;
			}
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
			case 0b110:	// 0xda
				output << disassemble_Implied("*NOP");
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
			case 0b110:	// 0xfa
				output << disassemble_Implied("*NOP");
				break;
			default:	// INC
				output << disassemble_AM_10(bbb, "INC");
				break;
			}
			break;
		default:
			UNREACHABLE;
		}
		break;
	case 0b11:
		switch (aaa) {
		case 0b000:
			output << disassemble_AM_01(bbb, "*SLO");
			break;
		case 0b001:
			output << disassemble_AM_01(bbb, "*RLA");
			break;
		case 0b010:
			output << disassemble_AM_01(bbb, "*SRE");
			break;
		case 0b100:
			output << disassemble_AM_11(bbb, "*SAX");
			break;
		case 0b101:
			output << disassemble_AM_11(bbb, "*LAX");
			break;
		case 0b110:
			output << disassemble_AM_11_x(bbb, "*DCP");
			break;
		case 0b111:
			switch (bbb) {
			case 0b000:	// *ISB
			case 0b001:
			case 0b011:
			case 0b100:
			case 0b101:
			case 0b110:
			case 0b111:
				output << disassemble_AM_01(bbb, "*ISB");
				break;
			case 0b010:
				output << disassemble_AM_11(bbb, "*SBC");
				break;
			default:
				UNREACHABLE;
			}
			break;
		default:
			throw std::domain_error("Illegal instruction group");
		}
		break;
	default:
		UNREACHABLE;
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

////

std::string EightBit::Disassembly::convertAddress(uint16_t address) const {
	auto label = symbols.getLabels().find(address);
	if (label != symbols.getLabels().end())
		return label->second;
	std::ostringstream output;
	output << "$" << dump_WordValue(address);
	return output.str();
}

std::string EightBit::Disassembly::convertAddress(uint8_t address) const {
	auto label = symbols.getLabels().find(address);
	if (label != symbols.getLabels().end())
		return label->second;
	std::ostringstream output;
	output << "$" << dump_ByteValue(address);
	return output.str();
}

std::string EightBit::Disassembly::convertConstant(uint16_t constant) const {
	auto label = symbols.getConstants().find(constant);
	if (label != symbols.getConstants().end())
		return label->second;
	return dump_DByte(constant);
}

std::string EightBit::Disassembly::convertConstant(uint8_t constant) const {
	auto label = symbols.getConstants().find(constant);
	if (label != symbols.getConstants().end())
		return label->second;
	return dump_ByteValue(constant);
}
 
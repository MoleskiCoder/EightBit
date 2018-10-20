#pragma once

#include <cstdint>
#include <string>

#include "mos6502.h"
#include "Symbols.h"

namespace EightBit {
	class Disassembly {
	public:
		Disassembly(Bus& bus, MOS6502& processor, const Symbols& symbols);

		std::string disassemble(uint16_t current);

		static std::string dump_Flags(uint8_t value);
		static std::string dump_ByteValue(uint8_t value);
		static std::string dump_WordValue(uint16_t value);

	private:
		Bus& m_bus;
		MOS6502& processor;
		const Symbols& symbols;

		mutable uint16_t m_address = 0xffff;

		std::string disassemble_Implied(const std::string& instruction) const {
			return "\t" + instruction;
		}

		std::string disassemble_Absolute(const std::string& instruction) {
			return AM_Absolute_dump() + "\t" + instruction + " " + AM_Absolute();
		}

		std::string disassemble_Indirect(const std::string& instruction) {
			return AM_Absolute_dump() + "\t" + instruction + " (" + AM_Absolute() + ")";
		}

		std::string disassemble_Relative(const std::string& instruction, uint16_t address) {
			return AM_Immediate_dump() + "\t" + instruction + " $" + dump_WordValue(address);
		}

		std::string disassemble_Immediate(const std::string& instruction) {
			return AM_Immediate_dump() + "\t" + instruction + " " + AM_Immediate();
		}

		std::string disassemble_AM_00(int bbb, const std::string& instruction) {
			return AM_00_dump(bbb) + "\t" + instruction + " " + AM_00(bbb);
		}

		std::string disassemble_AM_01(int bbb, const std::string& instruction) {
			return AM_01_dump(bbb) + "\t" + instruction + " " + AM_01(bbb);
		}

		std::string disassemble_AM_10(int bbb, const std::string& instruction) {
			return AM_10_dump(bbb) + "\t" + instruction + " " + AM_10(bbb);
		}

		std::string disassemble_AM_10_x(int bbb, const std::string& instruction) {
			return AM_10_x_dump(bbb) + "\t" + instruction + " " + AM_10_x(bbb);
		}

		std::string disassemble_AM_11(int bbb, const std::string& instruction) {
			return AM_11_dump(bbb) + "\t" + instruction + " " + AM_11(bbb);
		}

		std::string disassemble_AM_11_x(int bbb, const std::string& instruction) {
			return AM_11_x_dump(bbb) + "\t" + instruction + " " + AM_11_x(bbb);
		}

		std::string AM_Immediate_dump() {
			return dump_Byte(m_address + 1);
		}

		std::string AM_Immediate() {
			return "#$" + AM_Immediate_dump();
		}

		std::string AM_Absolute_dump() {
			return dump_DByte(m_address + 1);
		}

		std::string AM_Absolute() {
			return "$" + dump_Word(m_address + 1);
		}

		std::string AM_ZeroPage_dump() {
			return dump_Byte(m_address + 1);
		}

		std::string AM_ZeroPage() {
			return "$" + dump_Byte(m_address + 1);
		}

		std::string AM_ZeroPageX_dump() {
			return AM_ZeroPage_dump();
		}

		std::string AM_ZeroPageX() {
			return AM_ZeroPage() + ",X";
		}

		std::string AM_ZeroPageY_dump() {
			return AM_ZeroPage_dump();
		}

		std::string AM_ZeroPageY() {
			return AM_ZeroPage() + ",Y";
		}

		std::string AM_AbsoluteX_dump() {
			return AM_Absolute_dump();
		}

		std::string AM_AbsoluteX() {
			return AM_Absolute() + ",X";
		}

		std::string AM_AbsoluteY_dump() {
			return AM_Absolute_dump();
		}

		std::string AM_AbsoluteY() {
			return AM_Absolute() + ",Y";
		}

		std::string AM_IndexedIndirectX_dump() {
			return AM_ZeroPage_dump();
		}

		std::string AM_IndexedIndirectX() {
			return "($" + dump_Byte(m_address + 1) + ",X)";
		}

		std::string AM_IndirectIndexedY_dump() {
			return AM_ZeroPage_dump();
		}

		std::string AM_IndirectIndexedY() {
			return "($" + dump_Byte(m_address + 1) + "),Y";
		}

		std::string AM_00_dump(int bbb) {
			switch (bbb) {
			case 0b000:
				return AM_Immediate_dump();
			case 0b001:
				return AM_ZeroPage_dump();
			case 0b011:
				return AM_Absolute_dump();
			case 0b101:
				return AM_ZeroPageX_dump();
			case 0b111:
				return AM_AbsoluteX_dump();
			case 0b010:
			case 0b100:
			case 0b110:
				throw std::domain_error("Illegal addressing mode");
			default:
				UNREACHABLE;
			}
		}

		std::string AM_00(int bbb) {
			switch (bbb) {
			case 0b000:
				return AM_Immediate();
			case 0b001:
				return AM_ZeroPage();
			case 0b011:
				return AM_Absolute();
			case 0b101:
				return AM_ZeroPageX();
			case 0b111:
				return AM_AbsoluteX();
			case 0b010:
			case 0b100:
			case 0b110:
				throw std::domain_error("Illegal addressing mode");
			default:
				UNREACHABLE;
			}
		}

		std::string AM_01_dump(int bbb) {
			switch (bbb) {
			case 0b000:
				return AM_IndexedIndirectX_dump();
			case 0b001:
				return AM_ZeroPage_dump();
			case 0b010:
				return AM_Immediate_dump();
			case 0b011:
				return AM_Absolute_dump();
			case 0b100:
				return AM_IndirectIndexedY_dump();
			case 0b101:
				return AM_ZeroPageX_dump();
			case 0b110:
				return AM_AbsoluteY_dump();
			case 0b111:
				return AM_AbsoluteX_dump();
			default:
				UNREACHABLE;
			}
		}

		std::string AM_01(int bbb) {
			switch (bbb) {
			case 0b000:
				return AM_IndexedIndirectX();
			case 0b001:
				return AM_ZeroPage();
			case 0b010:
				return AM_Immediate();
			case 0b011:
				return AM_Absolute();
			case 0b100:
				return AM_IndirectIndexedY();
			case 0b101:
				return AM_ZeroPageX();
			case 0b110:
				return AM_AbsoluteY();
			case 0b111:
				return AM_AbsoluteX();
			default:
				UNREACHABLE;
			}
		}

		std::string AM_10_dump(int bbb) {
			switch (bbb) {
			case 0b000:
				return AM_Immediate_dump();
			case 0b001:
				return AM_ZeroPage_dump();
			case 0b010:
				return "";
			case 0b011:
				return AM_Absolute_dump();
			case 0b101:
				return AM_ZeroPageX_dump();
			case 0b111:
				return AM_AbsoluteX_dump();
			case 0b100:
			case 0b110:
				throw std::domain_error("Illegal addressing mode");
			default:
				UNREACHABLE;
			}
		}

		std::string AM_10(int bbb) {
			switch (bbb) {
			case 0b000:
				return AM_Immediate();
			case 0b001:
				return AM_ZeroPage();
			case 0b010:
				return "A";
			case 0b011:
				return AM_Absolute();
			case 0b101:
				return AM_ZeroPageX();
			case 0b111:
				return AM_AbsoluteX();
			case 0b100:
			case 0b110:
				throw std::domain_error("Illegal addressing mode");
			default:
				UNREACHABLE;
			}
		}

		std::string AM_10_x_dump(int bbb) {
			switch (bbb) {
			case 0b000:
				return AM_Immediate_dump();
			case 0b001:
				return AM_ZeroPage_dump();
			case 0b010:
				return "";
			case 0b011:
				return AM_Absolute_dump();
			case 0b101:
				return AM_ZeroPageY_dump();
			case 0b111:
				return AM_AbsoluteY_dump();
			case 0b100:
			case 0b110:
				throw std::domain_error("Illegal addressing mode");
			default:
				UNREACHABLE;
			}
		}

		std::string AM_10_x(int bbb) {
			switch (bbb) {
			case 0b000:
				return AM_Immediate();
			case 0b001:
				return AM_ZeroPage();
			case 0b010:
				return "A";
			case 0b011:
				return AM_Absolute();
			case 0b101:
				return AM_ZeroPageY();
			case 0b111:
				return AM_AbsoluteY();
			case 0b100:
			case 0b110:
				throw std::domain_error("Illegal addressing mode");
			default:
				UNREACHABLE;
			}
		}

		std::string AM_11_dump(int bbb) {
			switch (bbb) {
			case 0b000:
				return AM_IndexedIndirectX_dump();
			case 0b001:
				return AM_ZeroPage_dump();
			case 0b010:
				return AM_Immediate_dump();
			case 0b011:
				return AM_Absolute_dump();
			case 0b100:
				return AM_IndirectIndexedY_dump();
			case 0b101:
				return AM_ZeroPageY_dump();
			case 0b110:
				throw std::domain_error("Illegal addressing mode");
			case 0b111:
				return AM_AbsoluteY_dump();
			default:
				UNREACHABLE;
			}
		}

		std::string AM_11_x_dump(int bbb) {
			switch (bbb) {
			case 0b000:
				return AM_IndexedIndirectX_dump();
			case 0b001:
				return AM_ZeroPage_dump();
			case 0b010:
				return AM_Immediate_dump();
			case 0b011:
				return AM_Absolute_dump();
			case 0b100:
				return AM_IndirectIndexedY_dump();
			case 0b101:
				return AM_ZeroPageX_dump();
			case 0b110:
				return AM_AbsoluteY_dump();
			case 0b111:
				return AM_AbsoluteX_dump();
			default:
				UNREACHABLE;
			}
		}

		std::string AM_11(int bbb) {
			switch (bbb) {
			case 0b000:
				return AM_IndexedIndirectX();
			case 0b001:
				return AM_ZeroPage();
			case 0b010:
				return AM_Immediate();
			case 0b011:
				return AM_Absolute();
			case 0b100:
				return AM_IndirectIndexedY();
			case 0b101:
				return AM_ZeroPageY();
			case 0b110:
				throw std::domain_error("Illegal addressing mode");
			case 0b111:
				return AM_AbsoluteY();
			default:
				UNREACHABLE;
			}
		}

		std::string AM_11_x(int bbb) {
			switch (bbb) {
			case 0b000:
				return AM_IndexedIndirectX();
			case 0b001:
				return AM_ZeroPage();
			case 0b010:
				return AM_Immediate();
			case 0b011:
				return AM_Absolute();
			case 0b100:
				return AM_IndirectIndexedY();
			case 0b101:
				return AM_ZeroPageX();
			case 0b110:
				return AM_AbsoluteY();
			case 0b111:
				return AM_AbsoluteX();
			default:
				UNREACHABLE;
			}
		}

		static void dump(std::ostream& out, int value, int width);

		uint8_t getByte(uint16_t address);
		uint16_t getWord(uint16_t address);

		std::string dump_Byte(uint16_t address);
		std::string dump_DByte(uint16_t address);
		std::string dump_Word(uint16_t address);

		std::string convertAddress(uint16_t address) const;
		std::string convertAddress(uint8_t address) const;
		std::string convertConstant(uint16_t constant);
		std::string convertConstant(uint8_t constant) const;

		Bus& BUS() { return m_bus; }
	};
}
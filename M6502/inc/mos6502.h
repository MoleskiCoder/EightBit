#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <functional>
#include <cassert>

#include "Memory.h"
#include "Processor.h"
#include "Signal.h"

namespace EightBit {
	class MOS6502 : public Processor {
	public:
		struct opcode_decoded_t {

			int aaa;
			int bbb;
			int cc;

			opcode_decoded_t() {
				aaa = bbb = cc = 0;
			}

			opcode_decoded_t(uint8_t opcode) {
				aaa = (opcode & 0b11100000) >> 5;	// 0 - 7
				bbb = (opcode & 0b00011100) >> 2;	// 0 - 7
				cc = (opcode & 0b00000011);			// 0 - 3
			}
		};

		enum StatusBits {
			NF = Bit7,	// Negative
			VF = Bit6,  // Overflow
			RF = Bit5,  // reserved
			BF = Bit4,  // Brk
			DF = Bit3,	// D (use BCD for arithmetic)
			IF = Bit2,	// I (IRQ disable)
			ZF = Bit1,	// Zero
			CF = Bit0,	// Carry
		};

		MOS6502(Memory& memory);
		virtual ~MOS6502();

		Signal<MOS6502> ExecutingInstruction;
		Signal<MOS6502> ExecutedInstruction;

		uint8_t& X() { return x; }
		uint8_t& Y() { return y; }
		uint8_t& A() { return a; }
		uint8_t& S() { return s; }
		uint8_t& P() { return p; }

		virtual void initialise();

		virtual int step();

		virtual void reset();

		virtual void triggerIRQ();
		virtual void triggerNMI();

		void getWord(register16_t& output);
		void getWord(uint16_t offset, register16_t& output);

		uint8_t getByte() { return m_memory.read(); }
		uint8_t getByte(uint16_t offset) { return m_memory.read(offset); }

		void setByte(uint8_t value) { m_memory.write(value); }
		void setByte(uint16_t offset, uint8_t value) { m_memory.write(offset, value); }

	protected:
		virtual void interrupt(uint16_t vector);

		virtual int execute(uint8_t opcode);

	private:
		register16_t& MEMPTR() { return m_memptr; }

		void adjustZero(uint8_t datum) { clearFlag(P(), ZF, datum); }
		void adjustNegative(uint8_t datum) { setFlag(P(), NF, datum & NF); }
		
		void adjustNZ(uint8_t datum) {
			adjustZero(datum);
			adjustNegative(datum);
		}

		void pushByte(uint8_t value);
		uint8_t popByte();
		void pushWord(register16_t value);
		void popWord(register16_t& output);

#pragma region 6502 addressing modes

#pragma region Addresses

		void Address_Absolute() {
			fetchWord(MEMPTR());
		}

		void Address_ZeroPage() {
			MEMPTR().low = fetchByte();
			MEMPTR().high = 0;
		}

		void Address_ZeroPageIndirect() {
			Address_ZeroPage();
			m_memory.ADDRESS() = MEMPTR();
			getWord(MEMPTR());
		}

		void Address_Indirect() {
			Address_Absolute();
			m_memory.ADDRESS() = MEMPTR();
			getWord(MEMPTR());
		}

		void Address_ZeroPageX() {
			Address_ZeroPage();
			MEMPTR().low += X();
		}

		void Address_ZeroPageY() {
			Address_ZeroPage();
			MEMPTR().low += Y();
		}

		void Address_AbsoluteX() {
			Address_Absolute();
			MEMPTR().word += X();
		}

		void Address_AbsoluteY() {
			Address_Absolute();
			MEMPTR().word += Y();
		}

		void Address_IndexedIndirectX() {
			Address_ZeroPageX();
			m_memory.ADDRESS() = MEMPTR();
			getWord(MEMPTR());
		}

		void Address_IndirectIndexedY() {
			Address_ZeroPageIndirect();
			MEMPTR().word += Y();
		}

#pragma endregion Addresses

#pragma region Addressing modes, read

		uint8_t AM_A() {
			return A();
		}

		uint8_t AM_Immediate() {
			return fetchByte();
		}

		uint8_t AM_Absolute() {
			Address_Absolute();
			return m_memory.read(MEMPTR());
		}

		uint8_t AM_ZeroPage() {
			Address_ZeroPage();
			return m_memory.read(MEMPTR());
		}

		uint8_t AM_AbsoluteX() {
			Address_AbsoluteX();
			m_memory.ADDRESS() = MEMPTR();
			if (m_memory.ADDRESS().low == Mask8)
				++cycles;
			return getByte();
		}

		uint8_t AM_AbsoluteY() {
			Address_AbsoluteY();
			m_memory.ADDRESS() = MEMPTR();
			if (m_memory.ADDRESS().low == Mask8)
				++cycles;
			return getByte();
		}

		uint8_t AM_ZeroPageX() {
			Address_ZeroPageX();
			return m_memory.read(MEMPTR());
		}

		uint8_t AM_ZeroPageY() {
			Address_ZeroPageY();
			return m_memory.read(MEMPTR());
		}

		uint8_t AM_IndexedIndirectX() {
			Address_IndexedIndirectX();
			return m_memory.read(MEMPTR());
		}

		uint8_t AM_IndirectIndexedY() {
			Address_IndirectIndexedY();
			m_memory.ADDRESS() = MEMPTR();
			if (m_memory.ADDRESS().low == Mask8)
				++cycles;
			return getByte();
		}

#pragma endregion Addressing modes, read

#pragma region Addressing modes, write

		void AM_A(uint8_t value) {
			A() = value;
		}

		void AM_Absolute(uint8_t value) {
			Address_Absolute();
			m_memory.write(MEMPTR(), value);
		}

		void AM_ZeroPage(uint8_t value) {
			Address_ZeroPage();
			m_memory.write(MEMPTR(), value);
		}

		void AM_AbsoluteX(uint8_t value) {
			Address_AbsoluteX();
			m_memory.write(MEMPTR(), value);
		}

		void AM_AbsoluteY(uint8_t value) {
			Address_AbsoluteY();
			m_memory.write(MEMPTR(), value);
		}

		void AM_ZeroPageX(uint8_t value) {
			Address_ZeroPageX();
			m_memory.write(MEMPTR(), value);
		}

		void AM_ZeroPageY(uint8_t value) {
			Address_ZeroPageY();
			m_memory.write(MEMPTR(), value);
		}

		void AM_IndexedIndirectX(uint8_t value) {
			Address_IndexedIndirectX();
			m_memory.write(MEMPTR(), value);
		}

		void AM_IndirectIndexedY(uint8_t value) {
			Address_IndirectIndexedY();
			m_memory.write(MEMPTR(), value);
		}

#pragma endregion Addressing modes, write

#pragma region 6502 addressing mode switching

		uint8_t AM_00(int bbb) {
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
				__assume(0);
			}
		}

		void AM_00(int bbb, uint8_t value) {
			switch (bbb) {
			case 0b000:
				assert(false);
				break;
			case 0b001:
				AM_ZeroPage(value);
				break;
			case 0b011:
				AM_Absolute(value);
				break;
			case 0b101:
				AM_ZeroPageX(value);
				break;
			case 0b111:
				AM_AbsoluteX(value);
				break;
			case 0b010:
			case 0b100:
			case 0b110:
				throw std::domain_error("Illegal addressing mode");
			default:
				__assume(0);
			}
		}

		uint8_t AM_01(int bbb) {
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
				__assume(0);
			}
		}

		void AM_01(int bbb, uint8_t value) {
			switch (bbb) {
			case 0b000:
				AM_IndexedIndirectX(value);
				break;
			case 0b001:
				AM_ZeroPage(value);
				break;
			case 0b010:
				assert(false);
				break;
			case 0b011:
				AM_Absolute(value);
				break;
			case 0b100:
				AM_IndirectIndexedY(value);
				break;
			case 0b101:
				AM_ZeroPageX(value);
				break;
			case 0b110:
				AM_AbsoluteY(value);
				break;
			case 0b111:
				AM_AbsoluteX(value);
				break;
			default:
				__assume(0);
			}
		}

		uint8_t AM_10(int bbb) {
			switch (bbb) {
			case 0b000:
				return AM_Immediate();
			case 0b001:
				return AM_ZeroPage();
			case 0b010:
				return AM_A();
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
				__assume(0);
			}
			return 0xff;
		}

		void AM_10(int bbb, uint8_t value) {
			switch (bbb) {
			case 0b010:
				AM_A(value);
				break;
			case 0b001:
			case 0b011:
			case 0b101:
			case 0b111:
				m_memory.write(MEMPTR(), value);
				break;
			case 0b000:
			case 0b100:
			case 0b110:
				throw std::domain_error("Illegal addressing mode");
			default:
				__assume(0);
			}
		}

		uint8_t AM_10_x(int bbb) {
			switch (bbb) {
			case 0b000:
				return AM_Immediate();
			case 0b001:
				return AM_ZeroPage();
			case 0b010:
				return AM_A();
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
				__assume(0);
			}
		}

		void AM_10_x(int bbb, uint8_t value) {
			switch (bbb) {
			case 0b000:
				assert(false);
				break;
			case 0b001:
				AM_ZeroPage(value);
				break;
			case 0b010:
				AM_A(value);
				break;
			case 0b011:
				AM_Absolute(value);
				break;
			case 0b101:
				AM_ZeroPageY(value);
				break;
			case 0b111:
				AM_AbsoluteY(value);
				break;
			case 0b100:
			case 0b110:
				throw std::domain_error("Illegal addressing mode");
			default:
				__assume(0);
			}
		}

#pragma endregion 6502 addressing mode switching

#pragma endregion 6502 addressing modes

		void ASL(int bbb) {
			auto operand = AM_10(bbb);
			ASL(operand);
			AM_10(bbb, operand);
		}

		void ROL(int bbb) {
			auto operand = AM_10(bbb);
			ROL(operand);
			AM_10(bbb, operand);
		}

		void LSR(int bbb) {
			auto operand = AM_10(bbb);
			LSR(operand);
			AM_10(bbb, operand);
		}

		void ROR(int bbb) {
			auto operand = AM_10(bbb);
			ROR(operand);
			AM_10(bbb, operand);
		}

		void DEC(int bbb) {
			auto operand = AM_10(bbb);
			adjustNZ(--operand);
			AM_10(bbb, operand);
		}

		void INC(int bbb) {
			auto operand = AM_10(bbb);
			adjustNZ(++operand);
			AM_10(bbb, operand);
		}

		void ROR(uint8_t& output);

		void LSR(uint8_t& output);

		void BIT(uint8_t data);

		void ROL(uint8_t& output);

		void ASL(uint8_t& output);

		void SBC(uint8_t data);
		void SBC_b(uint8_t data);
		void SBC_d(uint8_t data);

		void CMP(uint8_t first, uint8_t second);

		void ADC(uint8_t data);
		void ADC_b(uint8_t data);
		void ADC_d(uint8_t data);

		void Branch(int8_t displacement);

		void Branch(bool flag);

		void PHP();
		void PLP();

		void JSR_abs();
		void RTI();
		void RTS();
		void JMP_abs();
		void JMP_ind();
		void BRK();

		const uint16_t PageOne = 0x100;
		const uint16_t IRQvector = 0xfffe;
		const uint16_t RSTvector = 0xfffc;
		const uint16_t NMIvector = 0xfffa;

		uint8_t x;		// index register X
		uint8_t y;		// index register Y
		uint8_t a;		// accumulator
		uint8_t s;		// stack pointer
		uint8_t p;		// processor status

		register16_t m_memptr;

		std::array<int, 0x100> m_timings;
		std::array<opcode_decoded_t, 0x100> m_decodedOpcodes;
	};
}
#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <functional>

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

		virtual int execute(uint8_t cell);

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

		uint8_t fetchByte();
		void fetchWord(register16_t& output);

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

#pragma region References

		uint8_t& AM_A() {
			m_busRW = false;
			return A();
		}

		uint8_t& AM_Immediate() {
			m_busRW = false;
			fetchByte();
			return m_memory.reference();
		}

		uint8_t& AM_Absolute() {
			m_busRW = true;
			Address_Absolute();
			m_memory.ADDRESS() = MEMPTR();
			return m_memory.reference();
		}

		uint8_t& AM_ZeroPage() {
			m_busRW = true;
			Address_ZeroPage();
			m_memory.ADDRESS() = MEMPTR();
			return m_memory.reference();
		}

		uint8_t& AM_AbsoluteX(bool read = true) {
			m_busRW = true;
			Address_AbsoluteX();
			m_memory.ADDRESS() = MEMPTR();
			if (read && (m_memory.ADDRESS().low == Mask8))
				++cycles;
			return m_memory.reference();
		}

		uint8_t& AM_AbsoluteY(bool read = true) {
			m_busRW = true;
			Address_AbsoluteY();
			m_memory.ADDRESS() = MEMPTR();
			if (read && (m_memory.ADDRESS().low == Mask8))
				++cycles;
			return m_memory.reference();
		}

		uint8_t& AM_ZeroPageX() {
			m_busRW = true;
			Address_ZeroPageX();
			m_memory.ADDRESS() = MEMPTR();
			return m_memory.reference();
		}

		uint8_t& AM_ZeroPageY() {
			m_busRW = true;
			Address_ZeroPageY();
			m_memory.ADDRESS() = MEMPTR();
			return m_memory.reference();
		}

		uint8_t& AM_IndexedIndirectX() {
			m_busRW = true;
			Address_IndexedIndirectX();
			m_memory.ADDRESS() = MEMPTR();
			return m_memory.reference();
		}

		uint8_t& AM_IndirectIndexedY(bool read = true) {
			m_busRW = true;
			Address_IndirectIndexedY();
			m_memory.ADDRESS() = MEMPTR();
			if (read && (m_memory.ADDRESS().low == Mask8))
				++cycles;
			return m_memory.reference();
		}

#pragma endregion References

#pragma region 6502 addressing mode switching

		uint8_t& AM_00(int bbb, bool read = true) {
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
				return AM_AbsoluteX(read);
			case 0b010:
			case 0b100:
			case 0b110:
				throw std::domain_error("Illegal addressing mode");
			default:
				__assume(0);
			}
		}

		uint8_t& AM_01(int bbb, bool read = true) {
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
				return AM_IndirectIndexedY(read);
			case 0b101:
				return AM_ZeroPageX();
			case 0b110:
				return AM_AbsoluteY(read);
			case 0b111:
				return AM_AbsoluteX(read);
			default:
				__assume(0);
			}
		}

		uint8_t& AM_10(int bbb, bool read = true) {
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
				return AM_AbsoluteX(read);
			case 0b100:
			case 0b110:
				throw std::domain_error("Illegal addressing mode");
			default:
				__assume(0);
			}
		}

		uint8_t& AM_10_x(int bbb, bool read = true) {
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
				return AM_AbsoluteY(read);
			case 0b100:
			case 0b110:
				throw std::domain_error("Illegal addressing mode");
			default:
				__assume(0);
			}
		}

#pragma endregion 6502 addressing mode switching

#pragma endregion 6502 addressing modes

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

		bool m_busRW;
	};
}
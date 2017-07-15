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
		enum StatusBits {
			NF = 0x80,	// Negative
			VF = 0x40,  // Overflow
			RF = 0x20,  // reserved
			BF = 0x10,  // Brk
			DF = 0x08,	// D (use BCD for arithmetic)
			IF = 0x04,	// I (IRQ disable)
			ZF = 0x02,	// Zero
			CF = 0x01,	// Carry
		};

		MOS6502(Memory& memory);

		Signal<MOS6502> ExecutingInstruction;
		Signal<MOS6502> ExecutedInstruction;

		uint8_t& X() { return x; }
		uint8_t& Y() { return y; }
		uint8_t& A() { return a; }
		uint8_t& S() { return s; }
		uint8_t& P() { return p; }

		virtual void initialise();

		virtual int step();

		virtual void Reset();

		virtual void TriggerIRQ();
		virtual void TriggerNMI();

		void GetWord(register16_t& output);
		void GetWord(uint16_t offset, register16_t& output);

		uint8_t GetByte() { return m_memory.read(); }
		uint8_t GetByte(uint16_t offset) { return m_memory.read(offset); }

		void SetByte(uint8_t value) { m_memory.write(value); }
		void SetByte(uint16_t offset, uint8_t value) { m_memory.write(offset, value); }

	protected:
		virtual void Interrupt(uint16_t vector);

		virtual int Execute(uint8_t cell);

	private:
		void UpdateZeroFlag(uint8_t datum) { clearFlag(P(), ZF, datum); }
		void UpdateNegativeFlag(uint8_t datum) { setFlag(P(), NF, datum & NF); }
		
		void UpdateZeroNegativeFlags(uint8_t datum) {
			UpdateZeroFlag(datum);
			UpdateNegativeFlag(datum);
		}

		void PushByte(uint8_t value);
		uint8_t PopByte();
		void PushWord(register16_t value);
		void PopWord(register16_t& output);

		uint8_t FetchByte();
		void FetchWord(register16_t& output);

#pragma region 6502 addressing modes

#pragma region Addresses

		void Address_Absolute() {
			FetchWord(m_memptr);
		}

		void Address_ZeroPage() {
			m_memptr.low = FetchByte();
			m_memptr.high = 0;
		}

		void Address_ZeroPageIndirect() {
			Address_ZeroPage();
			m_memory.ADDRESS() = m_memptr;
			GetWord(m_memptr);
		}

		void Address_Indirect() {
			Address_Absolute();
			m_memory.ADDRESS() = m_memptr;
			GetWord(m_memptr);
		}

		void Address_IndirectX() {
			Address_Absolute();
			m_memory.ADDRESS().word = m_memptr.word + X();
			GetWord(m_memptr);
		}

		void Address_ZeroPageX() {
			Address_ZeroPage();
			m_memptr.low += X();
		}

		void Address_ZeroPageY() {
			Address_ZeroPage();
			m_memptr.low += Y();
		}

		void Address_AbsoluteX() {
			Address_Absolute();
			m_memptr.word += X();
		}

		void Address_AbsoluteY() {
			Address_Absolute();
			m_memptr.word += Y();
		}

		void Address_IndexedIndirectX() {
			Address_ZeroPageX();
			m_memory.ADDRESS() = m_memptr;
			GetWord(m_memptr);
		}

		void Address_IndirectIndexedY() {
			Address_ZeroPageIndirect();
			m_memptr.word += Y();
		}

#pragma endregion Addresses

#pragma region References

		uint8_t& AM_Immediate() {
			FetchByte();
			return m_memory.reference();
		}

		uint8_t& AM_Absolute() {
			Address_Absolute();
			m_memory.ADDRESS() = m_memptr;
			return m_memory.reference();
		}

		uint8_t& AM_ZeroPage() {
			Address_ZeroPage();
			m_memory.ADDRESS() = m_memptr;
			return m_memory.reference();
		}

		uint8_t& AM_ZeroPageIndirect() {
			Address_ZeroPageIndirect();
			m_memory.ADDRESS() = m_memptr;
			return m_memory.reference();
		}

		uint8_t& AM_AbsoluteX(bool read = true) {
			Address_AbsoluteX();
			m_memory.ADDRESS() = m_memptr;
			if (read && (m_memory.ADDRESS().low == 0xff))
				++cycles;
			return m_memory.reference();
		}

		uint8_t& AM_AbsoluteY(bool read = true) {
			Address_AbsoluteY();
			m_memory.ADDRESS() = m_memptr;
			if (read && (m_memory.ADDRESS().low == 0xff))
				++cycles;
			return m_memory.reference();
		}

		uint8_t& AM_ZeroPageX() {
			Address_ZeroPageX();
			m_memory.ADDRESS() = m_memptr;
			return m_memory.reference();
		}

		uint8_t& AM_ZeroPageY() {
			Address_ZeroPageY();
			m_memory.ADDRESS() = m_memptr;
			return m_memory.reference();
		}

		uint8_t& AM_IndexedIndirectX() {
			Address_IndexedIndirectX();
			m_memory.ADDRESS() = m_memptr;
			return m_memory.reference();
		}

		uint8_t& AM_IndirectIndexedY(bool read = true) {
			Address_IndirectIndexedY();
			m_memory.ADDRESS() = m_memptr;
			if (read && (m_memory.ADDRESS().low == 0xff))
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
				return A();
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
				return A();
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

		void DEC(uint8_t& output);

		void ROR(uint8_t& output);

		void LSR(uint8_t& output);

		void BIT(uint8_t data);

		void INC(uint8_t& output);

		void ROL(uint8_t& output);

		void ASL(uint8_t& output);

		void ORA(uint8_t data);

		void AND(uint8_t data);

		void SBC(uint8_t data);
		void SBC_b(uint8_t data);
		void SBC_d(uint8_t data);

		void EOR(uint8_t data);

		void CMP(uint8_t first, uint8_t second);

		void LDA(uint8_t data);
		void LDY(uint8_t data);
		void LDX(uint8_t data);

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
		void JMP_absxind();
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
	};
}
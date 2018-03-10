#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <functional>
#include <cassert>

#include <Bus.h>
#include <Processor.h>
#include <Signal.h>

namespace EightBit {
	class MOS6502 : public Processor {
	public:
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

		MOS6502(Bus& bus);

		Signal<MOS6502> ExecutingInstruction;
		Signal<MOS6502> ExecutedInstruction;

		virtual int execute(uint8_t opcode) final;
		virtual int step() final;
		virtual void powerOn() override;

		uint8_t& X() { return x; }
		uint8_t& Y() { return y; }
		uint8_t& A() { return a; }
		uint8_t& S() { return s; }
		uint8_t& P() { return p; }

		PinLevel& SO() { return m_soLine;  }	// In

	protected:
		virtual void reset() final;

		virtual uint8_t SUB(uint8_t operand, uint8_t data, int borrow = 0);
		uint8_t SBC(uint8_t operand, uint8_t data);
		uint8_t SUB_b(uint8_t operand, uint8_t data, int borrow);
		uint8_t SUB_d(uint8_t operand, uint8_t data, int borrow);

		virtual uint8_t ADD(uint8_t operand, uint8_t data, int carry = 0);
		uint8_t ADC(uint8_t operand, uint8_t data);
		uint8_t ADD_b(uint8_t operand, uint8_t data, int carry);
		uint8_t ADD_d(uint8_t operand, uint8_t data, int carry);

	private:
		void interrupt(uint8_t vector);

		void adjustZero(uint8_t datum) { clearFlag(P(), ZF, datum); }
		void adjustNegative(uint8_t datum) { setFlag(P(), NF, datum & NF); }
		
		void adjustNZ(uint8_t datum) {
			adjustZero(datum);
			adjustNegative(datum);
		}

		register16_t getWordPaged(uint8_t page, uint8_t offset);
		uint8_t getBytePaged(uint8_t page, uint8_t offset);
		void setBytePaged(uint8_t page, uint8_t offset, uint8_t value);

		virtual void push(uint8_t value) final;
		virtual uint8_t pop() final;

		// Address resolution

		void Address_Absolute() {
			MEMPTR() = fetchWord();
		}

		void Address_ZeroPage() {
			MEMPTR().low = fetchByte();
			MEMPTR().high = 0;
		}

		void Address_ZeroPageIndirect() {
			Address_ZeroPage();
			MEMPTR() = getWordPaged(0, MEMPTR().low);
		}

		void Address_Indirect() {
			Address_Absolute();
			MEMPTR() = getWordPaged(MEMPTR().high, MEMPTR().low);
		}

		void Address_ZeroPageX() {
			Address_ZeroPage();
			MEMPTR().low += X();
		}

		void Address_ZeroPageY() {
			Address_ZeroPage();
			MEMPTR().low += Y();
		}

		bool Address_AbsoluteX() {
			Address_Absolute();
			const auto page = MEMPTR().high;
			MEMPTR().word += X();
			return MEMPTR().high != page;
		}

		bool Address_AbsoluteY() {
			Address_Absolute();
			const auto page = MEMPTR().high;
			MEMPTR().word += Y();
			return MEMPTR().high != page;
		}

		void Address_IndexedIndirectX() {
			Address_ZeroPageX();
			MEMPTR() = getWordPaged(0, MEMPTR().low);
		}

		bool Address_IndirectIndexedY() {
			Address_ZeroPageIndirect();
			const auto page = MEMPTR().high;
			MEMPTR().word += Y();
			return MEMPTR().high != page;
		}

		// Addressing modes, read

		uint8_t AM_Immediate() {
			return fetchByte();
		}

		uint8_t AM_Absolute() {
			Address_Absolute();
			return BUS().read(MEMPTR());
		}

		uint8_t AM_ZeroPage() {
			Address_ZeroPage();
			return BUS().read(MEMPTR());
		}

		uint8_t AM_AbsoluteX() {
			if (UNLIKELY(Address_AbsoluteX()))
				addCycle();
			return BUS().read(MEMPTR());
		}

		uint8_t AM_AbsoluteY() {
			if (UNLIKELY(Address_AbsoluteY()))
				addCycle();
			return BUS().read(MEMPTR());
		}

		uint8_t AM_ZeroPageX() {
			Address_ZeroPageX();
			return BUS().read(MEMPTR());
		}

		uint8_t AM_ZeroPageY() {
			Address_ZeroPageY();
			return BUS().read(MEMPTR());
		}

		uint8_t AM_IndexedIndirectX() {
			Address_IndexedIndirectX();
			return BUS().read(MEMPTR());
		}

		uint8_t AM_IndirectIndexedY() {
			if (UNLIKELY(Address_IndirectIndexedY()))
				addCycle();
			return BUS().read(MEMPTR());
		}

		// Addressing modes, write

		void AM_Absolute(uint8_t value) {
			Address_Absolute();
			BUS().write(MEMPTR(), value);
		}

		void AM_ZeroPage(uint8_t value) {
			Address_ZeroPage();
			BUS().write(MEMPTR(), value);
		}

		void AM_AbsoluteX(uint8_t value) {
			Address_AbsoluteX();
			BUS().write(MEMPTR(), value);
		}

		void AM_AbsoluteY(uint8_t value) {
			Address_AbsoluteY();
			BUS().write(MEMPTR(), value);
		}

		void AM_ZeroPageX(uint8_t value) {
			Address_ZeroPageX();
			BUS().write(MEMPTR(), value);
		}

		void AM_ZeroPageY(uint8_t value) {
			Address_ZeroPageY();
			BUS().write(MEMPTR(), value);
		}

		void AM_IndexedIndirectX(uint8_t value) {
			Address_IndexedIndirectX();
			BUS().write(MEMPTR(), value);
		}

		void AM_IndirectIndexedY(uint8_t value) {
			Address_IndirectIndexedY();
			BUS().write(MEMPTR(), value);
		}

		// Operations

		void DCP(uint8_t value) {
			BUS().write(--value);
			CMP(A(), value);
		}

		void ISB(uint8_t value) {
			BUS().write(++value);
			A() = SBC(A(), value);
		}

		void SLO(uint8_t value) {
			const auto result = ASL(value);
			BUS().write(result);
			ORA(result);
		}

		void SRE(uint8_t value) {
			const auto result = LSR(value);
			BUS().write(result);
			EORA(result);
		}

		void RLA(uint8_t value) {
			const auto result = ROL(value);
			BUS().write(result);
			ANDA(result);
		}

		void RRA(uint8_t value) {
			const auto result = ROR(value);
			BUS().write(result);
			A() = ADC(A(), result);
		}

		void LAX(uint8_t value) {
			adjustNZ(X() = A() = value);
		}

		void AAC(uint8_t value) {
			ANDA(value);
			setFlag(P(), CF, A() & Bit7);
		}

		void ASR(uint8_t value) {
			A() = LSR(A() & value);
		}

		void ARR(uint8_t value) {
		}

		void ATX(uint8_t value) {
			ANDA(value);
			X() = A();
		}

		void AXS(uint8_t value) {
		}

		//

		uint8_t DEC(uint8_t value) {
			const auto result = --value;
			adjustNZ(result);
			return result;
		}

		uint8_t INC(uint8_t value) {
			const auto result = ++value;
			adjustNZ(result);
			return result;
		}

		void ORA(uint8_t value) {
			adjustNZ(A() |= value);
		}

		void ANDA(uint8_t value) {
			adjustNZ(A() &= value);
		}

		void EORA(uint8_t value) {
			adjustNZ(A() ^= value);
		}

		uint8_t ROR(uint8_t value);

		uint8_t LSR(uint8_t value);

		void BIT(uint8_t data);

		uint8_t ROL(uint8_t value);

		uint8_t ASL(uint8_t value);

		void CMP(uint8_t first, uint8_t second);

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

		// All interrupt vectors are on the 0xFF page
		const uint8_t IRQvector = 0xfe;
		const uint8_t RSTvector = 0xfc;
		const uint8_t NMIvector = 0xfa;

		uint8_t x = 0;		// index register X
		uint8_t y = 0;		// index register Y
		uint8_t a = 0;		// accumulator
		uint8_t s = 0;		// stack pointer
		uint8_t p = 0;		// processor status

		PinLevel m_soLine = Low;
	};
}
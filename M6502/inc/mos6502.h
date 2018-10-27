#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <functional>
#include <cassert>

#include <Bus.h>
#include <LittleEndianProcessor.h>
#include <Signal.h>

namespace EightBit {
	class MOS6502 : public LittleEndianProcessor {
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
		virtual void powerOn() final;

		uint8_t& X() { return x; }
		uint8_t& Y() { return y; }
		uint8_t& A() { return a; }
		uint8_t& S() { return s; }
		uint8_t& P() { return p; }

		PinLevel& NMI() { return m_nmiLine; }	// In
		PinLevel& SO() { return m_soLine;  }	// In

	protected:
		virtual void handleRESET() final;
		virtual void handleIRQ() final;

		virtual uint8_t SUB(uint8_t operand, uint8_t data, int borrow = 0);
		uint8_t SBC(uint8_t operand, uint8_t data);
		uint8_t SUB_b(uint8_t operand, uint8_t data, int borrow);
		uint8_t SUB_d(uint8_t operand, uint8_t data, int borrow);

		virtual uint8_t ADD(uint8_t operand, uint8_t data, int carry = 0);
		uint8_t ADC(uint8_t operand, uint8_t data);
		uint8_t ADD_b(uint8_t operand, uint8_t data, int carry);
		uint8_t ADD_d(uint8_t operand, uint8_t data, int carry);

	private:
		void handleNMI();
		void handleSO();
		void handleHALT();

		void interrupt(uint8_t vector);

		void adjustZero(const uint8_t datum) { clearFlag(P(), ZF, datum); }
		void adjustNegative(const uint8_t datum) { setFlag(P(), NF, datum & NF); }
		
		void adjustNZ(const uint8_t datum) {
			adjustZero(datum);
			adjustNegative(datum);
		}

		virtual void push(uint8_t value) final;
		virtual uint8_t pop() final;

		// Address resolution

		auto Address_Absolute() {
			return fetchWord();
		}

		auto Address_ZeroPage() {
			return fetchByte();
		}

		auto Address_ZeroPageIndirect() {
			return getWordPaged(0, Address_ZeroPage());
		}

		auto Address_Indirect() {
			const auto address = Address_Absolute();
			return getWordPaged(address.high, address.low);
		}

		uint8_t Address_ZeroPageX() {
			return Address_ZeroPage() + X();
		}

		uint8_t Address_ZeroPageY() {
			return Address_ZeroPage() + Y();
		}

		auto Address_AbsoluteX() {
			auto address = Address_Absolute();
			const auto page = address.high;
			address += X();
			return std::make_pair(address, address.high != page);
		}

		auto Address_AbsoluteY() {
			auto address = Address_Absolute();
			const auto page = address.high;
			address += Y();
			return std::make_pair(address, address.high != page);
		}

		auto Address_IndexedIndirectX() {
			return getWordPaged(0, Address_ZeroPageX());
		}

		auto Address_IndirectIndexedY() {
			auto address = Address_ZeroPageIndirect();
			const auto page = address.high;
			address += Y();
			return std::make_pair(address, address.high != page);
		}

		// Addressing modes, read

		auto AM_Immediate() {
			return fetchByte();
		}

		auto AM_Absolute() {
			return BUS().read(Address_Absolute());
		}

		auto AM_ZeroPage() {
			return BUS().read(Address_ZeroPage());
		}

		auto AM_AbsoluteX() {
			const auto [address, paged] = Address_AbsoluteX();
			if (UNLIKELY(paged))
				addCycle();
			return BUS().read(address);
		}

		auto AM_AbsoluteY() {
			const auto [address, paged] = Address_AbsoluteY();
			if (UNLIKELY(paged))
				addCycle();
			return BUS().read(address);
		}

		auto AM_ZeroPageX() {
			return BUS().read(Address_ZeroPageX());
		}

		auto AM_ZeroPageY() {
			return BUS().read(Address_ZeroPageY());
		}

		auto AM_IndexedIndirectX() {
			return BUS().read(Address_IndexedIndirectX());
		}

		auto AM_IndirectIndexedY() {
			const auto [address, paged] = Address_IndirectIndexedY();
			if (UNLIKELY(paged))
				addCycle();
			return BUS().read(address);
		}

		// Addressing modes, write

		void AM_Absolute(const uint8_t value) {
			BUS().write(Address_Absolute(), value);
		}

		void AM_ZeroPage(const uint8_t value) {
			BUS().write(Address_ZeroPage(), value);
		}

		void AM_AbsoluteX(const uint8_t value) {
			const auto [address, paged] = Address_AbsoluteX();
			BUS().write(address, value);
		}

		void AM_AbsoluteY(const uint8_t value) {
			const auto [address, paged] = Address_AbsoluteY();
			BUS().write(address, value);
		}

		void AM_ZeroPageX(const uint8_t value) {
			BUS().write(Address_ZeroPageX(), value);
		}

		void AM_ZeroPageY(const uint8_t value) {
			BUS().write(Address_ZeroPageY(), value);
		}

		void AM_IndexedIndirectX(const uint8_t value) {
			BUS().write(Address_IndexedIndirectX(), value);
		}

		void AM_IndirectIndexedY(const uint8_t value) {
			const auto [address, paged] = Address_IndirectIndexedY();
			BUS().write(address, value);
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

		void SLO(const uint8_t value) {
			const auto result = ASL(value);
			BUS().write(result);
			ORA(result);
		}

		void SRE(const uint8_t value) {
			const auto result = LSR(value);
			BUS().write(result);
			EORA(result);
		}

		void RLA(const uint8_t value) {
			const auto result = ROL(value);
			BUS().write(result);
			ANDA(result);
		}

		void RRA(const uint8_t value) {
			const auto result = ROR(value);
			BUS().write(result);
			A() = ADC(A(), result);
		}

		void LAX(const uint8_t value) {
			adjustNZ(X() = A() = value);
		}

		void AAC(const uint8_t value) {
			ANDA(value);
			setFlag(P(), CF, A() & Bit7);
		}

		void ASR(const uint8_t value) {
			A() = LSR(A() & value);
		}

		void ARR(const uint8_t value) {
		}

		void ATX(const uint8_t value) {
			ANDA(value);
			X() = A();
		}

		void AXS(const uint8_t value) {
		}

		//

		auto DEC(uint8_t value) {
			const auto result = --value;
			adjustNZ(result);
			return result;
		}

		auto INC(uint8_t value) {
			const auto result = ++value;
			adjustNZ(result);
			return result;
		}

		void ORA(const uint8_t value) {
			adjustNZ(A() |= value);
		}

		void ANDA(const uint8_t value) {
			adjustNZ(A() &= value);
		}

		void EORA(const uint8_t value) {
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

		PinLevel m_nmiLine = Low;
		PinLevel m_soLine = Low;

		register16_t m_intermediate;
	};
}
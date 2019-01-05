#pragma once

#include <cstdint>
#include <utility>

#include <EightBitCompilerDefinitions.h>
#include <LittleEndianProcessor.h>
#include <Register.h>
#include <Signal.h>

namespace EightBit {

	class Bus;

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

		virtual int execute() final;
		virtual int step() final;
		virtual void powerOn() final;

		auto& X() { return x; }
		auto& Y() { return y; }
		auto& A() { return a; }
		auto& S() { return s; }

		auto& P() { return p; }
		const auto& P() const { return p; }

		auto& NMI() { return m_nmiLine; }	// In
		auto& SO() { return m_soLine;  }	// In

	protected:
		virtual void handleRESET() final;
		virtual void handleIRQ() final;

		virtual void busWrite() final;
		virtual uint8_t busRead() final;

		virtual uint8_t sub(uint8_t operand, uint8_t data, int borrow = 0);
		uint8_t sbc(uint8_t operand, uint8_t data);
		uint8_t sub_b(uint8_t operand, uint8_t data, int borrow);
		uint8_t sub_d(uint8_t operand, uint8_t data, int borrow);

		virtual uint8_t add(uint8_t operand, uint8_t data, int carry = 0);
		uint8_t adc(uint8_t operand, uint8_t data);
		uint8_t add_b(uint8_t operand, uint8_t data, int carry);
		uint8_t add_d(uint8_t operand, uint8_t data, int carry);

	private:
		const uint8_t IRQvector = 0xfe;		// IRQ vector
		const uint8_t RSTvector = 0xfc;		// RST vector
		const uint8_t NMIvector = 0xfa;		// NMI vector

		void handleNMI();
		void handleSO();
		void handleHALT();

		void interrupt(uint8_t vector);

		virtual void push(uint8_t value) final;
		virtual uint8_t pop() final;

		// Addressing modes

		register16_t Address_Absolute();
		uint8_t Address_ZeroPage();
		register16_t Address_ZeroPageIndirect();
		register16_t Address_Indirect();
		uint8_t Address_ZeroPageX();
		uint8_t Address_ZeroPageY();
		std::pair<register16_t, bool> Address_AbsoluteX();
		std::pair<register16_t, bool> Address_AbsoluteY();
		register16_t Address_IndexedIndirectX();
		std::pair<register16_t, bool> Address_IndirectIndexedY();
		register16_t Address_relative_byte();

		// Addressing modes, read

		uint8_t AM_Immediate();
		uint8_t AM_Absolute();
		uint8_t AM_ZeroPage();
		uint8_t AM_AbsoluteX();
		uint8_t AM_AbsoluteY();
		uint8_t AM_ZeroPageX();
		uint8_t AM_ZeroPageY();
		uint8_t AM_IndexedIndirectX();
		uint8_t AM_IndirectIndexedY();

		// Flag adjustment

		void adjustZero(const uint8_t datum) { clearFlag(P(), ZF, datum); }
		void adjustNegative(const uint8_t datum) { setFlag(P(), NF, datum & NF); }
		
		void adjustNZ(const uint8_t datum) {
			adjustZero(datum);
			adjustNegative(datum);
		}

		// Flag checking

		auto interruptMasked() const { return P() & IF; }
		auto decimal() const  { return P() & DF; }

		auto negative() const { return P() & NF; }
		auto zero() const { return P() & ZF; }
		auto overflow() const { return P() & VF; }
		auto carry() const { return P() & CF; }

		// Miscellaneous

		void branch(int condition);

		auto through(const uint8_t data) {
			adjustNZ(data);
			return data;
		}

		void busReadModifyWrite(const uint8_t data) {
			// The read will have already taken place...
			busWrite();
			Processor::busWrite(data);
		}

		// Instruction implementations

		uint8_t andr(uint8_t operand, uint8_t data);
		uint8_t asl(uint8_t value);
		void bit(uint8_t operand, uint8_t data);
		void brk();
		void cmp(uint8_t first, uint8_t second);
		uint8_t dec(uint8_t value);
		uint8_t eorr(uint8_t operand, uint8_t data);
		uint8_t inc(uint8_t value);
		void jsr();
		uint8_t lsr(uint8_t value);
		uint8_t orr(uint8_t operand, uint8_t data);
		void php();
		void plp();
		uint8_t rol(uint8_t operand);
		uint8_t ror(uint8_t operand);
		void rti();
		void rts();

		// Undocumented compound instructions

		void anc(uint8_t value);
		void arr(uint8_t value);
		void asr(uint8_t value);
		void axs(uint8_t value);
		void dcp(uint8_t value);
		void isb(uint8_t value);
		void rla(uint8_t value);
		void rra(uint8_t value);
		void slo(uint8_t value);
		void sre(uint8_t value);

		uint8_t x = 0;		// index register X
		uint8_t y = 0;		// index register Y
		uint8_t a = 0;		// accumulator
		uint8_t s = 0;		// stack pointer
		uint8_t p = 0;		// processor status

		PinLevel m_nmiLine = PinLevel::Low;
		PinLevel m_soLine = PinLevel::Low;

		register16_t m_intermediate;
	};
}
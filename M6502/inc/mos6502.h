#pragma once

#include <cstdint>
#include <utility>

#include <EightBitCompilerDefinitions.h>
#include <LittleEndianProcessor.h>
#include <Register.h>
#include <Signal.h>
#include <EventArgs.h>

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

		int execute() final;
		[[nodiscard]] int step() final;

		[[nodiscard]] auto& X() { return x; }
		[[nodiscard]] auto& Y() { return y; }
		[[nodiscard]] auto& A() { return a; }
		[[nodiscard]] auto& S() { return s; }

		[[nodiscard]] auto& P() { return p; }
		[[nodiscard]] const auto& P() const { return p; }

		DECLARE_PIN_INPUT(NMI)
		DECLARE_PIN_INPUT(SO)
		DECLARE_PIN_OUTPUT(SYNC)
		DECLARE_PIN_INPUT(RDY)
		DECLARE_PIN_OUTPUT(RW)

	protected:
		void handleRESET() final;
		void handleINT() final;

		void busWrite() final;
		[[nodiscard]] uint8_t busRead() final;

		[[nodiscard]] virtual uint8_t sub(uint8_t operand, uint8_t data, int borrow = 0);
		[[nodiscard]] uint8_t sbc(uint8_t operand, uint8_t data);
		[[nodiscard]] uint8_t sub_b(uint8_t operand, uint8_t data, int borrow);
		[[nodiscard]] uint8_t sub_d(uint8_t operand, uint8_t data, int borrow);

		[[nodiscard]] virtual uint8_t add(uint8_t operand, uint8_t data, int carry = 0);
		[[nodiscard]] uint8_t adc(uint8_t operand, uint8_t data);
		[[nodiscard]] uint8_t add_b(uint8_t operand, uint8_t data, int carry);
		[[nodiscard]] uint8_t add_d(uint8_t operand, uint8_t data, int carry);

	private:
		const uint8_t IRQvector = 0xfe;		// IRQ vector
		const uint8_t RSTvector = 0xfc;		// RST vector
		const uint8_t NMIvector = 0xfa;		// NMI vector

		void handleNMI();
		void handleSO();

		void interrupt();

		void push(uint8_t value) final;
		[[nodiscard]] uint8_t pop() final;

		// Dummy stack push, used during RESET
		void dummyPush(uint8_t value);

		// Addressing modes

		[[nodiscard]] register16_t Address_Absolute();
		[[nodiscard]] uint8_t Address_ZeroPage();
		[[nodiscard]] register16_t Address_ZeroPageIndirect();
		[[nodiscard]] register16_t Address_Indirect();
		[[nodiscard]] uint8_t Address_ZeroPageX();
		[[nodiscard]] uint8_t Address_ZeroPageY();
		[[nodiscard]] std::pair<register16_t, uint8_t> Address_AbsoluteX();
		[[nodiscard]] std::pair<register16_t, uint8_t> Address_AbsoluteY();
		[[nodiscard]] register16_t Address_IndexedIndirectX();
		[[nodiscard]] std::pair<register16_t, uint8_t> Address_IndirectIndexedY();
		[[nodiscard]] register16_t Address_relative_byte();

		// Addressing modes, read

		enum class PageCrossingBehavior { AlwaysReadTwice, MaybeReadTwice };

		uint8_t AM_Immediate();
		uint8_t AM_Absolute();
		uint8_t AM_ZeroPage();
		uint8_t AM_AbsoluteX(PageCrossingBehavior behaviour = PageCrossingBehavior::MaybeReadTwice);
		uint8_t AM_AbsoluteY();
		uint8_t AM_ZeroPageX();
		uint8_t AM_ZeroPageY();
		uint8_t AM_IndexedIndirectX();
		uint8_t AM_IndirectIndexedY();

		// Flag adjustment

		void adjustZero(const uint8_t datum) { P() = clearBit(P(), ZF, datum); }
		void adjustNegative(const uint8_t datum) { P() = setBit(P(), NF, datum & NF); }
		
		void adjustNZ(const uint8_t datum) {
			adjustZero(datum);
			adjustNegative(datum);
		}

		// Flag checking

		[[nodiscard]] auto interruptMasked() const { return P() & IF; }
		[[nodiscard]] auto decimal() const  { return P() & DF; }

		[[nodiscard]] auto negative() const { return P() & NF; }
		[[nodiscard]] auto zero() const { return P() & ZF; }
		[[nodiscard]] auto overflow() const { return P() & VF; }
		[[nodiscard]] auto carry() const { return P() & CF; }

		// Miscellaneous

		void branch(int condition);

		[[nodiscard]] auto through(const uint8_t data) {
			adjustNZ(data);
			return data;
		}

		void memoryReadModifyWrite(const uint8_t data) {
			// The read will have already taken place...
			memoryWrite();
			memoryWrite(data);
		}

		// Instruction implementations

		[[nodiscard]] uint8_t andr(uint8_t operand, uint8_t data);
		[[nodiscard]] uint8_t asl(uint8_t value);
		void bit(uint8_t operand, uint8_t data);
		void cmp(uint8_t first, uint8_t second);
		[[nodiscard]] uint8_t dec(uint8_t value);
		[[nodiscard]] uint8_t eorr(uint8_t operand, uint8_t data);
		[[nodiscard]] uint8_t inc(uint8_t value);
		void jsr();
		[[nodiscard]] uint8_t lsr(uint8_t value);
		[[nodiscard]] uint8_t orr(uint8_t operand, uint8_t data);
		void php();
		void plp();
		[[nodiscard]] uint8_t rol(uint8_t operand);
		[[nodiscard]] uint8_t ror(uint8_t operand);
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

		// Complicated addressing mode implementations

		void sta_AbsoluteX();
		void sta_AbsoluteY();

		uint8_t x = 0;		// index register X
		uint8_t y = 0;		// index register Y
		uint8_t a = 0;		// accumulator
		uint8_t s = 0;		// stack pointer
		uint8_t p = 0;		// processor status

		register16_t m_intermediate;

		bool m_handlingRESET = false;
		bool m_handlingNMI = false;
		bool m_handlingINT = false;
	};
}
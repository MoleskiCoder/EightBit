#pragma once

// Uses some information from:
// http://www.cpu-world.com/Arch/6809.html

#include <cstdint>

#include <Bus.h>
#include <BigEndianProcessor.h>
#include <Signal.h>
#include <Register.h>

namespace EightBit {
	class mc6809 : public BigEndianProcessor {
	public:
		enum StatusBits {

			// Entire flag: set if the complete machine state was saved in the stack.
			// If this bit is not set then only program counter and condition code
			// registers were saved in the stack. This bit is used by interrupt
			// handling routines only.
			// The bit is cleared by fast interrupts, and set by all other interrupts.
			EF = Bit7,

			// Fast interrupt mask: set if the FIRQ interrupt is disabled.
			FF = Bit6,

			// Half carry: set if there was a carry from bit 3 to bit 4 of the result
			// during the last add operation.
			HF = Bit5,

			// Interrupt mask: set if the IRQ interrupt is disabled.
			IF = Bit4,

			// Negative: set if the most significant bit of the result is set.
			// This bit can be set not only by arithmetic and logical operations,
			// but also by load / store operations.
			NF = Bit3,

			// Zero: set if the result is zero. Like the N bit, this bit can be
			// set not only by arithmetic and logical operations, but also
			// by load / store operations.
			ZF = Bit2,

			// Overflow: set if there was an overflow during last result calculation.
			// Logical, load and store operations clear this bit.
			VF = Bit1,

			// Carry: set if there was a carry from the bit 7 during last add
			// operation, or if there was a borrow from last subtract operation,
			// or if bit 7 of the A register was set during last MUL operation.
			CF = Bit0,
		};

		mc6809(Bus& bus);

		Signal<mc6809> ExecutingInstruction;
		Signal<mc6809> ExecutedInstruction;

		virtual int execute(uint8_t opcode) final;
		virtual int step() final;

		virtual void powerOn() final;

		register16_t& D() { return m_d; }
		uint8_t& A() { return D().high; }
		uint8_t& B() { return D().low; }

		register16_t& X() { return m_x; }
		register16_t& Y() { return m_y; }
		register16_t& U() { return m_u; }
		register16_t& S() { return m_s; }

		uint8_t& DP() { return m_dp; }
		uint8_t& CC() { return m_cc; }

		PinLevel& NMI() { return m_nmiLine; }		// In
		PinLevel& FIRQ() { return m_firqLine; }		// In

		//	|---------------|-----------------------------------|
		//	|	MPU State	|									|
		//	|_______________|	MPU State Definition			|
		//	|	BA  |	BS	|									|
		//	|_______|_______|___________________________________|
		//	|	0	|	0	|	Normal (running)				|
		//	|	0	|	1	|	Interrupt or RESET Acknowledge	|
		//	|	1	|	0	|	SYNC Acknowledge				|
		//	|	1	|	1	|	HALT Acknowledge				|
		//	|-------|-------|-----------------------------------|

		PinLevel& BA() { return m_baLine; }			// Out
		PinLevel& BS() { return m_bsLine; }			// Out

	protected:
		// Default push/pop handlers

		virtual void push(uint8_t value) final { pushS(value);  }
		virtual uint8_t pop() final { return popS(); }

		// Interrupt (etc.) handlers

		virtual void handleRESET() final;
		virtual void handleIRQ() final;	

	private:
		const uint8_t RESETvector = 0xfe;		// RESET vector
		const uint8_t NMIvector = 0xfc;			// NMI vector
		const uint8_t SWIvector = 0xfa;			// SWI vector
		const uint8_t IRQvector = 0xf8;			// IRQ vector
		const uint8_t FIRQvector = 0xf6;		// FIRQ vector
		const uint8_t SWI2vector = 0xf4;		// SWI2 vector
		const uint8_t SWI3vector = 0xf2;		// SWI3 vector
		const uint8_t RESERVEDvector = 0xf0;	// RESERVED vector

		// Stack manipulation

		void push(register16_t& stack, uint8_t value);
		void pushS(uint8_t value) { push(S(), value); }
		void pushU(uint8_t value) { push(U(), value); }

		void pushWord(register16_t& stack, register16_t value) {
			push(stack, value.low);
			push(stack, value.high);
		}

		void pushWordS(register16_t value) { pushWord(S(), value); }
		void pushWordU(register16_t value) { pushWord(U(), value); }

		uint8_t pop(register16_t& stack);
		uint8_t popS() { return pop(S()); }
		uint8_t popU() { return pop(U()); }

		register16_t popWord(register16_t& stack) {
			const auto high = pop(stack);
			const auto low = pop(stack);
			return register16_t(low, high);
		}

		register16_t popWordS() { return popWord(S()); }
		register16_t popWordU() { return popWord(U()); }

		// Interrupt (etc.) handlers

		void handleHALT();
		void handleNMI();
		void handleFIRQ();

		// Execution helpers

		void executeUnprefixed(uint8_t opcode);
		void execute10(uint8_t opcode);
		void execute11(uint8_t opcode);

		// Register selection for "indexed"
		register16_t& RR(int which);

		// Register selection for 8-bit transfer/exchange
		uint8_t& referenceTransfer8(int specifier);

		// Register selection for 16-bit transfer/exchange
		register16_t& referenceTransfer16(int specifier);

		// Addressing modes

		register16_t Address_direct();			// DP + fetched offset
		register16_t Address_indexed();			// Indexed address, complicated!
		register16_t Address_extended();		// Fetched address
		register16_t Address_relative_byte();	// PC + fetched byte offset
		register16_t Address_relative_word();	// PC + fetched word offset

		// Addressing mode readers

		// Single byte readers

		uint8_t AM_immediate_byte();
		uint8_t AM_direct_byte();
		uint8_t AM_indexed_byte();
		uint8_t AM_extended_byte();

		// Word readers

		register16_t AM_immediate_word();
		register16_t AM_direct_word();
		register16_t AM_indexed_word();
		register16_t AM_extended_word();

		// Flag adjustment

		template<class T> void adjustZero(T datum) { clearFlag(CC(), ZF, datum); }
		void adjustZero(register16_t datum) { clearFlag(CC(), ZF, datum.word); }
		void adjustNegative(uint8_t datum) { setFlag(CC(), NF, datum & Bit7); }
		void adjustNegative(uint16_t datum) { setFlag(CC(), NF, datum & Bit15); }
		void adjustNegative(register16_t datum) { adjustNegative(datum.word); }

		template<class T> void adjustNZ(T datum) {
			adjustZero(datum);
			adjustNegative(datum);
		}

		void adjustCarry(uint16_t datum) { setFlag(CC(), CF, datum & Bit8); }		// 8-bit addition
		void adjustCarry(uint32_t datum) { setFlag(CC(), CF, datum & Bit16); }		// 16-bit addition
		void adjustCarry(register16_t datum) { adjustCarry(datum.word); }

		void adjustBorrow(uint16_t datum) { clearFlag(CC(), CF, datum & Bit8); }	// 8-bit subtraction
		void adjustBorrow(uint32_t datum) { clearFlag(CC(), CF, datum & Bit16); }	// 16-bit subtraction
		void adjustBorrow(register16_t datum) { adjustBorrow(datum.word); }

		void adjustOverflow(uint8_t before, uint8_t data, register16_t after) {
			const uint8_t lowAfter = after.low;
			const uint8_t highAfter = after.high;
			setFlag(CC(), VF, (before ^ data ^ lowAfter ^ (highAfter << 7)) & Bit7);
		}

		void adjustOverflow(uint16_t before, uint16_t data, uint32_t after) {
			const uint16_t lowAfter = after & Mask16;
			const uint16_t highAfter = after >> 16;
			setFlag(CC(), VF, (before ^ data ^ lowAfter ^ (highAfter << 15)) & Bit15);
		}

		void adjustOverflow(register16_t before, register16_t data, register16_t after) {
			adjustOverflow(before.word, data.word, after.word);
		}

		void adjustHalfCarry(uint8_t before, uint8_t data, uint8_t after) {
			setFlag(CC(), HF, !!((before ^ data ^ after) & Bit4));
		}

		void adjustAddition(uint8_t before, uint8_t data, register16_t after) {
			const auto result = after.low;
			adjustNZ(result);
			adjustCarry(after);
			adjustOverflow(before, data, after);
			adjustHalfCarry(before, data, result);
		}

		void adjustAddition(uint16_t before, uint16_t data, uint32_t after) {
			const register16_t result = after & Mask16;
			adjustNZ(result);
			adjustCarry(after);
			adjustOverflow(before, data, after);
		}

		void adjustAddition(register16_t before, register16_t data, uint32_t after) {
			adjustAddition(before.word, data.word, after);
		}

		void adjustSubtraction(uint8_t before, uint8_t data, register16_t after) {
			const auto result = after.low;
			adjustNZ(result);
			adjustCarry(after);
			adjustOverflow(before, data, after);
		}

		void adjustSubtraction(uint16_t before, uint16_t data, uint32_t after) {
			const register16_t result = after & Mask16;
			adjustNZ(result);
			adjustCarry(after);
			adjustOverflow(before, data, after);
		}

		void adjustSubtraction(register16_t before, register16_t data, uint32_t after) {
			adjustSubtraction(before.word, data.word, after);
		}

		// Flag checking

		int fastInterruptMasked() { return CC() & FF; }
		int interruptMasked() { return CC() & IF; }

		int negative() { return CC() & NF; }
		int zero() { return CC() & ZF; }
		int overflow() { return CC() & VF; }
		int carry() { return CC() & CF; }
		int halfCarry() { return CC() & HF; }

		bool LS() { return carry() | (zero() >> 2); }									// (C OR Z)
		bool HI() { return !LS(); }														// !(C OR Z)
		bool LT() { return (negative() >> 1) ^ overflow(); }							// (N XOR V)
		bool GE() { return !LT(); }														// !(N XOR V)
		bool LE() { return (zero() >> 2) & ((negative() >> 3) ^ (overflow() >> 1)); }	// (Z OR (N XOR V))
		bool GT() { return !LE(); }														// !(Z OR (N XOR V))

		// Branching

		bool branch(register16_t destination, int condition) {
			if (condition)
				jump(destination);
			return !!condition;
		}

		void branchShort(int condition) {
			branch(Address_relative_byte(), condition);
		}

		void branchLong(int condition) {
			if (branch(Address_relative_word(), condition))
				addCycle();
		}

		// Miscellaneous

		void saveEntireRegisterState();

		// Instruction implementations

		uint8_t adc(uint8_t operand, uint8_t data);
		uint8_t add(uint8_t operand, uint8_t data, uint8_t carry = 0);
		register16_t add(register16_t operand, register16_t data);
		uint8_t andr(uint8_t operand, uint8_t data);
		uint8_t asl(uint8_t operand);
		uint8_t asr(uint8_t operand);
		uint8_t clr();
		void cmp(uint8_t operand, uint8_t data);
		void cmp(register16_t operand, register16_t data);
		uint8_t com(uint8_t operand);
		void cwai(uint8_t data);
		uint8_t da(uint8_t operand);
		uint8_t dec(uint8_t operand);
		uint8_t eor(uint8_t operand, uint8_t data);
		void exg(uint8_t data);
		uint8_t inc(uint8_t operand);
		void jsr(register16_t address);
		uint8_t ld(uint8_t data);
		register16_t ld(register16_t data);
		uint8_t lsr(uint8_t operand);
		register16_t mul(uint8_t first, uint8_t second);
		uint8_t neg(uint8_t operand);
		uint8_t orr(uint8_t operand, uint8_t data);
		void psh(register16_t& stack, uint8_t data);
		void pul(register16_t& stack, uint8_t data);
		uint8_t rol(uint8_t operand);
		uint8_t ror(uint8_t operand);
		void rti();
		void rts();
		uint8_t sbc(uint8_t operand, uint8_t data);
		uint8_t sub(uint8_t operand, uint8_t data, uint8_t carry = 0);
		register16_t sub(register16_t operand, register16_t data);
		uint8_t sex(uint8_t from);
		void swi();
		void swi2();
		void swi3();
		uint8_t st(uint8_t data);
		register16_t st(register16_t data);
		void tfr(uint8_t data);
		void tst(uint8_t data);

		register16_t m_d;
		register16_t m_x;
		register16_t m_y;
		register16_t m_u;
		register16_t m_s;

		uint8_t m_dp;
		uint8_t m_cc;

		PinLevel m_nmiLine = Low;
		PinLevel m_firqLine = Low;

		PinLevel m_baLine = Low;
		PinLevel m_bsLine = Low;

		bool m_prefix10 = false;
		bool m_prefix11 = false;
	};
}
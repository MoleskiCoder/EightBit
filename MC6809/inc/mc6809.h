#pragma once

// Uses some information from:
// http://www.cpu-world.com/Arch/6809.html

#include <cstdint>

#include <Bus.h>
#include <BigEndianProcessor.h>

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

		PinLevel& IRQ() { return INT(); }
		PinLevel& FIRQ() { return m_firq; }

	protected:
		virtual void reset() final;

	private:
		const uint8_t RESETvector = 0xfe;		// RESET vector
		const uint8_t NMIvector = 0xfc;			// NMI vector
		const uint8_t SWIvector = 0xfa;			// SWI vector
		const uint8_t IRQvector = 0xf8;			// IRQ vector
		const uint8_t FIRQvector = 0xf6;		// FIRQ vector
		const uint8_t SWI2vector = 0xf4;		// SWI2 vector
		const uint8_t SWI3vector = 0xf2;		// SWI3 vector
		const uint8_t RESERVEDvector = 0xf0;	// RESERVED vector

		// Execution helpers

		int executeUnprefixed(uint8_t opcode);
		int execute10(uint8_t opcode);
		int execute11(uint8_t opcode);

		// Register selection for "indexed"
		register16_t& RR(int which);

		// Addressing modes

		void Address_direct();		// DP + fetched offset
		void Address_indexed();		// Indexed address, complicated!
		void Address_extended();	// Fetched address

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
		void adjustNegative(uint8_t datum) { setFlag(CC(), NF, datum & Bit7); }
		void adjustNegative(uint16_t datum) { setFlag(CC(), NF, datum & Bit15); }

		template<class T> void adjustNZ(T datum) {
			adjustZero(datum);
			adjustNegative(datum);
		}

		void adjustCarry(uint16_t datum) { setFlag(CC(), CF, datum & Bit8); }		// 8-bit addition
		void adjustCarry(uint32_t datum) { setFlag(CC(), CF, datum & Bit16); }		// 16-bit addition

		void adjustBorrow(uint16_t datum) { clearFlag(CC(), CF, datum & Bit8); }	// 8-bit subtraction
		void adjustBorrow(uint32_t datum) { clearFlag(CC(), CF, datum & Bit16); }	// 16-bit subtraction

		void adjustOverflow(uint8_t before, uint8_t data, uint8_t after) {
			setFlag(CC(), VF, (before ^ data) & (before ^ after) & Bit7);
		}

		void adjustOverflow(uint16_t before, uint16_t data, uint16_t after) {
			setFlag(CC(), VF, (before ^ data) & (before ^ after) & Bit15);
		}

		void adjustAddition(uint8_t before, uint8_t data, register16_t after) {
			const auto result = after.low;
			adjustNZ(result);
			adjustCarry(after.word);
			adjustOverflow(before, data, result);
		}

		void adjustAddition(uint16_t before, uint16_t data, uint32_t after) {
			const register16_t result = after & Mask16;
			adjustNZ(result.word);
			adjustCarry(after);
			adjustOverflow(before, data, result.word);
		}

		void adjustSubtraction(uint8_t before, uint8_t data, register16_t after) {
			const auto result = after.low;
			adjustNZ(result);
			adjustBorrow(after.word);
			adjustOverflow(before, data, result);
		}

		void adjustSubtraction(uint16_t before, uint16_t data, uint32_t after) {
			const register16_t result = after & Mask16;
			adjustNZ(result.word);
			adjustBorrow(after);
			adjustOverflow(before, data, result.word);
		}

		// Instruction implementations

		void abx();
		uint8_t adc(uint8_t operand, uint8_t data);
		uint8_t add(uint8_t operand, uint8_t data, int carry = 0);
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
		uint8_t neg(uint8_t operand);

		register16_t m_d;
		register16_t m_x;
		register16_t m_y;
		register16_t m_u;
		register16_t m_s;

		uint8_t m_dp;
		uint8_t m_cc;

		PinLevel m_firq;

		bool m_prefix10 = false;
		bool m_prefix11 = false;
	};
}
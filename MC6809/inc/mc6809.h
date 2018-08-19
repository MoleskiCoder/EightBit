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
		const uint8_t RESETvector = 0xfe;
		const uint8_t NMIvector = 0xfc;
		const uint8_t SWIvector = 0xfa;
		const uint8_t IRQvector = 0xf8;
		const uint8_t FIRQvector = 0xf6;
		const uint8_t SWI2vector = 0xf4;
		const uint8_t SWI3vector = 0xf2;
		const uint8_t RESERVEDvector = 0xf0;

		// Register selection for "indexed"
		register16_t& RR(int which);

		// Addressing modes
		void Address_direct();
		void Address_indexed();
		void Address_extended();

		// Addressing mode readers
		uint8_t AM_immediate();
		uint8_t AM_direct();
		uint8_t AM_indexed();
		uint8_t AM_extended();

		void abx();
		uint8_t adc(uint8_t operand, uint8_t data);
		uint8_t add(uint8_t operand, uint8_t data, int carry = 0);
		uint8_t neg(uint8_t operand);

		register16_t m_d;
		register16_t m_x;
		register16_t m_y;
		register16_t m_u;
		register16_t m_s;

		uint8_t m_dp;
		uint8_t m_cc;

		PinLevel m_firq;
	};
}
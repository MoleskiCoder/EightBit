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
			EF = Bit7,	// Entire flag: set if the complete machine state was saved in the stack.
						// If this bit is not set then only program counter and condition code
						// registers were saved in the stack. This bit is used by interrupt
						// handling routines only.
						// The bit is cleared by fast interrupts, and set by all other interrupts.
			FF = Bit6,  // Fast interrupt mask: set if the FIRQ interrupt is disabled.
			HF = Bit5,  // Half carry: set if there was a carry from bit 3 to bit 4 of the result
						// during the last add operation.
			IF = Bit4,  // Interrupt mask: set if the IRQ interrupt is disabled.
			NF = Bit3,	// Negative: set if the most significant bit of the result is set.
						// This bit can be set not only by arithmetic and logical operations,
						// but also by load / store operations.
			ZF = Bit2,	// Zero: set if the result is zero. Like the N bit, this bit can be
						// set not only by arithmetic and logical operations, but also
						// by load / store operations.
			VF = Bit1,	// Overflow: set if there was an overflow during last result calculation.
						// Logical, load and store operations clear this bit.
			CF = Bit0,	// Carry: set if there was a carry from the bit 7 during last add
						// operation, or if there was a borrow from last subtract operation,
						// or if bit 7 of the A register was set during last MUL operation.
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

		//

		void Address_direct() {
			BUS().ADDRESS() = register16_t(fetchByte(), DP());
		}

		register16_t& RR(int which) {
			ASSUME(which >= 0);
			ASSUME(which <= 3);
			switch (which) {
			case 0b00:
				return X();
			case 0b01:
				return Y();
			case 0b10:
				return U();
			case 0b11:
				return S();
			default:
				UNREACHABLE;
			}
		}

		void Address_indexed() {
			const auto type = fetchByte();
			auto& rr = RR(type & (Bit6 | Bit5));
			switch (type & Bit7) {
			case 0:		// EA = ,R + 5-bit offset
				addCycle();
				BUS().ADDRESS() = rr + (type & Mask5);
				break;
			case Bit7: {
					const auto indirect = type & Bit4;
					switch (type & Mask4) {
					case 0b0000:	// , R+
						ASSUME(!indirect);
						addCycles(2);
						BUS().ADDRESS() = rr++;
						break;
					case 0b0001:	// , R++
						addCycles(3);
						BUS().ADDRESS() = rr;
						rr += 2;
						break;
					case 0b0010:	// , -R
						ASSUME(!indirect);
						addCycles(2);
						BUS().ADDRESS() = --rr;
						break;
					case 0b0011:	// , --R
						addCycles(3);
						rr -= 2;
						BUS().ADDRESS() = rr;
						break;
					case 0b0100:	// , R
						BUS().ADDRESS() = rr;
						break;
					case 0b0101:	// B, R
						addCycles(1);
						BUS().ADDRESS() = rr + (int8_t)B();
						break;
					case 0b0110:	// A, R
						addCycles(1);
						BUS().ADDRESS() = rr + (int8_t)A();
						break;
					case 0b1000:	// n, R (eight-bit)
						addCycles(1);
						BUS().ADDRESS() = rr + (int8_t)fetchByte();
						break;
					case 0b1001:	// n, R (sixteen-bit)
						addCycles(4);
						BUS().ADDRESS() = rr + fetchWord();
						break;
					case 0b1011:	// D, R
						addCycles(4);
						BUS().ADDRESS() = rr + D();
						break;
					case 0b1100:	// n, PCR (eight-bit)
						addCycles(1);
						BUS().ADDRESS() = PC() + (int8_t)fetchByte();
						break;
					case 0b1101:	// n, PCR (sixteen-bit)
						addCycles(1);
						BUS().ADDRESS() = PC() + (int16_t)fetchWord().word;
						break;
					default:
						ASSUME(false);
					}
					if (indirect) {
						addCycles(3);
						BUS().ADDRESS() = fetchWord();
					}
				}
				break;
			default:
				UNREACHABLE;
			}
		}

		void Address_extended() {
			BUS().ADDRESS() = fetchWord();
		}

		//

		uint8_t AM_direct() {
			Address_direct();
			return BUS().read();
		}

		uint8_t AM_indexed() {
			Address_indexed();
			return BUS().read();
		}

		uint8_t AM_extended() {
			AM_extended();
			return BUS().read();
		}

		//

		void abx();
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
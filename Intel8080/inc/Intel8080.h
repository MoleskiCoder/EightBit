#pragma once

// Auxiliary carry logic from https://github.com/begoon/i8080-core

#include "IntelProcessor.h"
#include "InputOutput.h"

namespace EightBit {
	class Intel8080 : public IntelProcessor {
	public:
		enum StatusBits {
			SF = Bit7,
			ZF = Bit6,
			AC = Bit4,
			PF = Bit2,
			CF = Bit0,
		};

		Intel8080(Memory& memory, InputOutput& ports);

		Signal<Intel8080> ExecutingInstruction;

		bool isInterruptable() const;

		int interrupt(uint8_t value);

		int step();

		virtual register16_t& AF() override {
			auto& f = af.low;
			f = (f | Bit1) & ~(Bit5 | Bit3);
			return af;
		}

		virtual register16_t& BC() override {
			return bc;
		}

		virtual register16_t& DE() override {
			return de;
		}

		virtual register16_t& HL() override {
			return hl;
		}

		virtual void initialise();

	private:
		InputOutput& m_ports;

		register16_t af;
		register16_t bc;
		register16_t de;
		register16_t hl;

		bool m_interrupt;

		uint8_t& R(int r) {
			__assume(r < 8);
			__assume(r >= 0);
			switch (r) {
			case 0b000:
				return B();
			case 0b001:
				return C();
			case 0b010:
				return D();
			case 0b011:
				return E();
			case 0b100:
				return H();
			case 0b101:
				return L();
			case 0b110:
				m_memory.ADDRESS() = HL();
				return m_memory.reference();
			case 0b111:
				return A();
			default:
				__assume(0);
			}
			throw std::logic_error("Unhandled registry mechanism");
		}

		register16_t& RP(int rp) {
			__assume(rp < 4);
			__assume(rp >= 0);
			switch (rp) {
			case 0b00:
				return BC();
			case 0b01:
				return DE();
			case 0b10:
				return HL();
			case 0b11:
				return SP();
			default:
				__assume(0);
			}
		}

		register16_t& RP2(int rp) {
			__assume(rp < 4);
			__assume(rp >= 0);
			switch (rp) {
			case 0b00:
				return BC();
			case 0b01:
				return DE();
			case 0b10:
				return HL();
			case 0b11:
				return AF();
			default:
				__assume(0);
			}
		}

		static void adjustAuxiliaryCarryAdd(uint8_t& f, uint8_t before, uint8_t value, int calculation) {
			setFlag(f, AC, calculateHalfCarryAdd(before, value, calculation));
		}

		static void adjustAuxiliaryCarrySub(uint8_t& f, uint8_t before, uint8_t value, int calculation) {
			clearFlag(f, AC, calculateHalfCarrySub(before, value, calculation));
		}

		static void subtract(uint8_t& f, uint8_t& operand, uint8_t value, int carry = 0);

		virtual int execute(uint8_t opcode);
		void execute(int x, int y, int z, int p, int q);

		static void increment(uint8_t& f, uint8_t& operand);

		static void decrement(uint8_t& f, uint8_t& operand);

		bool returnConditionalFlag(uint8_t& f, int flag);
		bool jumpConditionalFlag(uint8_t& f, int flag);
		bool callConditionalFlag(uint8_t& f, int flag);

		static void add(uint8_t& f, register16_t& operand, register16_t value);

		static void add(uint8_t& f, uint8_t& operand, uint8_t value, int carry = 0);
		static void adc(uint8_t& f, uint8_t& operand, uint8_t value);
		static void sbb(uint8_t& f, uint8_t& operand, uint8_t value);
		static void andr(uint8_t& f, uint8_t& operand, uint8_t value);
		static void xorr(uint8_t& f, uint8_t& operand, uint8_t value);
		static void orr(uint8_t& f, uint8_t& operand, uint8_t value);
		static void compare(uint8_t& f, uint8_t check, uint8_t value);

		void rlc();
		void rrc();
		void ral();
		void rar();

		void daa();

		void cma();
		void stc();
		void cmc();

		void xhtl();

		void out();
		void in();

		void ei();
		void di();
	};
}
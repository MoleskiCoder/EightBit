#pragma once

// Auxiliary carry logic from https://github.com/begoon/i8080-core

#include <cstdint>
#include <stdexcept>

#include <Bus.h>
#include <InputOutput.h>
#include <IntelProcessor.h>
#include <Signal.h>
#include <Register.h>

namespace EightBit {
	class Intel8080 final : public IntelProcessor {
	public:
		enum StatusBits {
			SF = Bit7,
			ZF = Bit6,
			AC = Bit4,
			PF = Bit2,
			CF = Bit0,
		};

		Intel8080(Bus& bus, InputOutput& ports);

		Signal<Intel8080> ExecutingInstruction;

		virtual int execute(uint8_t opcode) final;
		virtual int step() final;

		virtual register16_t& AF() final;
		virtual register16_t& BC() final;
		virtual register16_t& DE() final;
		virtual register16_t& HL() final;

	protected:
		virtual void reset() final;

	private:
		bool m_interruptEnable = false;

		InputOutput& m_ports;

		register16_t af;
		register16_t bc = { { 0xff, 0xff } };
		register16_t de = { { 0xff, 0xff } };
		register16_t hl = { { 0xff, 0xff } };

		uint8_t R(int r, uint8_t a) {
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
				return BUS().read(HL());
			case 0b111:
				return a;
			default:
				UNREACHABLE;
			}
			throw std::logic_error("Unhandled registry mechanism");
		}

		void R(int r, uint8_t& a, uint8_t value) {
			switch (r) {
			case 0b000:
				B() = value;
				break;
			case 0b001:
				C() = value;
				break;
			case 0b010:
				D() = value;
				break;
			case 0b011:
				E() = value;
				break;
			case 0b100:
				H() = value;
				break;
			case 0b101:
				L() = value;
				break;
			case 0b110:
				BUS().write(HL(), value);
				break;
			case 0b111:
				a = value;
				break;
			default:
				UNREACHABLE;
			}
		}

		register16_t& RP(int rp) {
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
				UNREACHABLE;
			}
		}

		register16_t& RP2(int rp) {
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
				UNREACHABLE;
			}
		}

		static void adjustAuxiliaryCarryAdd(uint8_t& f, uint8_t before, uint8_t value, int calculation) {
			setFlag(f, AC, calculateHalfCarryAdd(before, value, calculation));
		}

		static void adjustAuxiliaryCarrySub(uint8_t& f, uint8_t before, uint8_t value, int calculation) {
			clearFlag(f, AC, calculateHalfCarrySub(before, value, calculation));
		}

		static void subtract(uint8_t& f, uint8_t& operand, uint8_t value, int carry = 0);

		void execute(uint8_t& a, uint8_t& f, int x, int y, int z, int p, int q);

		static void increment(uint8_t& f, uint8_t& operand);
		static void decrement(uint8_t& f, uint8_t& operand);

		void di();
		void ei();

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

		static void rlc(uint8_t& f, uint8_t& operand);
		static void rrc(uint8_t& f, uint8_t& operand);
		static void rl(uint8_t& f, uint8_t& operand);
		static void rr(uint8_t& f, uint8_t& operand);

		static void daa(uint8_t& a, uint8_t& f);

		static void cma(uint8_t& a, uint8_t& f);
		static void stc(uint8_t& a, uint8_t& f);
		static void cmc(uint8_t& a, uint8_t& f);

		void xhtl(register16_t& operand);

		void writePort(uint8_t port, uint8_t a);
		void writePort();

		void readPort(uint8_t port, uint8_t& a);
		void readPort();
	};
}
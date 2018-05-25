#pragma once

#include <cstdint>
#include <cassert>

#include <IntelProcessor.h>
#include <Signal.h>
#include <Register.h>

namespace EightBit {
	namespace GameBoy {

		class Bus;

		class LR35902 final : public IntelProcessor {
		public:
			enum StatusBits {
				ZF = Bit7,
				NF = Bit6,
				HC = Bit5,
				CF = Bit4,
			};

			LR35902(Bus& memory);

			Signal<LR35902> ExecutingInstruction;
			Signal<LR35902> ExecutedInstruction;

			int clockCycles() const {
				return cycles() * 4;
			}

			virtual register16_t& AF() final {
				af.low = higherNibble(af.low);
				return af;
			}

			virtual register16_t& BC() final { return bc; }
			virtual register16_t& DE() final { return de; }
			virtual register16_t& HL() final { return hl; }

			int singleStep();

		protected:
			virtual void reset() final;
			virtual int execute(uint8_t opcode) final;
			virtual int step() final;

		private:
			Bus& m_bus;

			register16_t af = { { 0xff, 0xff } };
			register16_t bc = { { 0xff, 0xff } };
			register16_t de = { { 0xff, 0xff } };
			register16_t hl = { { 0xff, 0xff } };

			bool m_ime = false;
			bool m_stopped = false;

			bool m_prefixCB = false;

			bool& IME() { return m_ime; }

			uint8_t R(int r, uint8_t& a) {
				switch (r) {
				case 0:
					return B();
				case 1:
					return C();
				case 2:
					return D();
				case 3:
					return E();
				case 4:
					return H();
				case 5:
					return L();
				case 6:
					return BUS().read(HL());
				case 7:
					return a;
				default:
					UNREACHABLE;
				}
				throw std::logic_error("Unhandled registry mechanism");
			}

			void R(int r, uint8_t& a, uint8_t value) {
				switch (r) {
				case 0:
					B() = value;
					break;
				case 1:
					C() = value;
					break;
				case 2:
					D() = value;
					break;
				case 3:
					E() = value;
					break;
				case 4:
					H() = value;
					break;
				case 5:
					L() = value;
					break;
				case 6:
					BUS().write(HL(), value);
					break;
				case 7:
					a = value;
					break;
				default:
					UNREACHABLE;
				}
			}

			register16_t& RP(int rp) {
				assert(rp < 4);
				assert(rp >= 0);
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
				assert(rp < 4);
				assert(rp >= 0);
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

			static void adjustHalfCarryAdd(uint8_t& f, uint8_t before, uint8_t value, int calculation) {
				setFlag(f, HC, calculateHalfCarryAdd(before, value, calculation));
			}

			static void adjustHalfCarrySub(uint8_t& f, uint8_t before, uint8_t value, int calculation) {
				setFlag(f, HC, calculateHalfCarrySub(before, value, calculation));
			}

			static void subtract(uint8_t& f, uint8_t& operand, uint8_t value, int carry = 0);

			void executeCB(uint8_t& a, uint8_t& f, int x, int y, int z, int p, int q);
			void executeOther(uint8_t& a, uint8_t& f, int x, int y, int z, int p, int q);

			static void increment(uint8_t& f, uint8_t& operand);
			static void decrement(uint8_t& f, uint8_t& operand);

			void stop() { m_stopped = true; }
			void start() { m_stopped = false; }
			bool stopped() const { return m_stopped; }

			void di();
			void ei();

			void reti();

			bool jrConditionalFlag(uint8_t f, int flag);
			bool returnConditionalFlag(uint8_t f, int flag);
			bool jumpConditionalFlag(uint8_t f, int flag);
			bool callConditionalFlag(uint8_t f, int flag);

			void add(uint8_t& f, register16_t& operand, register16_t value);

			static void add(uint8_t& f, uint8_t& operand, uint8_t value, int carry = 0);
			static void adc(uint8_t& f, uint8_t& operand, uint8_t value);
			static void sbc(uint8_t& f, uint8_t& operand, uint8_t value);
			static void andr(uint8_t& f, uint8_t& operand, uint8_t value);
			static void xorr(uint8_t& f, uint8_t& operand, uint8_t value);
			static void orr(uint8_t& f, uint8_t& operand, uint8_t value);
			static void compare(uint8_t& f, uint8_t check, uint8_t value);

			static uint8_t rlc(uint8_t& f, uint8_t operand);
			static uint8_t rrc(uint8_t& f, uint8_t operand);
			static uint8_t rl(uint8_t& f, uint8_t operand);
			static uint8_t rr(uint8_t& f, uint8_t operand);
			static uint8_t sla(uint8_t& f, uint8_t operand);
			static uint8_t sra(uint8_t& f, uint8_t operand);
			static uint8_t srl(uint8_t& f, uint8_t operand);

			static uint8_t bit(uint8_t& f, int n, uint8_t operand);
			static uint8_t res(int n, uint8_t operand);
			static uint8_t set(int n, uint8_t operand);

			static void daa(uint8_t& a, uint8_t& f);

			static void scf(uint8_t& f);
			static void ccf(uint8_t& f);
			static void cpl(uint8_t& a, uint8_t& f);

			static uint8_t swap(uint8_t& f, uint8_t operand);
		};
	}
}
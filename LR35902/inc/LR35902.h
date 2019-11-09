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

			auto clockCycles() const {
				return cycles() * 4;
			}

			virtual register16_t& AF() final;
			virtual register16_t& BC() final;
			virtual register16_t& DE() final;
			virtual register16_t& HL() final;

		protected:
			virtual int execute() final;
			virtual int step() final;

			virtual void handleRESET() final;
			virtual void handleINT() final;

		private:
			Bus& m_bus;

			register16_t af = 0xffff;
			register16_t bc = 0xffff;
			register16_t de = 0xffff;
			register16_t hl = 0xffff;

			bool m_ime = false;
			bool m_stopped = false;

			bool m_prefixCB = false;

			bool& IME() { return m_ime; }

			auto R(const int r) {
				ASSUME(r >= 0);
				ASSUME(r <= 7);
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
					return busRead(HL());
				case 7:
					return A();
				default:
					UNREACHABLE;
				}
			}

			void R(const int r, const uint8_t value) {
				ASSUME(r >= 0);
				ASSUME(r <= 7);
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
					busWrite(HL(), value);
					break;
				case 7:
					A() = value;
					break;
				default:
					UNREACHABLE;
				}
			}

			auto& RP(const int rp) {
				ASSUME(rp >= 0);
				ASSUME(rp <= 3);
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

			auto& RP2(const int rp) {
				ASSUME(rp >= 0);
				ASSUME(rp <= 3);
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

			void adjustHalfCarryAdd(const uint8_t before, const uint8_t value, const int calculation) {
				F() = setBit(F(), HC, calculateHalfCarryAdd(before, value, calculation));
			}

			void adjustHalfCarrySub(const uint8_t before, const uint8_t value, const int calculation) {
				F() = setBit(F(), HC, calculateHalfCarrySub(before, value, calculation));
			}

			void subtract(uint8_t& operand, uint8_t value, int carry = 0);

			void executeCB(int x, int y, int z, int p, int q);
			void executeOther(int x, int y, int z, int p, int q);

			void increment(uint8_t& operand);
			void decrement(uint8_t& operand);

			void stop() { m_stopped = true; }
			void start() { m_stopped = false; }
			bool stopped() const { return m_stopped; }

			void di();
			void ei();

			void reti();

			bool jrConditionalFlag(int flag);
			bool returnConditionalFlag(int flag);
			bool jumpConditionalFlag(int flag);
			bool callConditionalFlag(int flag);

			void add(register16_t& operand, register16_t value);

			void add(uint8_t& operand, uint8_t value, int carry = 0);
			void adc(uint8_t& operand, uint8_t value);
			void sbc(uint8_t value);
			void andr(uint8_t& operand, uint8_t value);
			void xorr(uint8_t value);
			void orr(uint8_t value);
			void compare(uint8_t check, uint8_t value);

			uint8_t rlc(uint8_t operand);
			uint8_t rrc(uint8_t operand);
			uint8_t rl(uint8_t operand);
			uint8_t rr(uint8_t operand);
			uint8_t sla(uint8_t operand);
			uint8_t sra(uint8_t operand);
			uint8_t srl(uint8_t operand);

			void bit(int n, uint8_t operand);
			static uint8_t res(int n, uint8_t operand);
			static uint8_t set(int n, uint8_t operand);

			void daa();

			void scf();
			void ccf();
			void cpl();

			uint8_t swap(uint8_t operand);
		};
	}
}
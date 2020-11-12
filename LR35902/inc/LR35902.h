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

			[[nodiscard]] auto clockCycles() const noexcept {
				return cycles() * 4;
			}

			[[nodiscard]] register16_t& AF() final;
			[[nodiscard]] register16_t& BC() final;
			[[nodiscard]] register16_t& DE() final;
			[[nodiscard]] register16_t& HL() final;

			bool& IME() noexcept { return m_ime; }

			[[nodiscard]] uint8_t enabledInterrupts();
			[[nodiscard]] uint8_t flaggedInterrupts();
			[[nodiscard]] uint8_t maskedInterrupts();

		protected:
			virtual int execute() final;
			virtual int step() final;

			void handleRESET() final;
			void handleINT() final;

			void jr(int8_t offset) final {
				IntelProcessor::jr(offset);
				tick(5);
			}

			int callConditional(const int condition) final {
				if (IntelProcessor::callConditional(condition))
					tick(3);
				tick(3);
				return condition;
			}

			int jumpConditional(const int condition) final {
				IntelProcessor::jumpConditional(condition);
				tick(3);
				return condition;
			}

			int returnConditional(const int condition) final {
				if (IntelProcessor::returnConditional(condition))
					tick(3);
				tick(2);
				return condition;
			}

			int jrConditional(const int condition) final {
				if (IntelProcessor::jrConditional(condition))
					tick();
				tick(2);
				return condition;
			}

		private:
			Bus& m_bus;

			register16_t af = 0xffff;
			register16_t bc = 0xffff;
			register16_t de = 0xffff;
			register16_t hl = 0xffff;

			bool m_ime = false;
			bool m_stopped = false;

			bool m_prefixCB = false;

			[[nodiscard]] auto R(const int r) {
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
					return IntelProcessor::memoryRead(HL());
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
					IntelProcessor::memoryWrite(HL(), value);
					break;
				case 7:
					A() = value;
					break;
				default:
					UNREACHABLE;
				}
			}

			[[nodiscard]] auto& RP(const int rp) {
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

			[[nodiscard]] auto& RP2(const int rp) {
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

			[[nodiscard]] static auto adjustHalfCarryAdd(uint8_t f, const uint8_t before, const uint8_t value, const int calculation) {
				return setBit(f, HC, calculateHalfCarryAdd(before, value, calculation));
			}

			[[nodiscard]] static auto adjustHalfCarrySub(uint8_t f, const uint8_t before, const uint8_t value, const int calculation) {
				return setBit(f, HC, calculateHalfCarrySub(before, value, calculation));
			}

			[[nodiscard]] static bool convertCondition(uint8_t f, int flag);

			static uint8_t subtract(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0);

			void executeCB(int x, int y, int z, int p, int q);
			void executeOther(int x, int y, int z, int p, int q);

			[[nodiscard]] static uint8_t increment(uint8_t& f, uint8_t operand);
			[[nodiscard]] static uint8_t decrement(uint8_t& f, uint8_t operand);

			void stop(bool value = true) noexcept { m_stopped = value; }
			void start() noexcept { stop(false); }
			bool stopped() const noexcept { return m_stopped; }

			void di();
			void ei();

			void reti();

			void jrConditionalFlag(uint8_t f, int flag);
			void returnConditionalFlag(uint8_t f, int flag);
			void jumpConditionalFlag(uint8_t f, int flag);
			void callConditionalFlag(uint8_t f, int flag);

			[[nodiscard]] register16_t add(uint8_t& f, register16_t operand, register16_t value);

			[[nodiscard]] static uint8_t add(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0);
			[[nodiscard]] static uint8_t adc(uint8_t& f, uint8_t operand, uint8_t value);
			[[nodiscard]] static uint8_t sbc(uint8_t& f, uint8_t operand, uint8_t value);
			[[nodiscard]] static uint8_t andr(uint8_t& f, uint8_t operand, uint8_t value);
			[[nodiscard]] static uint8_t xorr(uint8_t& f, uint8_t operand, uint8_t value);
			[[nodiscard]] static uint8_t orr(uint8_t& f, uint8_t operand, uint8_t value);
			[[nodiscard]] static void compare(uint8_t& f, uint8_t operand, uint8_t value);

			[[nodiscard]] static uint8_t rlc(uint8_t& f, uint8_t operand);
			[[nodiscard]] static uint8_t rrc(uint8_t& f, uint8_t operand);
			[[nodiscard]] static uint8_t rl(uint8_t& f, uint8_t operand);
			[[nodiscard]] static uint8_t rr(uint8_t& f, uint8_t operand);
			[[nodiscard]] static uint8_t sla(uint8_t& f, uint8_t operand);
			[[nodiscard]] static uint8_t sra(uint8_t& f, uint8_t operand);
			[[nodiscard]] static uint8_t srl(uint8_t& f, uint8_t operand);

			static void bit(uint8_t& f, int n, uint8_t operand);
			[[nodiscard]] static uint8_t res(int n, uint8_t operand);
			[[nodiscard]] static uint8_t set(int n, uint8_t operand);

			[[nodiscard]] static uint8_t daa(uint8_t& f, uint8_t operand);

			static void scf(uint8_t& f, uint8_t operand);
			static void ccf(uint8_t& f, uint8_t operand);
			static uint8_t cpl(uint8_t& f, uint8_t operand);

			static uint8_t swap(uint8_t& f, uint8_t operand);
		};
	}
}
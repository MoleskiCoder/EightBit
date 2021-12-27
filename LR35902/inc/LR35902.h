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

			[[nodiscard]] const register16_t& AF() const noexcept final;
			[[nodiscard]] auto& AF() noexcept { return IntelProcessor::AF(); }
			[[nodiscard]] const register16_t& BC() const noexcept final;
			[[nodiscard]] auto& BC() noexcept { return IntelProcessor::BC(); }
			[[nodiscard]] const register16_t& DE() const noexcept final;
			[[nodiscard]] auto& DE() noexcept { return IntelProcessor::DE(); }
			[[nodiscard]] const register16_t& HL() const noexcept final;
			[[nodiscard]] auto& HL() noexcept { return IntelProcessor::HL(); }

			bool& IME() noexcept { return m_ime; }

			[[nodiscard]] uint8_t enabledInterrupts() noexcept;
			[[nodiscard]] uint8_t flaggedInterrupts() noexcept;
			[[nodiscard]] uint8_t maskedInterrupts() noexcept;

			Signal<EventArgs> MachineTicked;

			void tickMachine(const int extra) { for (int i = 0; i < extra; ++i) tickMachine(); }
			void tickMachine() { tick(4); MachineTicked.fire(); }

		protected:
			int execute() final;
			int step() final;

			void handleRESET() final;
			void handleINT() final;

			void memoryWrite() final;
			uint8_t memoryRead() final;

			void pushWord(register16_t value) final;

			void jr(int8_t offset) final;
			int jumpConditional(int condition) final;
			int returnConditional(int condition) final;
			int jrConditional(int condition) final;
			void ret() final;

		private:
			Bus& m_bus;

			register16_t af = 0xffff;
			register16_t bc = 0xffff;
			register16_t de = 0xffff;
			register16_t hl = 0xffff;

			bool m_ime = false;
			bool m_stopped = false;

			bool m_prefixCB = false;

			[[nodiscard]] uint8_t R(int r);
			void R(int r, uint8_t value);

			[[nodiscard]] register16_t& RP(int rp) noexcept;
			[[nodiscard]] register16_t& RP2(int rp) noexcept;

			[[nodiscard]] static constexpr auto adjustHalfCarryAdd(uint8_t f, const uint8_t before, const uint8_t value, const int calculation) noexcept {
				return setBit(f, HC, calculateHalfCarryAdd(before, value, calculation));
			}

			[[nodiscard]] static constexpr auto adjustHalfCarrySub(uint8_t f, const uint8_t before, const uint8_t value, const int calculation) noexcept {
				return setBit(f, HC, calculateHalfCarrySub(before, value, calculation));
			}

			[[nodiscard]] static bool convertCondition(uint8_t f, int flag) noexcept;

			static uint8_t subtract(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0) noexcept;

			void executeCB(int x, int y, int z, int p, int q);
			void executeOther(int x, int y, int z, int p, int q);

			[[nodiscard]] static uint8_t increment(uint8_t& f, uint8_t operand) noexcept;
			[[nodiscard]] static uint8_t decrement(uint8_t& f, uint8_t operand) noexcept;

			void stop(bool value = true) noexcept { m_stopped = value; }
			void start() noexcept { stop(false); }
			[[nodiscard]] bool stopped() const noexcept { return m_stopped; }

			void di() noexcept;
			void ei() noexcept;

			void reti();

			void jrConditionalFlag(uint8_t f, int flag);
			void returnConditionalFlag(uint8_t f, int flag);
			void jumpConditionalFlag(uint8_t f, int flag);
			void callConditionalFlag(uint8_t f, int flag);

			[[nodiscard]] register16_t add(uint8_t& f, register16_t operand, register16_t value);

			[[nodiscard]] static uint8_t add(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0) noexcept;
			[[nodiscard]] static uint8_t adc(uint8_t& f, uint8_t operand, uint8_t value) noexcept;
			[[nodiscard]] static uint8_t sbc(uint8_t& f, uint8_t operand, uint8_t value) noexcept;
			static uint8_t andr(uint8_t& f, uint8_t operand, uint8_t value) noexcept;
			[[nodiscard]] static uint8_t xorr(uint8_t& f, uint8_t operand, uint8_t value) noexcept;
			[[nodiscard]] static uint8_t orr(uint8_t& f, uint8_t operand, uint8_t value) noexcept;
			[[nodiscard]] static void compare(uint8_t& f, uint8_t operand, uint8_t value) noexcept;

			[[nodiscard]] static uint8_t rlc(uint8_t& f, uint8_t operand) noexcept;
			[[nodiscard]] static uint8_t rrc(uint8_t& f, uint8_t operand) noexcept;
			[[nodiscard]] static uint8_t rl(uint8_t& f, uint8_t operand) noexcept;
			[[nodiscard]] static uint8_t rr(uint8_t& f, uint8_t operand) noexcept;
			[[nodiscard]] static uint8_t sla(uint8_t& f, uint8_t operand) noexcept;
			[[nodiscard]] static uint8_t sra(uint8_t& f, uint8_t operand) noexcept;
			[[nodiscard]] static uint8_t srl(uint8_t& f, uint8_t operand) noexcept;

			static void bit(uint8_t& f, int n, uint8_t operand) noexcept;
			[[nodiscard]] static uint8_t res(int n, uint8_t operand) noexcept;
			[[nodiscard]] static uint8_t set(int n, uint8_t operand) noexcept;

			[[nodiscard]] static uint8_t daa(uint8_t& f, uint8_t operand) noexcept;

			static void scf(uint8_t& f, uint8_t operand) noexcept;
			static void ccf(uint8_t& f, uint8_t operand) noexcept;
			[[nodiscard]] static uint8_t cpl(uint8_t& f, uint8_t operand) noexcept;

			[[nodiscard]] static uint8_t swap(uint8_t& f, uint8_t operand) noexcept;
		};
	}
}
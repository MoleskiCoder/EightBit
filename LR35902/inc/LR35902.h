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
			DECLARE_PIN_OUTPUT(MWR)
			DECLARE_PIN_OUTPUT(RD)
			DECLARE_PIN_OUTPUT(WR)

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

			[[nodiscard]] register16_t& AF() noexcept final;
			[[nodiscard]] register16_t& BC() noexcept final;
			[[nodiscard]] register16_t& DE() noexcept final;
			[[nodiscard]] register16_t& HL() noexcept final;

			bool& IME() { return m_ime; }

			[[nodiscard]] uint8_t enabledInterrupts();
			[[nodiscard]] uint8_t flaggedInterrupts();
			[[nodiscard]] uint8_t maskedInterrupts();

			Signal<EventArgs> MachineTicked;

			void tickMachine(const int extra) { for (int i = 0; i < extra; ++i) tickMachine(); }
			void tickMachine() { tick(4); MachineTicked.fire(); }

		protected:
			void execute() noexcept final;
			void poweredStep() noexcept final;

			void handleRESET() noexcept final;
			void handleINT() noexcept final;

			void memoryUpdate(int ticks) noexcept;
			void memoryWrite() noexcept final;
			uint8_t memoryRead() noexcept final;

			void pushWord(register16_t value) noexcept final;

			void jumpRelative(int8_t offset) noexcept final;
			void jumpConditional(bool condition) noexcept final;
			void returnConditional(bool condition) noexcept final;
			void ret() noexcept final;
			void jumpIndirect() noexcept final;

		private:
			Bus& m_bus;

			register16_t m_af = 0xffff;
			register16_t m_bc = 0xffff;
			register16_t m_de = 0xffff;
			register16_t m_hl = 0xffff;

			bool m_ime = false;
			bool m_stopped = false;
			bool m_ei = false;

			bool m_prefixCB = false;

			[[nodiscard]] uint8_t R(int r);
			void R(int r, uint8_t value);

			[[nodiscard]] register16_t& RP(int rp);
			[[nodiscard]] register16_t& RP2(int rp);

			[[nodiscard]] static constexpr auto adjustHalfCarryAdd(uint8_t f, const uint8_t before, const uint8_t value, const int calculation) {
				return setBit(f, HC, calculateHalfCarryAdd(before, value, calculation));
			}

			[[nodiscard]] static constexpr auto adjustHalfCarrySub(uint8_t f, const uint8_t before, const uint8_t value, const int calculation) {
				return setBit(f, HC, calculateHalfCarrySub(before, value, calculation));
			}

			[[nodiscard]] bool convertCondition(int flag) noexcept final;

			static uint8_t subtract(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0);

			void executeCB(int x, int y, int z, int p, int q);
			void executeOther(int x, int y, int z, int p, int q);

			[[nodiscard]] static uint8_t increment(uint8_t& f, uint8_t operand);
			[[nodiscard]] static uint8_t decrement(uint8_t& f, uint8_t operand);

			void stop(bool value = true) { m_stopped = value; }
			void start() { stop(false); }
			[[nodiscard]] bool stopped() const { return m_stopped; }

			[[nodiscard]] constexpr bool& EI() noexcept { return m_ei; }

			virtual void disableInterrupts() noexcept final;
			virtual void enableInterrupts() noexcept final;

			void reti();

			[[nodiscard]] register16_t add(uint8_t& f, register16_t operand, register16_t value);

			[[nodiscard]] static uint8_t add(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0);
			[[nodiscard]] static uint8_t adc(uint8_t& f, uint8_t operand, uint8_t value);
			[[nodiscard]] static uint8_t sbc(uint8_t& f, uint8_t operand, uint8_t value);
			static uint8_t andr(uint8_t& f, uint8_t operand, uint8_t value);
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
			[[nodiscard]] static uint8_t cpl(uint8_t& f, uint8_t operand);

			[[nodiscard]] static uint8_t swap(uint8_t& f, uint8_t operand);
		};
	}
}
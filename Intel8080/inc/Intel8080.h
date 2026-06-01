#pragma once

// Auxiliary carry logic from https://github.com/begoon/i8080-core

#include <cassert>
#include <cstdint>
#include <stdexcept>

#include <Bus.h>
#include <IntelProcessor.h>
#include <EventArgs.h>
#include <Signal.h>
#include <Register.h>

namespace EightBit {
	class Intel8080 final : public IntelProcessor {
	public:
		DECLARE_PIN_OUTPUT(DBIN)	// Active high
		DECLARE_PIN_OUTPUT(WR)		// Active low

	public:
		enum StatusBits {
			SF = Bit7,
			ZF = Bit6,
			AC = Bit4,
			PF = Bit2,
			CF = Bit0,
		};

		Intel8080(Bus& bus);

		void execute() noexcept final;
		void poweredStep() noexcept final;

		[[nodiscard]] register16_t& AF() noexcept final;
		[[nodiscard]] register16_t& BC() noexcept final;
		[[nodiscard]] register16_t& DE() noexcept final;
		[[nodiscard]] register16_t& HL() noexcept final;

		[[nodiscard]] constexpr bool requestingIO() const noexcept { return m_requestIO; }
		[[nodiscard]] constexpr bool requestingMemory() const noexcept { return m_requestMemory; }

		[[nodiscard]] constexpr bool requestingRead() const noexcept { return raised(DBIN()); }
		[[nodiscard]] constexpr bool requestingWrite() const noexcept { return lowered(WR()); }

	protected:
		void handleRESET() noexcept final;
		void handleINT() noexcept final;

		void memoryUpdate(int ticks = 1) noexcept final;
		void memoryWrite() noexcept final;
		void memoryRead() noexcept final;

	private:
		bool m_requestIO = false;
		bool m_requestMemory = false;

		bool m_interruptEnable = false;

		register16_t m_af;
		register16_t m_bc = Mask16;
		register16_t m_de = Mask16;
		register16_t m_hl = Mask16;

		[[nodiscard]] uint8_t& R(int r, MemoryMapping::AccessLevel access = MemoryMapping::AccessLevel::ReadOnly) noexcept final;
		register16_t& RP(int rp);
		register16_t& RP2(int rp);

		[[nodiscard]] static constexpr auto zeroTest(uint8_t data) noexcept { return data & ZF; }
		[[nodiscard]] static constexpr auto carryTest(uint8_t data) noexcept { return data & CF; }
		[[nodiscard]] static constexpr auto parityTest(uint8_t data) noexcept { return data & PF; }
		[[nodiscard]] static constexpr auto signTest(uint8_t data) noexcept { return data & SF; }
		[[nodiscard]] static constexpr auto auxiliaryCarryTest(uint8_t data) noexcept { return data & AC; }

		[[nodiscard]] constexpr auto zero() noexcept { return zeroTest(F()); }
		[[nodiscard]] constexpr auto carry() noexcept { return carryTest(F()); }
		[[nodiscard]] constexpr auto parity() noexcept { return parityTest(F()); }
		[[nodiscard]] constexpr auto sign() noexcept { return signTest(F()); }
		[[nodiscard]] constexpr auto auxiliaryCarry() noexcept { return auxiliaryCarryTest(F()); }

		void adjustStatusFlags(uint8_t value) noexcept { F() = value; }

		void setBit(StatusBits flag) noexcept { return adjustStatusFlags(setBit(F(), flag)); }
		void setBit(int flag) noexcept { return setBit((StatusBits)flag); }
		[[nodiscard]] static constexpr uint8_t setBit(uint8_t f, StatusBits flag) noexcept { return Chip::setBit(f, (uint8_t)flag); }
		void setBit(StatusBits flag, int condition) noexcept { adjustStatusFlags(Chip::setBit(F(), flag, condition)); }
		[[nodiscard]] static constexpr uint8_t setBit(uint8_t f, StatusBits flag, int condition) noexcept { return Chip::setBit(f, (uint8_t)flag, condition); }
		void setBit(StatusBits flag, bool condition) noexcept { adjustStatusFlags(setBit(F(), flag, condition)); }
		void setBit(int flag, bool condition) noexcept { setBit((StatusBits)flag, condition); }
		[[nodiscard]] static constexpr uint8_t setBit(uint8_t f, StatusBits flag, bool condition) noexcept { return Chip::setBit(f, (uint8_t)flag, condition); }

		void clearBit(StatusBits flag) noexcept { adjustStatusFlags(clearBit(F(), flag)); }
		void clearBit(int flag) noexcept { clearBit((StatusBits)flag); }
		[[nodiscard]] static constexpr uint8_t clearBit(uint8_t f, StatusBits flag) noexcept { return Chip::clearBit(f, (uint8_t)flag); }
		void clearBit(StatusBits flag, int condition) noexcept { adjustStatusFlags(clearBit(F(), flag, condition)); }
		[[nodiscard]] static constexpr uint8_t clearBit(uint8_t f, StatusBits flag, int condition) noexcept { return Chip::clearBit(f, (uint8_t)flag, condition); }

		void adjustSign(uint8_t value) noexcept { adjustStatusFlags(adjustSign(F(), value)); }
		[[nodiscard]] static constexpr uint8_t adjustSign(uint8_t input, uint8_t value) noexcept { return setBit(input, SF, signTest(value)); }

		void adjustZero(uint8_t value) noexcept { adjustStatusFlags(adjustZero(F(), value)); }
		[[nodiscard]] static constexpr uint8_t adjustZero(uint8_t input, uint8_t value) noexcept { return clearBit(input, ZF, value); }

		void adjustParity(uint8_t value) noexcept { adjustStatusFlags(adjustParity(F(), value)); }
		[[nodiscard]] static constexpr uint8_t adjustParity(uint8_t input, uint8_t value) noexcept { return setBit(input, PF, evenParity(value)); }

		void adjustSZ(uint8_t value) noexcept { adjustStatusFlags(adjustSZ(F(), value)); }
		[[nodiscard]] static constexpr uint8_t adjustSZ(uint8_t input, uint8_t value) noexcept {
			input = adjustSign(input, value);
			return adjustZero(input, value);
		}

		void adjustSZP(uint8_t value) noexcept { adjustStatusFlags(adjustSZP(F(), value)); }
		[[nodiscard]] static constexpr uint8_t adjustSZP(uint8_t input, uint8_t value) noexcept {
			input = adjustSZ(input, value);
			return adjustParity(input, value);
		}

		[[nodiscard]] static constexpr auto adjustAuxilliaryCarryAdd(uint8_t f, uint8_t before, uint8_t value, int calculation) noexcept {
			return setBit(f, AC, calculateHalfCarryAdd(before, value, calculation));
		}

		void adjustAuxiliaryCarryAdd(uint8_t before, uint8_t value, int calculation) noexcept {
			adjustStatusFlags(adjustAuxilliaryCarryAdd(F(), before, value, calculation));
		}

		[[nodiscard]] static constexpr auto adjustAuxiliaryCarrySub(uint8_t f, uint8_t before, uint8_t value, int calculation) noexcept {
			return setBit(f, AC, calculateHalfCarrySub(before, value, calculation));
		}

		void adjustAuxiliaryCarrySub(uint8_t before, uint8_t value, int calculation) noexcept {
			adjustStatusFlags(adjustAuxiliaryCarrySub(F(), before, value, calculation));
		}

		[[nodiscard]] bool convertCondition(int flag) noexcept final;

		void execute(int x, int y, int z, int p, int q);

		[[nodiscard]] uint8_t increment(uint8_t operand);
		[[nodiscard]] uint8_t decrement(uint8_t operand);

		void disableInterrupts() noexcept final;
		void enableInterrupts() noexcept final;

		void add(register16_t value);

		void add(uint8_t value, int carry = 0);
		void adc(uint8_t value);
		void sub(uint8_t value, int carry = 0) noexcept;
		uint8_t subtract(uint8_t value, int carry = 0);
		void sbb(uint8_t value);

		void andr(uint8_t value);
		void xorr(uint8_t value);
		void orr(uint8_t value);
		void compare(uint8_t value);

		void rlc();
		void rrc();
		void rl();
		void rr();

		void daa();

		void cma();
		void stc();
		void cmc();

		void xhtl(register16_t& exchange);

		void writePort(uint8_t port) noexcept;
		void writePort() noexcept;

		void readPort(uint8_t port) noexcept;
		void readPort() noexcept;

		constexpr void requestIO() noexcept { assert(!requestingMemory());  m_requestIO = true; }
		constexpr void releaseIO() noexcept { m_requestIO = false; }
		constexpr void requestMemory() noexcept { assert(!requestingIO()); m_requestMemory = true; }
		constexpr void releaseMemory() noexcept { m_requestMemory = false; }

		constexpr void requestRead() noexcept { raise(DBIN()); }
		constexpr void releaseRead() noexcept { lower(DBIN()); }
		constexpr void requestWrite() noexcept { lower(WR()); }
		constexpr void releaseWrite() noexcept { raise(WR()); }
	};
}
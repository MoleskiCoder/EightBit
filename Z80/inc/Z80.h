#pragma once

#include <cstdint>
#include <array>
#include <functional>

#include <IntelProcessor.h>
#include <InputOutput.h>
#include <EventArgs.h>
#include <Signal.h>
#include <Register.h>

namespace EightBit {

	class Bus;

	class Z80 final : public IntelProcessor {
	public:
		// ** From the Z80 CPU User Manual
		// RFSH.Refresh(output, active Low). RFSH, together with MREQ, indicates that the lower
		// seven bits of the system’s address bus can be used as a refresh address to the system’s
		// dynamic memories.
		DECLARE_PIN_OUTPUT(RFSH)

		DECLARE_PIN_INPUT(NMI)
		DECLARE_PIN_OUTPUT(M1)
		DECLARE_PIN_OUTPUT(MREQ)
		DECLARE_PIN_OUTPUT(IORQ)
		DECLARE_PIN_OUTPUT(RD)
		DECLARE_PIN_OUTPUT(WR)

	public:
		struct refresh_t {

			bool high : 1;
			uint8_t variable : 7;

			constexpr refresh_t(const uint8_t value) noexcept
			: high(!!(value & Bit7)),
			  variable(value & Mask7)
			{ }

			constexpr operator uint8_t() const noexcept {
				return (high << 7) | variable;
			}

			constexpr auto& operator++() noexcept {
				++variable;
				return *this;
			}
		};

		enum StatusBits {
			SF = Bit7,
			ZF = Bit6,
			YF = Bit5,
			HC = Bit4,
			XF = Bit3,
			PF = Bit2,
			VF = Bit2,
			NF = Bit1,
			CF = Bit0,
		};

		Z80(Bus& bus, InputOutput& ports) noexcept;

		Z80(const Z80& rhs) noexcept;
		bool operator==(const Z80& rhs) const noexcept;

		void execute() noexcept final;
		void poweredStep() noexcept final;

		[[nodiscard]] register16_t& AF() noexcept final;
		[[nodiscard]] register16_t& BC() noexcept final;
		[[nodiscard]] register16_t& DE() noexcept final;
		[[nodiscard]] register16_t& HL() noexcept final;

		[[nodiscard]] constexpr auto& IX() noexcept { return m_ix; }
		[[nodiscard]] constexpr auto& IXH() noexcept { return IX().high; }
		[[nodiscard]] constexpr auto& IXL() noexcept { return IX().low; }

		[[nodiscard]] constexpr auto& IY() noexcept { return m_iy; }
		[[nodiscard]] constexpr auto& IYH() noexcept { return IY().high; }
		[[nodiscard]] constexpr auto& IYL() noexcept { return IY().low; }

		// ** From the Z80 CPU User Manual
		// Memory Refresh(R) Register.The Z80 CPU contains a memory refresh counter,
		// enabling dynamic memories to be used with the same ease as static memories.Seven bits
		// of this 8-bit register are automatically incremented after each instruction fetch.The eighth
		// bit remains as programmed, resulting from an LD R, A instruction. The data in the refresh
		// counter is sent out on the lower portion of the address bus along with a refresh control
		// signal while the CPU is decoding and executing the fetched instruction. This mode of refresh
		// is transparent to the programmer and does not slow the CPU operation.The programmer
		// can load the R register for testing purposes, but this register is normally not used by the
		// programmer. During refresh, the contents of the I Register are placed on the upper eight
		// bits of the address bus.
		[[nodiscard]] constexpr auto& REFRESH() noexcept { return m_refresh; }

		[[nodiscard]] constexpr auto& IV() noexcept { return iv; }
		[[nodiscard]] constexpr auto& IM() noexcept { return m_interruptMode; }
		[[nodiscard]] constexpr auto& IFF1() noexcept { return m_iff1; }
		[[nodiscard]] constexpr auto& IFF2() noexcept { return m_iff2; }

		[[nodiscard]] constexpr auto& Q() noexcept { return m_q; }

		constexpr void exx() noexcept { m_registerSet ^= 1; }
		constexpr void exxAF() noexcept { m_accumulatorFlagsSet ^= 1; }

		[[nodiscard]] constexpr auto requestingIO() const noexcept { return lowered(IORQ()); }
		[[nodiscard]] constexpr auto requestingMemory() const noexcept { return lowered(MREQ()); }

		[[nodiscard]] constexpr auto requestingRead() const noexcept { return lowered(RD()); }
		[[nodiscard]] constexpr auto requestingWrite() const noexcept { return lowered(WR()); }

		[[nodiscard]] constexpr auto fetchingOpCode() const noexcept { return lowered(M1()); }

	protected:
		void handleRESET() noexcept final;
		void handleINT() noexcept final;

		void memoryUpdate(int ticks = 1) noexcept;
		void memoryWrite() noexcept final;
		uint8_t memoryRead() noexcept final;
		void refreshMemory() noexcept;

		void jumpRelative(int8_t offset) noexcept final;

		void call(register16_t destination) noexcept final;

	private:
		bool m_interruptPending = false;
		bool m_nonMaskableInterruptPending = false;
		bool m_resetPending = false;

		InputOutput& m_ports;
			
		enum { BC_IDX, DE_IDX, HL_IDX };

		std::array<std::array<register16_t, 3>, 2> m_registers;
		int m_registerSet = 0;

		std::array<register16_t, 2> m_accumulatorFlags;
		int m_accumulatorFlagsSet = 0;

		register16_t m_ix = 0xffff;
		register16_t m_iy = 0xffff;

		refresh_t m_refresh = 0x7f;

		uint8_t iv = 0xff;
		int m_interruptMode = 0;
		bool m_iff1 = false;
		bool m_iff2 = false;

		uint8_t m_q = 0;				// Previously modified instruction status register
		uint8_t m_modifiedF = 0;        // In-flight status register.  Used to build "Q"

		bool m_prefixCB = false;
		bool m_prefixDD = false;
		bool m_prefixED = false;
		bool m_prefixFD = false;

		int8_t m_displacement = 0;

		void handleNMI() noexcept;

		[[nodiscard]] constexpr auto displaced() const noexcept { return m_prefixDD || m_prefixFD; }

		[[nodiscard]] constexpr void displaceAddress() noexcept {
			const auto& index_register = m_prefixDD ? IX() : IY();
			BUS().ADDRESS() = MEMPTR() = index_register.word + m_displacement;
		}

		void fetchDisplacement() noexcept;
		[[nodiscard]] uint8_t fetchInstruction() noexcept final;

		uint8_t readDataUnderInterrupt() noexcept;

		typedef std::function<register16_t(void)> addresser_t;
		void loadAccumulatorIndirect(addresser_t addresser) noexcept;
		void storeAccumulatorIndirect(addresser_t addresser) noexcept;

		typedef std::function<uint8_t(void)> reader_t;
		void readInternalRegister(reader_t reader) noexcept;

		[[nodiscard]] register16_t& HL2() noexcept;
		[[nodiscard]] register16_t& RP(int rp) noexcept;
		[[nodiscard]] register16_t& RP2(int rp) noexcept;

		[[nodiscard]] uint8_t& R(int r, MemoryMapping::AccessLevel access = MemoryMapping::AccessLevel::ReadOnly) noexcept;
		void R(int r, uint8_t value, int ticks = 1) noexcept;
		void R2(int r, uint8_t value) noexcept;

		[[nodiscard]] static constexpr auto zeroTest(uint8_t data) noexcept { return data & ZF; }
		[[nodiscard]] static constexpr auto carryTest(uint8_t data) noexcept { return data & CF; }
		[[nodiscard]] static constexpr auto parityTest(uint8_t data) noexcept { return data & PF; }
		[[nodiscard]] static constexpr auto signTest(uint8_t data) noexcept { return data & SF; }
		[[nodiscard]] static constexpr auto halfCarryTest(uint8_t data) noexcept { return data & HC; }
		[[nodiscard]] static constexpr auto subtractingTest(uint8_t data) noexcept { return data & NF; }
		[[nodiscard]] static constexpr auto xTest(uint8_t data) noexcept { return data & XF; }
		[[nodiscard]] static constexpr auto yTest(uint8_t data) noexcept { return data & YF; }

		[[nodiscard]] constexpr auto zero() noexcept { return zeroTest(F()); }
		[[nodiscard]] constexpr auto carry() noexcept { return carryTest(F()); }
		[[nodiscard]] constexpr auto parity() noexcept { return parityTest(F()); }
		[[nodiscard]] constexpr auto sign() noexcept { return signTest(F()); }
		[[nodiscard]] constexpr auto halfCarry() noexcept { return halfCarryTest(F()); }
		[[nodiscard]] constexpr auto subtracting() noexcept { return subtractingTest(F()); }

		void adjustStatusFlags(uint8_t value) noexcept {
			m_modifiedF = F();
			F() = value;
		}

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

		void adjustXY(uint8_t value) noexcept { adjustStatusFlags(adjustXY(F(), value)); }
		[[nodiscard]] static constexpr uint8_t adjustXY(uint8_t input, uint8_t value) noexcept {
			input = setBit(input, XF, xTest(value));
			return setBit(input, YF, yTest(value));
		}

		void adjustSZPXY(uint8_t value) noexcept { adjustStatusFlags(adjustSZPXY(F(), value)); }
		[[nodiscard]] static constexpr uint8_t adjustSZPXY(uint8_t input, uint8_t value) noexcept {
			input = adjustSZP(input, value);
			return adjustXY(input, value);
		}

		void adjustSZXY(uint8_t value) noexcept { adjustStatusFlags(adjustSZXY(F(), value)); }
		[[nodiscard]] static constexpr uint8_t adjustSZXY(uint8_t input, uint8_t value) noexcept {
			input = adjustSZ(input, value);
			return adjustXY(input, value);
		}

		[[nodiscard]] static constexpr auto adjustHalfCarryAdd(uint8_t f, uint8_t before, uint8_t value, int calculation) noexcept {
			return setBit(f, HC, calculateHalfCarryAdd(before, value, calculation));
		}

		void adjustHalfCarryAdd(uint8_t before, uint8_t value, int calculation) noexcept {
			adjustStatusFlags(adjustHalfCarryAdd(F(), before, value, calculation));
		}

		[[nodiscard]] static constexpr auto adjustHalfCarrySub(uint8_t f, uint8_t before, uint8_t value, int calculation) noexcept {
			return setBit(f, HC, calculateHalfCarrySub(before, value, calculation));
		}

		void adjustHalfCarrySub(uint8_t before, uint8_t value, int calculation) noexcept {
			adjustStatusFlags(adjustHalfCarrySub(F(), before, value, calculation));
		}

		void adjustOverflowAdd(int beforeNegative, int valueNegative, int afterNegative) noexcept {
			adjustStatusFlags(adjustOverflowAdd(F(), beforeNegative, valueNegative, afterNegative));
		}

		[[nodiscard]] static constexpr uint8_t adjustOverflowAdd(uint8_t input, int beforeNegative, int valueNegative, int afterNegative) noexcept {
			const auto overflow = beforeNegative == valueNegative && beforeNegative != afterNegative;
			return setBit(input, VF, overflow);
		}

		void adjustOverflowAdd(uint8_t before, uint8_t value, uint8_t calculation) noexcept {
			adjustStatusFlags(adjustOverflowAdd(F(), before, value, calculation));
		}

		[[nodiscard]] static constexpr uint8_t adjustOverflowAdd(uint8_t input, uint8_t before, uint8_t value, uint8_t calculation) noexcept {
			return adjustOverflowAdd(input, signTest(before), signTest(value), signTest(calculation));
		}

		void adjustOverflowSub(int beforeNegative, int valueNegative, int afterNegative) noexcept {
			adjustStatusFlags(adjustOverflowSub(F(), beforeNegative, valueNegative, afterNegative));
		}

		[[nodiscard]] static constexpr uint8_t adjustOverflowSub(uint8_t input, int beforeNegative, int valueNegative, int afterNegative) noexcept {
			const auto overflow = beforeNegative != valueNegative && beforeNegative != afterNegative;
			return setBit(input, VF, overflow);
		}

		void adjustOverflowSub(uint8_t before, uint8_t value, uint8_t calculation) noexcept {
			adjustStatusFlags(adjustOverflowSub(F(), before, value, calculation));
		}

		[[nodiscard]] static constexpr uint8_t adjustOverflowSub(uint8_t input, uint8_t before, uint8_t value, uint8_t calculation) noexcept {
			return adjustOverflowSub(input, signTest(before), signTest(value), signTest(calculation));
		}

		[[nodiscard]] bool convertCondition(int flag) noexcept final;

		void executeCB(int x, int y, int z) noexcept;
		void executeED(int x, int y, int z, int p, int q) noexcept;
		void executeOther(int x, int y, int z, int p, int q) noexcept;

		[[nodiscard]] uint8_t increment(uint8_t operand) noexcept;
		[[nodiscard]] uint8_t decrement(uint8_t operand) noexcept;

		void disableInterrupts() noexcept final;
		void enableInterrupts() noexcept final;

		void retn() noexcept;
		void reti() noexcept;

		void returnConditionalFlag(int flag) noexcept final;

		void sbc(register16_t value) noexcept;
		void adc(register16_t value) noexcept;
		void add(register16_t value, int carry = 0) noexcept;

		void add(uint8_t value, int carry = 0) noexcept;
		void adc(uint8_t value) noexcept;
		void sub(uint8_t value, int carry = 0) noexcept;
		uint8_t subtract(uint8_t value, int carry = 0) noexcept;
		void sbc(uint8_t value) noexcept;

		void andr(uint8_t value) noexcept;
		void xorr(uint8_t value) noexcept;
		void orr(uint8_t value) noexcept;
		void compare(uint8_t value) noexcept;

		[[nodiscard]] uint8_t rlc(uint8_t operand) noexcept;
		[[nodiscard]] uint8_t rrc(uint8_t operand) noexcept;
		[[nodiscard]] uint8_t rl(uint8_t operand) noexcept;
		[[nodiscard]] uint8_t rr(uint8_t operand) noexcept;
		[[nodiscard]] uint8_t sla(uint8_t operand) noexcept;
		[[nodiscard]] uint8_t sra(uint8_t operand) noexcept;
		[[nodiscard]] uint8_t sll(uint8_t operand) noexcept;
		[[nodiscard]] uint8_t srl(uint8_t operand) noexcept;

		void bit(int n, uint8_t operand) noexcept;

		[[nodiscard]] static constexpr auto res(int n, uint8_t operand) noexcept { return Chip::clearBit(operand, Chip::bit(n)); }
		[[nodiscard]] static constexpr auto set(int n, uint8_t operand) noexcept { return Chip::setBit(operand, Chip::bit(n)); }

		void daa() noexcept;
		void scf() noexcept;
		void ccf() noexcept;
		void cpl() noexcept final;

		void xhtl(register16_t& exchange) noexcept;

		void blockCompare() noexcept;

		void cpi() noexcept;
		void cpir() noexcept;

		void cpd() noexcept;
		void cpdr() noexcept;

		void blockLoad() noexcept;

		void ldi() noexcept;
		void ldir() noexcept;

		void ldd() noexcept;
		void lddr() noexcept;

		void repeatBlockInstruction() noexcept;
		void adjustBlockRepeatFlagsIO() noexcept;
		void adjustBlockInputOutputFlags(int basis) noexcept;
		void adjustBlockInFlagsIncrement() noexcept;
		void adjustBlockInFlagsDecrement() noexcept;
		void adjustBlockOutFlags() noexcept;

		void blockIn() noexcept;

		void ini() noexcept;
		void inir() noexcept;

		void ind() noexcept;
		void indr() noexcept;

		void blockOut() noexcept;

		void outi() noexcept;
		void otir() noexcept;

		void outd() noexcept;
		void otdr() noexcept;

		void neg() noexcept;

		void rrd(register16_t address, uint8_t& update) noexcept;
		void rld(register16_t address, uint8_t& update) noexcept;

		void writePort(register16_t port) noexcept;
		void writePort(uint8_t port) noexcept;
		void writePort() noexcept;

		void readPort(register16_t port) noexcept;
		void readPort(uint8_t port) noexcept;
		void readPort() noexcept;
	};
}
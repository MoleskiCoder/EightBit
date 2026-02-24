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

	private:
		DEFINE_PIN_ACTIVATOR_LOW(RFSH)
		DEFINE_PIN_ACTIVATOR_LOW(M1)
		DEFINE_PIN_ACTIVATOR_LOW(MREQ)
		DEFINE_PIN_ACTIVATOR_LOW(IORQ)
		DEFINE_PIN_ACTIVATOR_LOW(RD)
		DEFINE_PIN_ACTIVATOR_LOW(WR)

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

		Z80(Bus& bus, InputOutput& ports);

		Z80(const Z80& rhs);
		bool operator==(const Z80& rhs) const;

		void execute() noexcept final;
		int step() noexcept final;

		[[nodiscard]] const register16_t& AF() const noexcept final;
		[[nodiscard]] auto& AF() noexcept { return IntelProcessor::AF(); }
		[[nodiscard]] const register16_t& BC() const noexcept final;
		[[nodiscard]] auto& BC() noexcept { return IntelProcessor::BC(); }
		[[nodiscard]] const register16_t& DE() const noexcept final;
		[[nodiscard]] auto& DE() noexcept { return IntelProcessor::DE(); }
		[[nodiscard]] const register16_t& HL() const noexcept final;
		[[nodiscard]] auto& HL() noexcept { return IntelProcessor::HL(); }

		[[nodiscard]] constexpr const auto& IX() const noexcept { return m_ix; }
		NON_CONST_REGISTOR_ACCESSOR(IX);
		[[nodiscard]] constexpr auto& IXH() noexcept { return IX().high; }
		[[nodiscard]] constexpr auto& IXL() noexcept { return IX().low; }

		[[nodiscard]] constexpr const auto& IY() const noexcept { return m_iy; }
		NON_CONST_REGISTOR_ACCESSOR(IY);
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
		[[nodiscard]] constexpr auto REFRESH() const noexcept { return m_refresh; }

		[[nodiscard]] constexpr auto& IV() noexcept { return iv; }
		[[nodiscard]] constexpr auto IV() const noexcept { return iv; }
		[[nodiscard]] constexpr auto& IM() noexcept { return m_interruptMode; }
		[[nodiscard]] constexpr auto IM() const noexcept { return m_interruptMode; }
		[[nodiscard]] constexpr auto& IFF1() noexcept { return m_iff1; }
		[[nodiscard]] constexpr auto IFF1() const noexcept { return m_iff1; }
		[[nodiscard]] constexpr auto& IFF2() noexcept { return m_iff2; }
		[[nodiscard]] constexpr auto IFF2() const noexcept { return m_iff2; }

		[[nodiscard]] constexpr auto& Q() noexcept { return m_q; }

		constexpr void exx() noexcept { m_registerSet ^= 1; }
		constexpr void exxAF() noexcept { m_accumulatorFlagsSet ^= 1; }

		[[nodiscard]] constexpr auto requestingIO() const noexcept { return lowered(IORQ()); }
		[[nodiscard]] constexpr auto requestingMemory() const noexcept { return lowered(MREQ()); }

		[[nodiscard]] constexpr auto requestingRead() const noexcept { return lowered(RD()); }
		[[nodiscard]] constexpr auto requestingWrite() const noexcept { return lowered(WR()); }

	protected:
		void handleRESET() noexcept final;
		void handleINT() noexcept final;

		void pushWord(register16_t destination) noexcept final;

		void memoryWrite() noexcept final;
		uint8_t memoryRead() noexcept final;

		void busWrite() noexcept final;
		uint8_t busRead() noexcept final;

		void jr(int8_t offset) noexcept final;
		int jrConditional(int condition) noexcept final;

	private:
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

		constexpr void resetPrefixes() noexcept {
			m_prefixCB = m_prefixDD = m_prefixED = m_prefixFD = false;
		}

		[[nodiscard]] constexpr auto displaced() const noexcept { return m_prefixDD || m_prefixFD; }

		[[nodiscard]] constexpr const register16_t& displacedAddress() noexcept {
			const auto& index_register = m_prefixDD ? IX() : IY();
			const auto address = index_register.word + m_displacement;
			MEMPTR().word = address;
			return MEMPTR();
		}

		void fetchDisplacement() noexcept;
		[[nodiscard]] uint8_t fetchOpCode() noexcept;

		uint8_t readBusDataM1() noexcept;

		typedef std::function<register16_t(void)> addresser_t;
		void loadAccumulatorIndirect(addresser_t addresser) noexcept;
		void storeAccumulatorIndirect(addresser_t addresser) noexcept;

		typedef std::function<uint8_t(void)> reader_t;
		void readInternalRegister(reader_t reader) noexcept;

		[[nodiscard]] register16_t& HL2() noexcept;
		[[nodiscard]] register16_t& RP(int rp) noexcept;
		[[nodiscard]] register16_t& RP2(int rp) noexcept;

		[[nodiscard]] uint8_t R(int r) noexcept;
		void R(int r, uint8_t value) noexcept;
		void R2(int r, uint8_t value) noexcept;

		static [[nodiscard]] constexpr auto zeroTest(uint8_t data) noexcept { return data & ZF; }
		static [[nodiscard]] constexpr auto carryTest(uint8_t data) noexcept { return data & CF; }
		static [[nodiscard]] constexpr auto parityTest(uint8_t data) noexcept { return data & PF; }
		static [[nodiscard]] constexpr auto signTest(uint8_t data) noexcept { return data & SF; }
		static [[nodiscard]] constexpr auto halfCarryTest(uint8_t data) noexcept { return data & HC; }
		static [[nodiscard]] constexpr auto subtractingTest(uint8_t data) noexcept { return data & NF; }
		static [[nodiscard]] constexpr auto xTest(uint8_t data) noexcept { return data & XF; }
		static [[nodiscard]] constexpr auto yTest(uint8_t data) noexcept { return data & YF; }

		[[nodiscard]] constexpr auto zero() noexcept { return zeroTest(F()); }
		[[nodiscard]] constexpr auto carry() noexcept { return carryTest(F()); }
		[[nodiscard]] constexpr auto parity() noexcept { return parityTest(F()); }
		[[nodiscard]] constexpr auto sign() noexcept { return signTest(F()); }
		[[nodiscard]] constexpr auto halfCarry() noexcept { return halfCarryTest(F()); }
		[[nodiscard]] constexpr auto subtracting() noexcept { return subtractingTest(F()); }

		void adjustStatusFlags(uint8_t value) noexcept { m_modifiedF = F() = value; }

		void setBit(StatusBits flag) { return adjustStatusFlags(setBit(F(), flag)); }
		static [[nodiscard]] constexpr uint8_t setBit(uint8_t f, StatusBits flag) noexcept { return Chip::setBit(f, (uint8_t)flag); }
		void setBit(StatusBits flag, int condition) noexcept { adjustStatusFlags(Chip::setBit(F(), flag, condition)); }
		static [[nodiscard]] constexpr uint8_t setBit(uint8_t f, StatusBits flag, int condition) noexcept { return Chip::setBit(f, (uint8_t)flag, condition); }
		void setBit(StatusBits flag, bool condition) noexcept { adjustStatusFlags(setBit(F(), flag, condition)); }
		static [[nodiscard]] constexpr uint8_t setBit(uint8_t f, StatusBits flag, bool condition) noexcept { return Chip::setBit(f, (uint8_t)flag, condition); }

		void clearBit(StatusBits flag) noexcept { adjustStatusFlags(clearBit(F(), flag)); }
		static [[nodiscard]] constexpr uint8_t clearBit(uint8_t f, StatusBits flag) noexcept { return Chip::clearBit(f, (uint8_t)flag); }
		void clearBit(StatusBits flag, int condition) noexcept { adjustStatusFlags(clearBit(F(), flag, condition)); }
		static [[nodiscard]] constexpr uint8_t clearBit(uint8_t f, StatusBits flag, int condition) noexcept { return Chip::clearBit(f, (uint8_t)flag, condition); }

		void adjustSign(uint8_t value) noexcept { adjustStatusFlags(adjustSign(F(), value)); }
		static [[nodiscard]] uint8_t adjustSign(uint8_t input, uint8_t value) noexcept { return setBit(input, SF, signTest(value)); }

		void adjustZero(uint8_t value) noexcept { adjustStatusFlags(adjustZero(F(), value)); }
		static [[nodiscard]] constexpr uint8_t adjustZero(uint8_t input, uint8_t value) noexcept { return clearBit(input, ZF, value); }

		void adjustParity(uint8_t value) noexcept { adjustStatusFlags(adjustParity(F(), value)); }
		static [[nodiscard]] constexpr uint8_t adjustParity(uint8_t input, uint8_t value) noexcept { return setBit(input, PF, evenParity(value)); }

		void adjustSZ(uint8_t value) noexcept { adjustStatusFlags(adjustSZ(F(), value)); }
		static [[nodiscard]] uint8_t adjustSZ(uint8_t input, uint8_t value) noexcept {
			input = adjustSign(input, value);
			return adjustZero(input, value);
		}

		void adjustSZP(uint8_t value) noexcept { adjustStatusFlags(adjustSZP(F(), value)); }
		static [[nodiscard]] uint8_t adjustSZP(uint8_t input, uint8_t value) noexcept {
			input = adjustSZ(input, value);
			return adjustParity(input, value);
		}

		void adjustXY(uint8_t value) noexcept { adjustStatusFlags(adjustXY(F(), value)); }
		static [[nodiscard]] constexpr uint8_t adjustXY(uint8_t input, uint8_t value) noexcept {
			input = setBit(input, XF, xTest(value));
			return setBit(input, YF, yTest(value));
		}

		void adjustSZPXY(uint8_t value) noexcept { adjustStatusFlags(adjustSZPXY(F(), value)); }
		static [[nodiscard]] uint8_t adjustSZPXY(uint8_t input, uint8_t value) noexcept {
			input = adjustSZP(input, value);
			return adjustXY(input, value);
		}

		void adjustSZXY(uint8_t value) noexcept { adjustStatusFlags(adjustSZXY(F(), value)); }
		static [[nodiscard]] uint8_t adjustSZXY(uint8_t input, uint8_t value) noexcept {
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

		void adjustOverflowAdd(int beforeNegative, int valueNegative, int afterNegative) {
			adjustStatusFlags(adjustOverflowAdd(F(), beforeNegative, valueNegative, afterNegative));
		}

		[[nodiscard]] static uint8_t adjustOverflowAdd(uint8_t input, int beforeNegative, int valueNegative, int afterNegative) {
			const auto overflow = beforeNegative == valueNegative && beforeNegative != afterNegative;
			return setBit(input, VF, overflow);
		}

		void adjustOverflowAdd(uint8_t before, uint8_t value, uint8_t calculation) {
			adjustStatusFlags(adjustOverflowAdd(F(), before, value, calculation));
		}

		[[nodiscard]] static uint8_t adjustOverflowAdd(uint8_t input, uint8_t before, uint8_t value, uint8_t calculation) {
			return adjustOverflowAdd(input, signTest(before), signTest(value), signTest(calculation));
		}

		void adjustOverflowSub(int beforeNegative, int valueNegative, int afterNegative) {
			adjustStatusFlags(adjustOverflowSub(F(), beforeNegative, valueNegative, afterNegative));
		}

		[[nodiscard]] static uint8_t adjustOverflowSub(uint8_t input, int beforeNegative, int valueNegative, int afterNegative) {
			const auto overflow = beforeNegative != valueNegative && beforeNegative != afterNegative;
			return setBit(input, VF, overflow);
		}

		void adjustOverflowSub(uint8_t before, uint8_t value, uint8_t calculation) {
			adjustStatusFlags(adjustOverflowSub(F(), before, value, calculation));
		}

		[[nodiscard]] static uint8_t adjustOverflowSub(uint8_t input, uint8_t before, uint8_t value, uint8_t calculation) {
			return adjustOverflowSub(input, signTest(before), signTest(value), signTest(calculation));
		}

		[[nodiscard]] constexpr bool convertCondition(int flag) noexcept {
			switch (flag) {
			case 0:
				return zero() == 0;
			case 1:
				return zero() != 0;
			case 2:
				return carry() == 0;
			case 3:
				return carry() != 0;
			case 4:
				return parity() == 0;
			case 5:
				return parity() != 0;
			case 6:
				return sign() == 0;
			case 7:
				return sign() != 0;
			default:
				UNREACHABLE;
			}
		}

		auto subtract(uint8_t operand, uint8_t value, int carry = 0) noexcept {

			const register16_t subtraction = operand - value - carry;
			const auto result = subtraction.low;

			adjustHalfCarrySub(operand, value, result);
			adjustOverflowSub(operand, value, result);

			setBit(NF);
			setBit(CF, carryTest(subtraction.high));
			adjustSZ(result);

			return result;
		}

		void executeCB(int x, int y, int z) noexcept;
		void executeED(int x, int y, int z, int p, int q) noexcept;
		void executeOther(int x, int y, int z, int p, int q) noexcept;

		[[nodiscard]] auto increment(uint8_t operand) noexcept {
			clearBit(NF);
			const uint8_t result = operand + 1;
			adjustSZXY(result);
			setBit(VF, result == Bit7);
			clearBit(HC, lowNibble(result));
			return result;
		}

		[[nodiscard]] auto decrement(uint8_t operand) noexcept {
			setBit(NF);
			clearBit(HC, lowNibble(operand));
			const uint8_t result = operand - 1;
			adjustSZXY(result);
			setBit(VF, result == Mask7);
			return result;
		}

		void disableInterrupts() override;
		void enableInterrupts() override;

		void retn() noexcept;
		void reti() noexcept;

		void returnConditionalFlag(int flag) noexcept;
		void jrConditionalFlag(int flag) noexcept;
		void callConditionalFlag(int flag) noexcept;
		void jumpConditionalFlag(int flag) noexcept;

		[[nodiscard]] register16_t sbc(register16_t operand, register16_t value) noexcept;
		[[nodiscard]] register16_t adc(register16_t operand, register16_t value) noexcept;
		[[nodiscard]] register16_t add(register16_t operand, register16_t value, int carry = 0) noexcept;

		[[nodiscard]] auto add(uint8_t operand, uint8_t value, int carry = 0) noexcept {

			intermediate() = operand + value + carry;
			const auto result = intermediate().low;

			adjustHalfCarryAdd(operand, value, result);
			adjustOverflowAdd(operand, value, result);

			clearBit(NF);
			setBit(CF, carryTest(intermediate().high));
			adjustSZXY(result);

			return result;
		}

		[[nodiscard]] auto adc(uint8_t operand, uint8_t value) noexcept {
			return add(operand, value, carry());
		}

		[[nodiscard]] auto sub(uint8_t operand, uint8_t value, int carry = 0) noexcept {
			const auto subtraction = subtract(operand, value, carry);
			adjustSZXY(subtraction);
			return subtraction;
		}

		[[nodiscard]] auto sbc(uint8_t operand, uint8_t value) noexcept {
			return sub(operand, value, carry());
		}

		[[nodiscard]] void andr(uint8_t value) noexcept {
			setBit(HC);
			clearBit((StatusBits)(CF | NF));
			adjustSZPXY(A() &= value);
		}

		[[nodiscard]] void xorr(uint8_t value) noexcept {
			clearBit((StatusBits)(HC | CF | NF));
			adjustSZPXY(A() ^= value);
		}

		[[nodiscard]] void orr(uint8_t value) noexcept {
			clearBit((StatusBits)(HC | CF | NF));
			adjustSZPXY(A() |= value);
		}

		void compare(uint8_t value) noexcept {
			subtract(A(), value);
			adjustXY(value);
		}

		[[nodiscard]] auto rlc(uint8_t operand) noexcept {
			clearBit((StatusBits)(NF | HC));
			const auto carry = operand & Bit7;
			setBit(CF, carry);
			const uint8_t result = (operand << 1) | (carry >> 7);
			adjustXY(result);
			return result;
		}

		[[nodiscard]] auto rrc(uint8_t operand) noexcept {
			clearBit((StatusBits)(NF | HC));
			const auto carry = operand & Bit0;
			setBit(CF, carry);
			const uint8_t result = (operand >> 1) | (carry << 7);
			adjustXY(result);
			return result;
		}

		[[nodiscard]] auto rl(uint8_t operand) noexcept {
			clearBit((StatusBits)(NF | HC));
			const auto carrying = carry();
			setBit(CF, operand & Bit7);
			const uint8_t result = (operand << 1) | carrying;
			adjustXY(result);
			return result;
		}

		[[nodiscard]] auto rr(uint8_t operand) noexcept {
			clearBit((StatusBits)(NF | HC));
			const auto carrying = carry();
			setBit(CF, operand & Bit0);
			const uint8_t result = (operand >> 1) | (carrying << 7);
			adjustXY(result);
			return result;
		}

		[[nodiscard]] auto sla(uint8_t operand) noexcept {
			clearBit((StatusBits)(NF | HC));
			setBit(CF, operand & Bit7);
			const uint8_t result = operand << 1;
			adjustXY(result);
			return result;
		}

		[[nodiscard]] auto sra(uint8_t operand) noexcept {
			clearBit((StatusBits)(NF | HC));
			setBit(CF, operand & Bit0);
			const uint8_t result = (operand >> 1) | (operand & Bit7);
			adjustXY(result);
			return result;
		}

		[[nodiscard]] auto sll(uint8_t operand) noexcept {
			clearBit((StatusBits)(NF | HC));
			setBit(CF, operand & Bit7);
			const uint8_t result = (operand << 1) | Bit0;
			adjustXY(result);
			return result;
		}

		[[nodiscard]] auto srl(uint8_t operand) noexcept {
			clearBit((StatusBits)(NF | HC));
			setBit(CF, operand & Bit0);
			const uint8_t result = (operand >> 1) & ~Bit7;
			adjustXY(result);
			setBit(ZF, result);
			return result;
		}

		void bit(int n, uint8_t operand) noexcept {
			setBit(HC);
			clearBit(NF);
			const auto discarded = operand & Chip::bit(n);
			adjustSZ(discarded);
			clearBit(PF, discarded);
		}

		[[nodiscard]] static constexpr auto res(int n, uint8_t operand) noexcept { return Chip::clearBit(operand, Chip::bit(n)); }
		[[nodiscard]] static constexpr auto set(int n, uint8_t operand) noexcept { return Chip::setBit(operand, Chip::bit(n)); }

		[[nodiscard]] void daa() noexcept {

			auto updated = A();

			const auto lowAdjust = halfCarry() || (lowNibble(A()) > 9);
			const auto highAdjust = carry() || (A() > 0x99);

			if (subtracting()) {
				if (lowAdjust)
					updated -= 6;
				if (highAdjust)
					updated -= 0x60;
			} else {
				if (lowAdjust)
					updated += 6;
				if (highAdjust)
					updated += 0x60;
			}

			adjustStatusFlags((F() & (CF | NF) | (A() > 0x99 ? CF : 0) | halfCarryTest(A() ^ updated)));
			adjustSZPXY(A() = updated);
		}

		void scf(uint8_t operand) noexcept {
			setBit(CF);
			clearBit((StatusBits)(HC | NF));
			adjustXY((Q() ^ F()) | A());
		}

		void ccf(uint8_t operand) noexcept {
			clearBit(NF);
			const auto carrying = carry();
			setBit(HC, carrying);
			clearBit(CF, carrying);
			adjustXY(((Q() ^ F()) | A()));
		}

		void cpl() noexcept {
			setBit((StatusBits)(HC | NF));
			adjustXY(A() = ~A());
		}

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

		void repeatBlockInstruction();
		void adjustBlockRepeatFlagsIO();
		void adjustBlockInputOutputFlags(int basis);
		void adjustBlockInFlagsIncrement();
		void adjustBlockInFlagsDecrement();
		void adjustBlockOutFlags();

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

		void neg() noexcept {

			setBit(PF, A() == Bit7);
			setBit(CF, A());
			setBit(NF);

			auto original = A();

			A() = (~A() + 1);	// two's complement

			adjustHalfCarrySub(0U, original, A());
			adjustOverflowSub(0U, original, A());

			adjustSZXY(A());
		}

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
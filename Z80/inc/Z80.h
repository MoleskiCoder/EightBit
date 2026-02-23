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

		Signal<Z80> ExecutingInstruction;
		Signal<Z80> ExecutedInstruction;

		Signal<EventArgs> ReadingMemory;
		Signal<EventArgs> ReadMemory;

		Signal<EventArgs> WritingMemory;
		Signal<EventArgs> WrittenMemory;

		Signal<EventArgs> ReadingIO;
		Signal<EventArgs> ReadIO;

		Signal<EventArgs> WritingIO;
		Signal<EventArgs> WrittenIO;

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

		[[nodiscard]] static constexpr auto adjustHalfCarryAdd(uint8_t f, uint8_t before, uint8_t value, int calculation) noexcept {
			return setBit(f, HC, calculateHalfCarryAdd(before, value, calculation));
		}

		[[nodiscard]] static constexpr auto adjustHalfCarrySub(uint8_t f, uint8_t before, uint8_t value, int calculation) noexcept {
			return setBit(f, HC, calculateHalfCarrySub(before, value, calculation));
		}

		[[nodiscard]] static constexpr auto adjustOverflowAdd(uint8_t f, int beforeNegative, int valueNegative, int afterNegative) noexcept {
			const auto overflow = (beforeNegative == valueNegative) && (beforeNegative != afterNegative);
			return setBit(f, VF, overflow);
		}

		[[nodiscard]] static constexpr auto adjustOverflowAdd(uint8_t f, uint8_t before, uint8_t value, uint8_t calculation) noexcept {
			return adjustOverflowAdd(f, before & SF, value & SF, calculation & SF);
		}

		[[nodiscard]] static constexpr auto adjustOverflowSub(uint8_t f, int beforeNegative, int valueNegative, int afterNegative) noexcept {
			const auto overflow = (beforeNegative != valueNegative) && (beforeNegative != afterNegative);
			return setBit(f, VF, overflow);
		}

		[[nodiscard]] static constexpr auto adjustOverflowSub(uint8_t f, uint8_t before, uint8_t value, uint8_t calculation) noexcept {
			return adjustOverflowSub(f, before & SF, value & SF, calculation & SF);
		}

		[[nodiscard]] static constexpr bool convertCondition(uint8_t f, int flag) noexcept {
			switch (flag) {
			case 0:
				return !(f & ZF);
			case 1:
				return f & ZF;
			case 2:
				return !(f & CF);
			case 3:
				return f & CF;
			case 4:
				return !(f & PF);
			case 5:
				return f & PF;
			case 6:
				return !(f & SF);
			case 7:
				return f & SF;
			default:
				UNREACHABLE;
			}
		}

		static constexpr auto subtract(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0) noexcept {
			const register16_t subtraction = operand - value - carry;
			const auto result = subtraction.low;

			f = adjustHalfCarrySub(f, operand, value, result);
			f = adjustOverflowSub(f, operand, value, result);

			f = setBit(f, NF);
			f = setBit(f, CF, subtraction.high & CF);
			f = adjustSZ<Z80>(f, result);

			return result;
		}

		void executeCB(int x, int y, int z) noexcept;
		void executeED(int x, int y, int z, int p, int q) noexcept;
		void executeOther(int x, int y, int z, int p, int q) noexcept;

		[[nodiscard]] static constexpr auto increment(uint8_t& f, uint8_t operand) noexcept {
			f = clearBit(f, NF);
			const uint8_t result = operand + 1;
			f = adjustSZXY<Z80>(f, result);
			f = setBit(f, VF, result == Bit7);
			f = clearBit(f, HC, lowNibble(result));
			return result;
		}

		[[nodiscard]] static constexpr auto decrement(uint8_t& f, uint8_t operand) noexcept {
			f = setBit(f, NF);
			f = clearBit(f, HC, lowNibble(operand));
			const uint8_t result = operand - 1;
			f = adjustSZXY<Z80>(f, result);
			f = setBit(f, VF, result == Mask7);
			return result;
		}

		void disableInterrupts() override;
		void enableInterrupts() override;

		void retn() noexcept;
		void reti() noexcept;

		void returnConditionalFlag(uint8_t f, int flag) noexcept;
		void jrConditionalFlag(uint8_t f, int flag) noexcept;
		void callConditionalFlag(uint8_t f, int flag) noexcept;
		void jumpConditionalFlag(uint8_t f, int flag) noexcept;

		[[nodiscard]] register16_t sbc(uint8_t& f, register16_t operand, register16_t value) noexcept;
		[[nodiscard]] register16_t adc(uint8_t& f, register16_t operand, register16_t value) noexcept;
		[[nodiscard]] register16_t add(uint8_t& f, register16_t operand, register16_t value, int carry = 0) noexcept;

		[[nodiscard]] static constexpr auto add(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0) noexcept {
			const register16_t addition = operand + value + carry;
			const auto result = addition.low;

			f = adjustHalfCarryAdd(f, operand, value, result);
			f = adjustOverflowAdd(f, operand, value, result);

			f = clearBit(f, NF);
			f = setBit(f, CF, addition.high & CF);
			f = adjustSZXY<Z80>(f, result);

			return result;
		}

		[[nodiscard]] static constexpr auto adc(uint8_t& f, uint8_t operand, uint8_t value) noexcept {
			return add(f, operand, value, f & CF);
		}

		[[nodiscard]] static constexpr auto sub(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0) noexcept {
			const auto subtraction = subtract(f, operand, value, carry);
			f = adjustSZXY<Z80>(f, subtraction);
			return subtraction;
		}

		[[nodiscard]] static constexpr auto sbc(uint8_t& f, uint8_t operand, uint8_t value) noexcept {
			return sub(f, operand, value, f & CF);
		}

		[[nodiscard]] static constexpr auto andr(uint8_t& f, uint8_t operand, uint8_t value) noexcept {
			f = setBit(f, HC);
			f = clearBit(f, CF | NF);
			const uint8_t result = operand & value;
			f = adjustSZPXY<Z80>(f, result);
			return result;
		}

		[[nodiscard]] static constexpr auto xorr(uint8_t& f, uint8_t operand, uint8_t value) noexcept {
			f = clearBit(f, HC | CF | NF);
			const uint8_t result = operand ^ value;
			f = adjustSZPXY<Z80>(f, result);
			return result;
		}

		[[nodiscard]] static constexpr auto orr(uint8_t& f, uint8_t operand, uint8_t value) noexcept {
			f = clearBit(f, HC | CF | NF);
			const uint8_t result = operand | value;
			f = adjustSZPXY<Z80>(f, result);
			return result;
		}

		static void compare(uint8_t& f, uint8_t operand, uint8_t value) noexcept {
			subtract(f, operand, value);
			f = adjustXY<Z80>(f, value);
		}

		[[nodiscard]] static constexpr auto rlc(uint8_t& f, uint8_t operand) noexcept {
			f = clearBit(f, NF | HC);
			const auto carry = operand & Bit7;
			f = setBit(f, CF, carry);
			const uint8_t result = (operand << 1) | (carry >> 7);
			f = adjustXY<Z80>(f, result);
			return result;
		}

		[[nodiscard]] static constexpr auto rrc(uint8_t& f, uint8_t operand) noexcept {
			f = clearBit(f, NF | HC);
			const auto carry = operand & Bit0;
			f = setBit(f, CF, carry);
			const uint8_t result = (operand >> 1) | (carry << 7);
			f = adjustXY<Z80>(f, result);
			return result;
		}

		[[nodiscard]] static constexpr auto rl(uint8_t& f, uint8_t operand) noexcept {
			f = clearBit(f, NF | HC);
			const auto carry = f & CF;
			f = setBit(f, CF, operand & Bit7);
			const uint8_t result = (operand << 1) | carry;
			f = adjustXY<Z80>(f, result);
			return result;
		}

		[[nodiscard]] static constexpr auto rr(uint8_t& f, uint8_t operand) noexcept {
			f = clearBit(f, NF | HC);
			const auto carry = f & CF;
			f = setBit(f, CF, operand & Bit0);
			const uint8_t result = (operand >> 1) | (carry << 7);
			f = adjustXY<Z80>(f, result);
			return result;
		}

		[[nodiscard]] static constexpr auto sla(uint8_t& f, uint8_t operand) noexcept {
			f = clearBit(f, NF | HC);
			f = setBit(f, CF, operand & Bit7);
			const uint8_t result = operand << 1;
			f = adjustXY<Z80>(f, result);
			return result;
		}

		[[nodiscard]] static constexpr auto sra(uint8_t& f, uint8_t operand) noexcept {
			f = clearBit(f, NF | HC);
			f = setBit(f, CF, operand & Bit0);
			const uint8_t result = (operand >> 1) | (operand & Bit7);
			f = adjustXY<Z80>(f, result);
			return result;
		}

		[[nodiscard]] static constexpr auto sll(uint8_t& f, uint8_t operand) noexcept {
			f = clearBit(f, NF | HC);
			f = setBit(f, CF, operand & Bit7);
			const uint8_t result = (operand << 1) | Bit0;
			f = adjustXY<Z80>(f, result);
			return result;
		}

		[[nodiscard]] static constexpr auto srl(uint8_t& f, uint8_t operand) noexcept {
			f = clearBit(f, NF | HC);
			f = setBit(f, CF, operand & Bit0);
			const uint8_t result = (operand >> 1) & ~Bit7;
			f = adjustXY<Z80>(f, result);
			f = setBit(f, ZF, result);
			return result;
		}

		static constexpr void bit(uint8_t& f, int n, uint8_t operand) noexcept {
			f = setBit(f, HC);
			f = clearBit(f, NF);
			const auto discarded = operand & Chip::bit(n);
			f = adjustSZ<Z80>(f, discarded);
			f = clearBit(f, PF, discarded);
		}

		[[nodiscard]] static constexpr auto res(int n, uint8_t operand) noexcept { return clearBit(operand, Chip::bit(n)); }
		[[nodiscard]] static constexpr auto set(int n, uint8_t operand) noexcept { return setBit(operand, Chip::bit(n)); }

		[[nodiscard]] static constexpr auto daa(uint8_t& f, uint8_t operand) noexcept {
			const auto lowAdjust = (f & HC) || (lowNibble(operand) > 9);
			const auto highAdjust = (f & CF) || (operand > 0x99);

			auto updated = operand;
			if (f & NF) {
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

			f = (f & (CF | NF)) | (operand > 0x99 ? CF : 0) | ((operand ^ updated) & HC);

			f = adjustSZPXY<Z80>(f, updated);

			return updated;
		}

		static constexpr void scf(uint8_t& f, uint8_t operand) noexcept {
			f = setBit(f, CF);
			f = clearBit(f, HC | NF);
			f = adjustXY<Z80>(f, operand);
		}

		static constexpr void ccf(uint8_t& f, uint8_t operand) noexcept {
			f = clearBit(f, NF);
			const auto carry = f & CF;
			f = setBit(f, HC, carry);
			f = clearBit(f, CF, carry);
			f = adjustXY<Z80>(f, operand);
		}

		[[nodiscard]] static constexpr auto cpl(uint8_t& f, uint8_t operand) noexcept {
			f = setBit(f, HC | NF);
			const uint8_t result = ~operand;
			f = adjustXY<Z80>(f, result);
			return result;
		}

		void xhtl(register16_t& exchange) noexcept;

		void blockCompare(uint8_t& f, uint8_t value, register16_t source, register16_t& counter) noexcept;

		void cpi(uint8_t& f, uint8_t value) noexcept;
		[[nodiscard]] bool cpir(uint8_t& f, uint8_t value) noexcept;

		void cpd(uint8_t& f, uint8_t value) noexcept;
		[[nodiscard]] bool cpdr(uint8_t& f, uint8_t value) noexcept;

		void blockLoad(uint8_t& f, uint8_t a, register16_t source, register16_t destination, register16_t& counter) noexcept;

		void ldi(uint8_t& f, uint8_t a) noexcept;
		[[nodiscard]] bool ldir(uint8_t& f, uint8_t a) noexcept;

		void ldd(uint8_t& f, uint8_t a) noexcept;
		[[nodiscard]] bool lddr(uint8_t& f, uint8_t a) noexcept;

		void blockIn(register16_t& source, register16_t destination) noexcept;

		void ini() noexcept;
		[[nodiscard]] bool inir() noexcept;

		void ind() noexcept;
		[[nodiscard]] bool indr() noexcept;

		void blockOut(register16_t source, register16_t& destination) noexcept;

		void outi() noexcept;
		[[nodiscard]] bool otir() noexcept;

		void outd() noexcept;
		[[nodiscard]] bool otdr() noexcept;

		[[nodiscard]] static constexpr auto neg(uint8_t& f, uint8_t operand) noexcept {
			f = setBit(f, PF, operand == Bit7);
			f = setBit(f, CF, operand);
			f = setBit(f, NF);

			const uint8_t result = (~operand + 1);	// two's complement

			f = adjustHalfCarrySub(f, 0U, operand, result);
			f = adjustOverflowSub(f, 0U, operand, result);

			f = adjustSZXY<Z80>(f, result);

			return result;
		}

		void rrd(uint8_t& f, register16_t address, uint8_t& update) noexcept;
		void rld(uint8_t& f, register16_t address, uint8_t& update) noexcept;

		void portWrite(uint8_t port) noexcept;
		void portWrite() noexcept;

		void portRead(uint8_t port) noexcept;
		void portRead() noexcept;
	};
}
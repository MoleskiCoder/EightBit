#pragma once

#include <cstdint>
#include <cassert>
#include <stdexcept>

#include <Bus.h>
#include <InputOutput.h>
#include <IntelProcessor.h>
#include <EventArgs.h>
#include <Signal.h>
#include <Register.h>
#include <EightBitCompilerDefinitions.h>

namespace EightBit {
	class Z80 final : public IntelProcessor {
	public:
		struct refresh_t {

			bool high : 1;
			uint8_t variable : 7;

			refresh_t(const uint8_t value)
			: high(!!(value & Bit7)),
			  variable(value & Mask7)
			{ }

			operator uint8_t() const {
				return (high << 7) | variable;
			}

			auto& operator++() {
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

		Signal<Z80> ExecutingInstruction;
		Signal<Z80> ExecutedInstruction;

		int execute(uint8_t opcode) { return IntelProcessor::execute(opcode); }
		int execute() final;
		int step() final;

		[[nodiscard]] register16_t& AF() final;
		[[nodiscard]] register16_t& BC() final;
		[[nodiscard]] register16_t& DE() final;
		[[nodiscard]] register16_t& HL() final;

		[[nodiscard]] auto& IX() { return m_ix; }
		[[nodiscard]] auto& IXH() { return IX().high; }
		[[nodiscard]] auto& IXL() { return IX().low; }

		[[nodiscard]] auto& IY() { return m_iy; }
		[[nodiscard]] auto& IYH() { return IY().high; }
		[[nodiscard]] auto& IYL() { return IY().low; }

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
		[[nodiscard]] auto& REFRESH() { return m_refresh; }

		[[nodiscard]] auto& IV() { return iv; }
		[[nodiscard]] auto& IM() { return m_interruptMode; }
		[[nodiscard]] auto& IFF1() { return m_iff1; }
		[[nodiscard]] auto& IFF2() { return m_iff2; }

		void exx() {
			m_registerSet ^= 1;
		}

		void exxAF() {
			m_accumulatorFlagsSet ^= 1;
		}

		DECLARE_PIN_INPUT(NMI)
		DECLARE_PIN_OUTPUT(M1)

		// ** From the Z80 CPU User Manual
		// RFSH.Refresh(output, active Low). RFSH, together with MREQ, indicates that the lower
		// seven bits of the system’s address bus can be used as a refresh address to the system’s
		// dynamic memories.
		DECLARE_PIN_OUTPUT(RFSH)

		DECLARE_PIN_OUTPUT(MREQ)
		DECLARE_PIN_OUTPUT(IORQ)
		DECLARE_PIN_OUTPUT(RD)
		DECLARE_PIN_OUTPUT(WR)

	protected:
		void handleRESET() final;
		void handleINT() final;

		void busWrite() final;
		uint8_t busRead() final;

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
		bool m_displaced = false;

		void handleNMI();

		[[nodiscard]] uint16_t displacedAddress() {
			assert(m_displaced);
			return MEMPTR().word = (m_prefixDD ? IX() : IY()).word + m_displacement;
		}

		void fetchDisplacement() {
			m_displacement = fetchByte();
		}

		// ** From the Z80 CPU User Manual
		// Figure 5 depicts the timing during an M1 (op code fetch) cycle. The Program Counter is
		// placed on the address bus at the beginning of the M1 cycle. One half clock cycle later, the
		// MREQ signal goes active. At this time, the address to memory has had time to stabilize so
		// that the falling edge of MREQ can be used directly as a chip enable clock to dynamic
		// memories. The RD line also goes active to indicate that the memory read data should be
		// enabled onto the CPU data bus. The CPU samples the data from the memory space on the
		// data bus with the rising edge of the clock of state T3, and this same edge is used by the
		// CPU to turn off the RD and MREQ signals. As a result, the data is sampled by the CPU
		// before the RD signal becomes inactive. Clock states T3 and T4 of a fetch cycle are used to
		// refresh dynamic memories. The CPU uses this time to decode and execute the fetched
		// instruction so that no other concurrent operation can be performed.
		uint8_t readInitialOpCode() {
			lowerM1();
			const auto returned = IntelProcessor::busRead(PC());
			raiseM1();
			tick(2);
			BUS().ADDRESS().low = REFRESH();
			BUS().ADDRESS().high = IV();
			lowerRFSH();
			lowerMREQ();
			tick();
			raiseMREQ();
			raiseRFSH();
			tick();
			return returned;
		}

		uint8_t fetchInitialOpCode() {
			const auto returned = readInitialOpCode();
			++PC();
			return returned;
		}

		[[nodiscard]] auto& HL2() {
			if (LIKELY(!m_displaced))
				return HL();
			if (m_prefixDD)
				return IX();
			// Must be FD prefix
			return IY();
		}

		[[nodiscard]] auto& RP(const int rp) {
			ASSUME(rp >= 0);
			ASSUME(rp <= 3);
			switch (rp) {
			case 0:
				return BC();
			case 1:
				return DE();
			case 2:
				return HL2();
			case 3:
				return SP();
			default:
				UNREACHABLE;
			}
		}

		[[nodiscard]] auto& RP2(const int rp) {
			ASSUME(rp >= 0);
			ASSUME(rp <= 3);
			switch (rp) {
			case 0:
				return BC();
			case 1:
				return DE();
			case 2:
				return HL2();
			case 3:
				return AF();
			default:
				UNREACHABLE;
			}
		}

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
				return HL2().high;
			case 5:
				return HL2().low;
			case 6:
				return IntelProcessor::busRead(UNLIKELY(m_displaced) ?  displacedAddress() : HL().word);
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
				HL2().high = value;
				break;
			case 5:
				HL2().low = value;
				break;
			case 6:
				IntelProcessor::busWrite(UNLIKELY(m_displaced) ? displacedAddress() : HL().word, value);
				break;
			case 7:
				A() = value;
				break;
			default:
				UNREACHABLE;
			}
		}

		void R2(const int r, const uint8_t value) {
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
				IntelProcessor::busWrite(HL(), value);
				break;
			case 7:
				A() = value;
				break;
			default:
				UNREACHABLE;
			}
		}

		[[nodiscard]] static uint8_t adjustHalfCarryAdd(uint8_t f, const uint8_t before, const uint8_t value, const int calculation) {
			return setBit(f, HC, calculateHalfCarryAdd(before, value, calculation));
		}

		[[nodiscard]] static uint8_t adjustHalfCarrySub(uint8_t f, const uint8_t before, const uint8_t value, const int calculation) {
			return setBit(f, HC, calculateHalfCarrySub(before, value, calculation));
		}

		[[nodiscard]] static uint8_t adjustOverflowAdd(uint8_t f, const uint8_t before, const uint8_t value, const uint8_t calculation) {
			return adjustOverflowAdd(f, before & SF, value & SF, calculation & SF);
		}

		[[nodiscard]] static uint8_t adjustOverflowAdd(uint8_t f, const int beforeNegative, const int valueNegative, const int afterNegative) {
			const auto overflow = (beforeNegative == valueNegative) && (beforeNegative != afterNegative);
			return setBit(f, VF, overflow);
		}

		[[nodiscard]] static uint8_t adjustOverflowSub(uint8_t f, const uint8_t before, const uint8_t value, const uint8_t calculation) {
			return adjustOverflowSub(f, before & SF, value & SF, calculation & SF);
		}

		[[nodiscard]] static uint8_t adjustOverflowSub(uint8_t f, const int beforeNegative, const int valueNegative, const int afterNegative) {
			const auto overflow = (beforeNegative != valueNegative) && (beforeNegative != afterNegative);
			return setBit(f, VF, overflow);
		}

		static uint8_t subtract(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0);

		void executeCB(int x, int y, int z);
		void executeED(int x, int y, int z, int p, int q);
		void executeOther(int x, int y, int z, int p, int q);

		[[nodiscard]] static uint8_t increment(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t decrement(uint8_t& f, uint8_t operand);

		void di();
		void ei();

		void retn();
		void reti();

		[[nodiscard]] bool jrConditionalFlag(uint8_t f, int flag);
		[[nodiscard]] bool returnConditionalFlag(uint8_t f, int flag);
		[[nodiscard]] bool callConditionalFlag(uint8_t f, int flag);
		void jumpConditionalFlag(uint8_t f, int flag);

		[[nodiscard]] register16_t sbc(uint8_t& f, register16_t operand, register16_t value);
		[[nodiscard]] register16_t adc(uint8_t& f, register16_t operand, register16_t value);
		[[nodiscard]] register16_t add(uint8_t& f, register16_t operand, register16_t value, int carry = 0);

		[[nodiscard]] static uint8_t add(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0);
		[[nodiscard]] static uint8_t adc(uint8_t& f, uint8_t operand, uint8_t value);
		[[nodiscard]] static uint8_t sub(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0);
		[[nodiscard]] static uint8_t sbc(uint8_t& f, uint8_t operand, uint8_t value);
		[[nodiscard]] static uint8_t andr(uint8_t& f, uint8_t operand, uint8_t value);
		[[nodiscard]] static uint8_t xorr(uint8_t& f, uint8_t operand, uint8_t value);
		[[nodiscard]] static uint8_t orr(uint8_t& f, uint8_t operand, uint8_t value);
		static void compare(uint8_t& f, uint8_t operand, uint8_t value);

		[[nodiscard]] static uint8_t rlc(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t rrc(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t rl(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t rr(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t sla(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t sra(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t sll(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t srl(uint8_t& f, uint8_t operand);

		static void bit(uint8_t& f, int n, uint8_t operand);
		[[nodiscard]] static uint8_t res(int n, uint8_t operand);
		[[nodiscard]] static uint8_t set(int n, uint8_t operand);

		[[nodiscard]] static uint8_t daa(uint8_t& f, uint8_t operand);

		static void scf(uint8_t& f, uint8_t operand);
		static void ccf(uint8_t& f, uint8_t operand);
		static uint8_t cpl(uint8_t& f, uint8_t operand);

		void xhtl(register16_t& exchange);

		void blockCompare(uint8_t& f, uint8_t value, register16_t source, register16_t& counter);

		void cpi(uint8_t& f, uint8_t value);
		bool cpir(uint8_t& f, uint8_t value);

		void cpd(uint8_t& f, uint8_t value);
		bool cpdr(uint8_t& f, uint8_t value);

		void blockLoad(uint8_t& f, uint8_t a, register16_t source, register16_t destination, register16_t& counter);

		void ldi(uint8_t& f, uint8_t a);
		bool ldir(uint8_t& f, uint8_t a);

		void ldd(uint8_t& f, uint8_t a);
		bool lddr(uint8_t& f, uint8_t a);

		void blockIn(register16_t& source, register16_t destination);

		void ini();
		bool inir();

		void ind();
		bool indr();

		void blockOut(register16_t source, register16_t& destination);

		void outi();
		bool otir();

		void outd();
		bool otdr();

		[[nodiscard]] uint8_t neg(uint8_t& f, uint8_t operand);

		void rrd(uint8_t& f, register16_t address, uint8_t& update);
		void rld(uint8_t& f, register16_t address, uint8_t& update);

		void writePort(uint8_t port);
		void writePort();

		uint8_t readPort(uint8_t port);
		uint8_t readPort();
	};
}
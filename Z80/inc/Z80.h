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

		[[nodiscard]] auto& REFRESH() { return m_refresh; }
		[[nodiscard]] auto& IV() { return iv; }
		[[nodiscard]] auto& IM() { return m_interruptMode; }
		[[nodiscard]] auto& IFF1() { return m_iff1; }
		[[nodiscard]] auto& IFF2() { return m_iff2; }

		void exx() {
			m_registerSet ^= 1;
		}

		void exxAF() {
			m_accumulatorFlagsSet = !m_accumulatorFlagsSet;
		}

		DECLARE_PIN_INPUT(NMI)
		DECLARE_PIN_OUTPUT(M1)
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

		static void adjustHalfCarryAdd(uint8_t& f, const uint8_t before, const uint8_t value, const int calculation) {
			setFlag(f, HC, calculateHalfCarryAdd(before, value, calculation));
		}

		static void adjustHalfCarrySub(uint8_t& f, const uint8_t before, const uint8_t value, const int calculation) {
			setFlag(f, HC, calculateHalfCarrySub(before, value, calculation));
		}

		static void adjustOverflowAdd(uint8_t& f, const uint8_t before, const uint8_t value, const uint8_t calculation) {
			adjustOverflowAdd(f, before & SF, value & SF, calculation & SF);
		}

		static void adjustOverflowAdd(uint8_t& f, const int beforeNegative, const int valueNegative, const int afterNegative) {
			const auto overflow = (beforeNegative == valueNegative) && (beforeNegative != afterNegative);
			setFlag(f, VF, overflow);
		}

		static void adjustOverflowSub(uint8_t& f, const uint8_t before, const uint8_t value, const uint8_t calculation) {
			adjustOverflowSub(f, before & SF, value & SF, calculation & SF);
		}

		static void adjustOverflowSub(uint8_t& f, const int beforeNegative, const int valueNegative, const int afterNegative) {
			const auto overflow = (beforeNegative != valueNegative) && (beforeNegative != afterNegative);
			setFlag(f, VF, overflow);
		}

		uint8_t subtract(uint8_t operand, uint8_t value, int carry = 0);

		void executeCB(int x, int y, int z);
		void executeED(int x, int y, int z, int p, int q);
		void executeOther(int x, int y, int z, int p, int q);

		[[nodiscard]] uint8_t increment(uint8_t operand);
		[[nodiscard]] uint8_t decrement(uint8_t operand);

		void di();
		void ei();

		void retn();
		void reti();

		bool jrConditionalFlag(int flag);
		bool returnConditionalFlag(int flag);
		bool jumpConditionalFlag(int flag);
		bool callConditionalFlag(int flag);

		register16_t sbc(register16_t operand, register16_t value);
		register16_t adc(register16_t operand, register16_t value);
		register16_t add(register16_t operand, register16_t value, int carry = 0);

		[[nodiscard]] uint8_t add(uint8_t operand, uint8_t value, int carry = 0);
		[[nodiscard]] uint8_t adc(uint8_t operand, uint8_t value);
		[[nodiscard]] uint8_t sub(uint8_t operand, uint8_t value, int carry = 0);
		[[nodiscard]] uint8_t sbc(uint8_t operand, uint8_t value);
		void andr(uint8_t value);
		void xorr(uint8_t value);
		void orr(uint8_t value);
		void compare(uint8_t value);

		[[nodiscard]] uint8_t rlc(uint8_t operand);
		[[nodiscard]] uint8_t rrc(uint8_t operand);
		[[nodiscard]] uint8_t rl(uint8_t operand);
		[[nodiscard]] uint8_t rr(uint8_t operand);
		[[nodiscard]] uint8_t sla(uint8_t operand);
		[[nodiscard]] uint8_t sra(uint8_t operand);
		[[nodiscard]] uint8_t sll(uint8_t operand);
		[[nodiscard]] uint8_t srl(uint8_t operand);

		void bit(int n, uint8_t operand);
		[[nodiscard]] static uint8_t res(int n, uint8_t operand);
		[[nodiscard]] static uint8_t set(int n, uint8_t operand);

		void daa();

		void scf();
		void ccf();
		void cpl();

		void xhtl(register16_t& exchange);

		void blockCompare(register16_t source, register16_t& counter);

		void cpi();
		bool cpir();

		void cpd();
		bool cpdr();

		void blockLoad(register16_t source, register16_t destination, register16_t& counter);

		void ldi();
		bool ldir();

		void ldd();
		bool lddr();

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

		void neg();

		void rrd();
		void rld();

		void writePort(uint8_t port);
		void writePort();

		uint8_t readPort(uint8_t port);
		uint8_t readPort();
	};
}
#pragma once

#include <cstdint>
#include <cassert>
#include <stdexcept>

#include <Bus.h>
#include <InputOutput.h>
#include <IntelProcessor.h>
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
			: high((value & Bit7) != 0),
			  variable(value & Mask7)
			{ }

			operator uint8_t() const {
				return (high << 7) | variable;
			}

			refresh_t& operator++() {
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

		PinLevel& M1() { return m_m1Line; }			// Out

		virtual int execute(uint8_t opcode) final;
		virtual int step() final;

		virtual register16_t& AF() final;
		virtual register16_t& BC() final;
		virtual register16_t& DE() final;
		virtual register16_t& HL() final;

		register16_t& IX() { return m_ix; }
		uint8_t& IXH() { return IX().high; }
		uint8_t& IXL() { return IX().low; }

		register16_t& IY() { return m_iy; }
		uint8_t& IYH() { return IY().high; }
		uint8_t& IYL() { return IY().low; }

		refresh_t& REFRESH() { return m_refresh; }
		uint8_t& IV() { return iv; }
		int& IM() { return m_interruptMode; }
		bool& IFF1() { return m_iff1; }
		bool& IFF2() { return m_iff2; }

		void exx() {
			m_registerSet ^= 1;
		}

		void exxAF() {
			m_accumulatorFlagsSet = !m_accumulatorFlagsSet;
		}

	protected:
		virtual void reset() final;

	private:
		PinLevel m_m1Line = Low;

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

		uint16_t displacedAddress() {
			assert(m_displaced);
			return MEMPTR().word = (m_prefixDD ? IX() : IY()).word + m_displacement;
		}

		void fetchDisplacement() {
			m_displacement = fetchByte();
		}

		uint8_t R(const int r) {
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
				return BUS().read(UNLIKELY(m_displaced) ?  displacedAddress() : HL().word);
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
				BUS().write(UNLIKELY(m_displaced) ? displacedAddress() : HL().word, value);
				break;
			case 7:
				A() = value;
				break;
			default:
				UNREACHABLE;
			}
		}

		uint8_t R2(const int r) {
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
				return BUS().read(HL());
			case 7:
				return A();
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
				BUS().write(HL(), value);
				break;
			case 7:
				A() = value;
				break;
			default:
				UNREACHABLE;
			}
		}

		register16_t& RP(const int rp) {
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

		register16_t& HL2() {
			if (LIKELY(!m_displaced))
				return HL();
			if (m_prefixDD)
				return IX();
			// Must be FD prefix
			return IY();
		}

		register16_t& RP2(const int rp) {
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
			auto overflow = (beforeNegative == valueNegative) && (beforeNegative != afterNegative);
			setFlag(f, VF, overflow);
		}

		static void adjustOverflowSub(uint8_t& f, const uint8_t before, const uint8_t value, const uint8_t calculation) {
			adjustOverflowSub(f, before & SF, value & SF, calculation & SF);
		}

		static void adjustOverflowSub(uint8_t& f, const int beforeNegative, const int valueNegative, const int afterNegative) {
			auto overflow = (beforeNegative != valueNegative) && (beforeNegative != afterNegative);
			setFlag(f, VF, overflow);
		}

		void subtract(uint8_t& operand, uint8_t value, int carry = 0);

		void executeCB(int x, int y, int z);
		void executeED(int x, int y, int z, int p, int q);
		void executeOther(int x, int y, int z, int p, int q);

		void increment(uint8_t& operand);
		void decrement(uint8_t& operand);

		void di();
		void ei();

		void retn();
		void reti();

		bool jrConditionalFlag(int flag);
		bool returnConditionalFlag(int flag);
		bool jumpConditionalFlag(int flag);
		bool callConditionalFlag(int flag);

		void sbc(register16_t value);
		void adc(register16_t value);
		void add(register16_t value);

		void add(uint8_t value, int carry = 0);
		void adc(uint8_t value);
		void sub(uint8_t value, int carry = 0);
		void sbc(uint8_t value);
		void andr(uint8_t value);
		void xorr(uint8_t value);
		void orr(uint8_t value);
		void compare(uint8_t value);

		void rlc(uint8_t& operand);
		void rrc(uint8_t& operand);
		void rl(uint8_t& operand);
		void rr(uint8_t& operand);
		void sla(uint8_t& operand);
		void sra(uint8_t& operand);
		void sll(uint8_t& operand);
		void srl(uint8_t& operand);

		uint8_t bit(int n, uint8_t operand);
		static uint8_t res(int n, uint8_t operand);
		static uint8_t set(int n, uint8_t operand);

		void daa();

		void scf();
		void ccf();
		void cpl();

		void xhtl();

		void blockCompare();

		void cpi();
		bool cpir();

		void cpd();
		bool cpdr();

		void blockLoad(register16_t source, register16_t destination);

		void ldi();
		bool ldir();

		void ldd();
		bool lddr();

		void ini();
		bool inir();

		void ind();
		bool indr();

		void blockOut();

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
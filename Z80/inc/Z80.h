#pragma once

#include <cstdint>

#include "IntelProcessor.h"
#include "InputOutput.h"
#include "Signal.h"

namespace EightBit {
	class Z80 : public IntelProcessor {
	public:
		struct refresh_t {

			bool high : 1;
			uint8_t variable : 7;

			refresh_t(uint8_t value)
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

		struct opcode_decoded_t {

			int x;
			int y;
			int z;
			int p;
			int q;

			opcode_decoded_t() {
				x = y = z = p = q = 0;
			}

			opcode_decoded_t(uint8_t opcode) {
				x = (opcode & 0b11000000) >> 6;	// 0 - 3
				y = (opcode & 0b00111000) >> 3;	// 0 - 7
				z = (opcode & 0b00000111);		// 0 - 7
				p = (y & 0b110) >> 1;			// 0 - 3
				q = (y & 1);					// 0 - 1
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

		Z80(Memory& memory, InputOutput& ports);

		Signal<Z80> ExecutingInstruction;

		void di();
		void ei();

		int interruptMaskable(uint8_t value) { return interrupt(true, value); }
		int interruptMaskable() { return interruptMaskable(0); }
		int interruptNonMaskable() { return interrupt(false, 0); }

		int interrupt(bool maskable, uint8_t value);

		int execute(uint8_t opcode);
		int step();

		// Mutable access to processor!!

		virtual register16_t& AF() override {
			return m_accumulatorFlags[m_accumulatorFlagsSet];
		}

		virtual register16_t& BC() override {
			return m_registers[m_registerSet][BC_IDX];
		}

		virtual register16_t& DE() override {
			return m_registers[m_registerSet][DE_IDX];
		}

		virtual register16_t& HL() override {
			return m_registers[m_registerSet][HL_IDX];
		}

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

		bool& M1() { return m1; }

		void exx() {
			m_registerSet ^= 1;
		}

		void exxAF() {
			m_accumulatorFlagsSet = !m_accumulatorFlagsSet;
		}

		virtual void reset();
		virtual void initialise();

	protected:
		InputOutput& m_ports;

		enum { BC_IDX, DE_IDX, HL_IDX };

		std::array<std::array<register16_t, 3>, 2> m_registers;
		int m_registerSet;

		std::array<register16_t, 2> m_accumulatorFlags;
		int m_accumulatorFlagsSet;

		register16_t m_ix;
		register16_t m_iy;

		refresh_t m_refresh;

		uint8_t iv;
		int m_interruptMode;
		bool m_iff1;
		bool m_iff2;

		bool m1;

		bool m_prefixCB;
		bool m_prefixDD;
		bool m_prefixED;
		bool m_prefixFD;

		int8_t m_displacement;
		bool m_displaced;

		std::array<opcode_decoded_t, 0x100> m_decodedOpcodes;

		int fetchExecute() {
			M1() = true;
			return execute(fetchByte());
		}

		uint8_t& DISPLACED() {
			m_memory.ADDRESS().word = MEMPTR().word = (m_prefixDD ? IX() : IY()).word + m_displacement;
			return m_memory.reference();
		}

		uint8_t& R(int r, uint8_t& a) {
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
				if (!m_displaced) {
					m_memory.ADDRESS() = HL();
					return m_memory.reference();
				}
				m_displacement = fetchByte();
				return DISPLACED();
			case 7:
				return a;
			default:
				__assume(0);
			}
			throw std::logic_error("Unhandled registry mechanism");
		}

		uint8_t& R2(int r, uint8_t& a) {
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
				m_memory.ADDRESS() = HL();
				return m_memory.reference();
			case 7:
				return a;
			default:
				__assume(0);
			}
			throw std::logic_error("Unhandled registry mechanism");
		}

		register16_t& RP(int rp) {
			__assume(rp < 4);
			__assume(rp >= 0);
			switch (rp) {
			case 3:
				return SP();
			case HL_IDX:
				return HL2();
			default:
				return m_registers[m_registerSet][rp];
			}
		}

		register16_t& HL2() {
			if (!m_displaced)
				return HL();
			if (m_prefixDD)
				return IX();
			// Must be FD prefix
			return IY();
		}

		register16_t& RP2(int rp) {
			__assume(rp < 4);
			__assume(rp >= 0);
			switch (rp) {
			case 3:
				return AF();
			case HL_IDX:
				return HL2();
			default:
				return m_registers[m_registerSet][rp];
			}
		}

		void addViaMemptr(uint8_t& f, register16_t& hl, register16_t operand) {
			MEMPTR().word = hl.word + 1;
			add(f, hl, operand);
		}

		void sbcViaMemptr(uint8_t& f, register16_t& hl, register16_t operand) {
			MEMPTR().word = hl.word + 1;
			sbc(f, hl, operand);
		}

		void adcViaMemptr(uint8_t& f, register16_t& hl, register16_t operand) {
			MEMPTR().word = hl.word + 1;
			adc(f, hl, operand);
		}

		static void adjustHalfCarryAdd(uint8_t& f, uint8_t before, uint8_t value, int calculation) {
			setFlag(f, HC, calculateHalfCarryAdd(before, value, calculation));
		}

		static void adjustHalfCarrySub(uint8_t& f, uint8_t before, uint8_t value, int calculation) {
			setFlag(f, HC, calculateHalfCarrySub(before, value, calculation));
		}

		static void adjustOverflowAdd(uint8_t& f, uint8_t before, uint8_t value, uint8_t calculation) {
			adjustOverflowAdd(f, before & SF, value & SF, calculation & SF);
		}

		static void adjustOverflowAdd(uint8_t& f, int beforeNegative, int valueNegative, int afterNegative) {
			auto overflow = (beforeNegative == valueNegative) && (beforeNegative != afterNegative);
			setFlag(f, VF, overflow);
		}

		static void adjustOverflowSub(uint8_t& f, uint8_t before, uint8_t value, uint8_t calculation) {
			adjustOverflowSub(f, before & SF, value & SF, calculation & SF);
		}

		static void adjustOverflowSub(uint8_t& f, int beforeNegative, int valueNegative, int afterNegative) {
			auto overflow = (beforeNegative != valueNegative) && (beforeNegative != afterNegative);
			setFlag(f, VF, overflow);
		}

		static void subtract(uint8_t& f, uint8_t& operand, uint8_t value, int carry = 0);

		void executeCB(int x, int y, int z);
		void executeED(int x, int y, int z, int p, int q);
		void executeOther(int x, int y, int z, int p, int q);

		static void postIncrement(uint8_t& f, uint8_t value);
		static void postDecrement(uint8_t& f, uint8_t value);

		void retn();
		void reti();

		bool jrConditionalFlag(uint8_t& f, int flag);
		bool returnConditionalFlag(uint8_t& f, int flag);
		bool jumpConditionalFlag(uint8_t& f, int flag);
		bool callConditionalFlag(uint8_t& f, int flag);

		static void sbc(uint8_t& f, register16_t& operand, register16_t value);
		static void adc(uint8_t& f, register16_t& operand, register16_t value);

		static void add(uint8_t& f, register16_t& operand, register16_t value);

		static void add(uint8_t& f, uint8_t& operand, uint8_t value, int carry = 0);
		static void adc(uint8_t& f, uint8_t& operand, uint8_t value);
		static void sub(uint8_t& f, uint8_t& operand, uint8_t value, int carry = 0);
		static void sbc(uint8_t& f, uint8_t& operand, uint8_t value);
		static void andr(uint8_t& f, uint8_t& operand, uint8_t value);
		static void xorr(uint8_t& f, uint8_t& operand, uint8_t value);
		static void orr(uint8_t& f, uint8_t& operand, uint8_t value);
		static void compare(uint8_t& f, uint8_t check, uint8_t value);

		static uint8_t& rlc(uint8_t& f, uint8_t& operand);
		static uint8_t& rrc(uint8_t& f, uint8_t& operand);
		static uint8_t& rl(uint8_t& f, uint8_t& operand);
		static uint8_t& rr(uint8_t& f, uint8_t& operand);
		static uint8_t& sla(uint8_t& f, uint8_t& operand);
		static uint8_t& sra(uint8_t& f, uint8_t& operand);
		static uint8_t& sll(uint8_t& f, uint8_t& operand);
		static uint8_t& srl(uint8_t& f, uint8_t& operand);

		static uint8_t& bit(uint8_t& f, int n, uint8_t& operand);
		static uint8_t& res(int n, uint8_t& operand);
		static uint8_t& set(int nit, uint8_t& operand);

		static void daa(uint8_t& a, uint8_t& f);

		static void scf(uint8_t a, uint8_t& f);
		static void ccf(uint8_t a, uint8_t& f);
		static void cpl(uint8_t& a, uint8_t& f);

		void xhtl(register16_t& operand);

		void blockCompare(uint8_t a, uint8_t& f);

		void cpi(uint8_t a, uint8_t& f);
		bool cpir(uint8_t a, uint8_t& f);

		void cpd(uint8_t a, uint8_t& f);
		bool cpdr(uint8_t a, uint8_t& f);

		void blockLoad(uint8_t a, uint8_t& f, register16_t source, register16_t destination);

		void ldi(uint8_t a, uint8_t& f);
		bool ldir(uint8_t a, uint8_t& f);

		void ldd(uint8_t a, uint8_t& f);
		bool lddr(uint8_t a, uint8_t& f);

		void ini(uint8_t& f);
		bool inir(uint8_t& f);

		void ind(uint8_t& f);
		bool indr(uint8_t& f);

		void blockOut(uint8_t& f);

		void outi(uint8_t& f);
		bool otir(uint8_t& f);

		void outd(uint8_t& f);
		bool otdr(uint8_t& f);

		static void neg(uint8_t& a, uint8_t& f);

		void rrd(uint8_t& a, uint8_t& f);
		void rld(uint8_t& a, uint8_t& f);

		void writePort(uint8_t port, uint8_t a);
		void writePort();

		void readPort(uint8_t port, uint8_t& a);
		void readPort();
	};
}
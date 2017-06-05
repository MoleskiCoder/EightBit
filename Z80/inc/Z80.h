#pragma once

#include <cstdint>

#include "Processor.h"
#include "InputOutput.h"

namespace EightBit {
	class Z80 : public Processor {
	public:
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

		void disableInterrupts();
		void enableInterrupts();

		int interruptMaskable(uint8_t value) { return interrupt(true, value); }
		int interruptMaskable() { return interruptMaskable(0); }
		int interruptNonMaskable() { return interrupt(false, 0); }

		int interrupt(bool maskable, uint8_t value);

		int execute(uint8_t opcode);
		int step();

		bool getM1() const { return m1; }

		// Mutable access to processor!!

		register16_t& AF() {
			return m_accumulatorFlags[m_accumulatorFlagsSet];
		}

		uint8_t& A() { return AF().high; }
		uint8_t& F() { return AF().low; }

		register16_t& BC() {
			return m_registers[m_registerSet][BC_IDX];
		}

		uint8_t& B() { return BC().high; }
		uint8_t& C() { return BC().low; }

		register16_t& DE() {
			return m_registers[m_registerSet][DE_IDX];
		}

		uint8_t& D() { return DE().high; }
		uint8_t& E() { return DE().low; }

		register16_t& HL() {
			return m_registers[m_registerSet][HL_IDX];
		}

		uint8_t& H() { return HL().high; }
		uint8_t& L() { return HL().low; }

		register16_t& IX() { return m_ix; }
		uint8_t& IXH() { return IX().high; }
		uint8_t& IXL() { return IX().low; }

		register16_t& IY() { return m_iy; }
		uint8_t& IYH() { return IY().high; }
		uint8_t& IYL() { return IY().low; }

		uint8_t& REFRESH() { return m_refresh; }
		uint8_t& IV() { return iv; }
		int& IM() { return m_interruptMode; }
		bool& IFF1() { return m_iff1; }
		bool& IFF2() { return m_iff2; }

		register16_t& MEMPTR() { return m_memptr; }

		bool& M1() { return m1; }

		void exx() {
			m_registerSet ^= 1;
		}

		void exxAF() {
			m_accumulatorFlagsSet = !m_accumulatorFlagsSet;
		}

		virtual void reset();
		virtual void initialise();

	private:
		InputOutput& m_ports;

		enum { BC_IDX, DE_IDX, HL_IDX };

		std::array<std::array<register16_t, 3>, 2> m_registers;
		int m_registerSet;

		std::array<register16_t, 2> m_accumulatorFlags;
		int m_accumulatorFlagsSet;

		register16_t m_ix;
		register16_t m_iy;

		uint8_t m_refresh;
		uint8_t iv;
		int m_interruptMode;
		bool m_iff1;
		bool m_iff2;

		register16_t m_memptr;

		bool m1;

		bool m_prefixCB;
		bool m_prefixDD;
		bool m_prefixED;
		bool m_prefixFD;

		int8_t m_displacement;

		std::array<bool, 8> m_halfCarryTableAdd = { { false, false, true, false, true, false, true, true } };
		std::array<bool, 8> m_halfCarryTableSub = { { false, true, true, true, false, false, false, true } };

		int fetchExecute() {
			M1() = true;
			return execute(fetchByteExecute());
		}

		uint8_t fetchByteExecute() {
			if (!getM1())
				throw std::logic_error("M1 cannot be high");
			return fetchByte();
		}

		uint8_t fetchByteData() {
			if (getM1())
				throw std::logic_error("M1 cannot be low");
			return fetchByte();
		}

		void incrementRefresh() {
			auto incremented = ((REFRESH() & Mask7) + 1) & Mask7;
			REFRESH() = (REFRESH() & Bit7) | incremented;
		}

		void clearFlag(int flag) { F() &= ~flag; }
		void setFlag(int flag) { F() |= flag; }

		void setFlag(int flag, int condition) { setFlag(flag, condition != 0); }
		void setFlag(int flag, uint32_t condition) { setFlag(flag, condition != 0); }
		void setFlag(int flag, bool condition) { condition ? setFlag(flag) : clearFlag(flag); }

		void clearFlag(int flag, int condition) { clearFlag(flag, condition != 0); }
		void clearFlag(int flag, uint32_t condition) { clearFlag(flag, condition != 0); }
		void clearFlag(int flag, bool condition) { condition ? clearFlag(flag) : setFlag(flag); }

		uint8_t& DISPLACED() {
			if (!(m_prefixDD || m_prefixFD))
				throw std::logic_error("Unprefixed indexed displacement requested");
			m_memory.ADDRESS().word = MEMPTR().word = (m_prefixDD ? IX() : IY()).word + m_displacement;
			return m_memory.reference();
		}

		uint8_t& R(int r) {
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
				return ALT_HL().high;
			case 5:
				return ALT_HL().low;
			case 6:
				if (m_prefixDD || m_prefixFD) {
					m_displacement = fetchByteData();
					return DISPLACED();
				}
				m_memory.ADDRESS() = HL();
				return m_memory.reference();
			case 7:
				return A();
			}
			throw std::logic_error("Unhandled registry mechanism");
		}

		uint8_t& R2(int r) {
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
				return A();
			}
			throw std::logic_error("Unhandled registry mechanism");
		}

		register16_t& RP(int rp) {
			switch (rp) {
			case 3:
				return sp;
			case HL_IDX:
				return ALT_HL();
			default:
				return m_registers[m_registerSet][rp];
			}
		}

		register16_t& ALT_HL() {
			if (m_prefixDD)
				return IX();
			else if (m_prefixFD)
				return IY();
			return HL();
		}

		register16_t& RP2(int rp) {
			switch (rp) {
			case 3:
				return AF();
			case HL_IDX:
				return ALT_HL();
			default:
				return m_registers[m_registerSet][rp];
			}
		}

		uint8_t getViaMemptr(register16_t address) {
			m_memory.ADDRESS() = address;
			MEMPTR().word = address.word + 1;
			return m_memory.reference();
		}

		void setViaMemptr(register16_t address, uint8_t value) {
			m_memory.ADDRESS() = address;
			m_memory.reference() = value;
			++address.word;
			MEMPTR().low = address.low;
			MEMPTR().high = value;
		}

		register16_t getWordViaMemptr(register16_t address) {
			register16_t returned;
			m_memory.ADDRESS() = address;
			returned.low = m_memory.reference();
			m_memory.ADDRESS().word++;
			returned.high = m_memory.reference();
			MEMPTR() = m_memory.ADDRESS();
			return returned;
		}

		void setWordViaMemptr(register16_t address, register16_t value) {
			m_memory.ADDRESS() = address;
			m_memory.reference() = value.low;
			m_memory.ADDRESS().word++;
			m_memory.reference() = value.high;
			MEMPTR() = m_memory.ADDRESS();
		}

		void setPcViaMemptr(register16_t address) {
			MEMPTR() = pc = address;
		}

		void addViaMemptr(register16_t& hl, register16_t operand) {
			MEMPTR().word = hl.word + 1;
			add(hl, operand);
		}

		void sbcViaMemptr(register16_t& hl, register16_t operand) {
			MEMPTR().word = hl.word + 1;
			sbc(hl, operand);
		}

		void adcViaMemptr(register16_t& hl, register16_t operand) {
			MEMPTR().word = hl.word + 1;
			adc(hl, operand);
		}

		int buildHalfCarryIndex(uint8_t before, uint8_t value, int calculation) {
			return ((before & 0x88) >> 1) | ((value & 0x88) >> 2) | ((calculation & 0x88) >> 3);
		}

		void adjustHalfCarryAdd(uint8_t before, uint8_t value, int calculation) {
			auto index = buildHalfCarryIndex(before, value, calculation);
			setFlag(HC, m_halfCarryTableAdd[index & 0x7]);
		}

		void adjustHalfCarrySub(uint8_t before, uint8_t value, int calculation) {
			auto index = buildHalfCarryIndex(before, value, calculation);
			setFlag(HC, m_halfCarryTableSub[index & 0x7]);
		}

		void adjustOverflowAdd(uint8_t before, uint8_t value, uint8_t calculation) {
			adjustOverflowAdd(before & SF, value & SF, calculation & SF);
		}

		void adjustOverflowAdd(int beforeNegative, int valueNegative, int afterNegative) {
			auto overflow = (beforeNegative == valueNegative) && (beforeNegative != afterNegative);
			setFlag(VF, overflow);
		}

		void adjustOverflowSub(uint8_t before, uint8_t value, uint8_t calculation) {
			adjustOverflowSub(before & SF, value & SF, calculation & SF);
		}

		void adjustOverflowSub(int beforeNegative, int valueNegative, int afterNegative) {
			auto overflow = (beforeNegative != valueNegative) && (beforeNegative != afterNegative);
			setFlag(VF, overflow);
		}

		void executeCB(int x, int y, int z, int p, int q);
		void executeED(int x, int y, int z, int p, int q);
		void executeOther(int x, int y, int z, int p, int q);

		void adjustSign(uint8_t value);
		void adjustZero(uint8_t value);
		void adjustParity(uint8_t value);
		void adjustSZ(uint8_t value);
		void adjustSZP(uint8_t value);
		void adjustXY(uint8_t value);
		void adjustSZPXY(uint8_t value);
		void adjustSZXY(uint8_t value);

		void postIncrement(uint8_t value);
		void postDecrement(uint8_t value);

		void restart(uint8_t address);

		void jrConditional(int conditional);
		void jrConditionalFlag(int flag);

		void ret();
		void retn();
		void reti();

		void returnConditional(int condition);
		void returnConditionalFlag(int flag);

		void jumpConditional(int condition);
		void jumpConditionalFlag(int flag);

		void call(register16_t address);
		void callConditional(register16_t address, int condition);
		void callConditionalFlag(register16_t address, int flag);

		void sbc(register16_t& operand, register16_t value);
		void adc(register16_t& operand, register16_t value);

		void add(register16_t& operand, register16_t value);

		void add(uint8_t& operand, uint8_t value, int carry = 0);
		void adc(uint8_t& operand, uint8_t value);
		void sub(uint8_t& operand, uint8_t value, int carry = 0);
		void sbc(uint8_t& operand, uint8_t value);
		void andr(uint8_t& operand, uint8_t value);
		void xorr(uint8_t& operand, uint8_t value);
		void orr(uint8_t& operand, uint8_t value);
		void compare(uint8_t value);

		void rlca();
		void rrca();
		void rla();
		void rra();

		void rlc(uint8_t& operand);
		void rrc(uint8_t& operand);
		void rl(uint8_t& operand);
		void rr(uint8_t& operand);
		void sla(uint8_t& operand);
		void sra(uint8_t& operand);
		void sll(uint8_t& operand);
		void srl(uint8_t& operand);

		void bit(int n, uint8_t& operand);
		void res(int n, uint8_t& operand);
		void set(int nit, uint8_t& operand);

		void daa();

		void scf();
		void ccf();
		void cpl();

		void xhtl(register16_t& operand);
		void xhtl();

		void blockCompare();

		void cpi();
		void cpir();

		void cpd();
		void cpdr();

		void blockLoad(register16_t source, register16_t destination);

		void ldi();
		void ldir();

		void ldd();
		void lddr();

		void ini();
		void inir();

		void ind();
		void indr();

		void blockOut();

		void outi();
		void otir();

		void outd();
		void otdr();

		void neg();

		void rrd();
		void rld();

		void writePort() {
			m_ports.write(m_memory.ADDRESS().low, m_memory.DATA());
		}

		void readPort() {
			m_memory.placeDATA(m_ports.read(m_memory.ADDRESS().low));
		}
	};
}
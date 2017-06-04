#pragma once

#include <cstdint>

#include "Processor.h"

class LR35902 : public Processor {
public:
	enum StatusBits {
		ZF = Bit7,
		NF = Bit6,
		HC = Bit5,
		CF = Bit4,
	};

	LR35902(Bus& memory);

	Signal<LR35902> ExecutingInstruction;

	void stop() { m_stopped = true; }
	void start() { m_stopped = false; }
	bool stopped() const { return m_stopped; }

	bool& IME() { return m_ime; }

	void di();
	void ei();

	int interrupt(uint8_t value);

	int execute(uint8_t opcode);
	int step();

	// Mutable access to processor!!

	register16_t& AF() {
		m_accumulatorFlag.low &= 0xf0;
		return m_accumulatorFlag;
	}

	uint8_t& A() { return AF().high; }
	uint8_t& F() { return AF().low; }

	register16_t& BC() {
		return m_registers[BC_IDX];
	}

	uint8_t& B() { return BC().high; }
	uint8_t& C() { return BC().low; }

	register16_t& DE() {
		return m_registers[DE_IDX];
	}

	uint8_t& D() { return DE().high; }
	uint8_t& E() { return DE().low; }

	register16_t& HL() {
		return m_registers[HL_IDX];
	}

	uint8_t& H() { return HL().high; }
	uint8_t& L() { return HL().low; }

	virtual void reset();
	virtual void initialise();

private:
	enum { BC_IDX, DE_IDX, HL_IDX };

	std::array<register16_t, 3> m_registers;
	register16_t m_accumulatorFlag;

	bool m_ime;

	bool m_prefixCB;

	bool m_stopped;

	std::array<bool, 8> m_halfCarryTableAdd = { { false, false, true, false, true, false, true, true } };
	std::array<bool, 8> m_halfCarryTableSub = { { false, true, true, true, false, false, false, true } };

	int fetchExecute() {
		return execute(fetchByte());
	}

	void clearFlag(int flag) { F() &= ~flag; }
	void setFlag(int flag) { F() |= flag; }

	void setFlag(int flag, int condition) { setFlag(flag, condition != 0); }
	void setFlag(int flag, uint32_t condition) { setFlag(flag, condition != 0); }
	void setFlag(int flag, bool condition) { condition ? setFlag(flag) : clearFlag(flag); }

	void clearFlag(int flag, int condition) { clearFlag(flag, condition != 0); }
	void clearFlag(int flag, uint32_t condition) { clearFlag(flag, condition != 0); }
	void clearFlag(int flag, bool condition) { condition ? clearFlag(flag) : setFlag(flag); }

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
			return H();
		case 5:
			return L();
		case 6:
			return m_memory.reference(HL().word);
		case 7:
			return A();
		}
		throw std::logic_error("Unhandled registry mechanism");
	}

	uint16_t& RP(int rp) {
		switch (rp) {
		case 3:
			return sp;
		default:
			return m_registers[rp].word;
		}
	}

	uint16_t& RP2(int rp) {
		switch (rp) {
		case 3:
			return AF().word;
		default:
			return m_registers[rp].word;
		}
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

	void executeCB(int x, int y, int z, int p, int q);
	void executeOther(int x, int y, int z, int p, int q);

	void adjustZero(uint8_t value);

	void postIncrement(uint8_t value);
	void postDecrement(uint8_t value);

	void restart(uint8_t address);

	void jrConditional(int conditional);
	void jrConditionalFlag(int flag);

	void ret();
	void reti();

	void returnConditional(int condition);
	void returnConditionalFlag(int flag);

	void jumpConditional(int condition);
	void jumpConditionalFlag(int flag);

	void call(uint16_t address);
	void callConditional(uint16_t address, int condition);
	void callConditionalFlag(uint16_t address, int flag);

	uint16_t sbc(uint16_t value);
	uint16_t adc(uint16_t value);

	uint16_t add(uint16_t value);

	void sub(uint8_t& operand, uint8_t value, bool carry);
	void sub(uint8_t& operand, uint8_t value);
	void sbc(uint8_t& operand, uint8_t value);

	void add(uint8_t& operand, uint8_t value, bool carry);
	void add(uint8_t& operand, uint8_t value);
	void adc(uint8_t& operand, uint8_t value);

	void andr(uint8_t& operand, uint8_t value);

	void anda(uint8_t value);
	void xora(uint8_t value);
	void ora(uint8_t value);
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
	void srl(uint8_t& operand);

	void bit(int n, uint8_t& operand);
	void res(int n, uint8_t& operand);
	void set(int nit, uint8_t& operand);

	void daa();

	void scf();
	void ccf();
	void cpl();

	void swap(uint8_t& operand);
};
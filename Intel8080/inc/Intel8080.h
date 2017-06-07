#pragma once

// Auxiliary carry logic from https://github.com/begoon/i8080-core

#include "Processor.h"
#include "StatusFlags.h"
#include "InputOutput.h"

namespace EightBit {
	class Intel8080 : public Processor {
	public:
		typedef std::function<void()> instruction_t;

		enum AddressingMode {
			Unknown,
			Implied,	// zero bytes
			Immediate,	// single byte
			Absolute	// two bytes, little endian
		};

		struct Instruction {
			instruction_t vector = nullptr;
			AddressingMode mode = Unknown;
			std::string disassembly;
			int count = 0;
		};

		Intel8080(Memory& memory, InputOutput& ports);

		Signal<Intel8080> ExecutingInstruction;

		const std::array<Instruction, 0x100>& getInstructions() const { return instructions; }

		uint8_t& A() { return a; }
		StatusFlags& F() { return f; }

		register16_t& BC() { return bc; }
		uint8_t& B() { return BC().high; }
		uint8_t& C() { return BC().low; }

		register16_t& DE() { return de; }
		uint8_t& D() { return DE().high; }
		uint8_t& E() { return DE().low; }

		register16_t& HL() { return hl; }
		uint8_t& H() { return HL().high; }
		uint8_t& L() { return HL().low; }

		bool isInterruptable() const {
			return m_interrupt;
		}

		void disableInterrupts() { m_interrupt = false; }
		void enableInterrupts() { m_interrupt = true; }

		int interrupt(uint8_t value) {
			if (isInterruptable()) {
				disableInterrupts();
				return execute(value);
			}
			return 0;
		}

		virtual void initialise();
		int step();

	private:
		InputOutput& m_ports;

		std::array<Instruction, 0x100> instructions;

		std::array<bool, 8> m_halfCarryTableAdd = { { false, false, true, false, true, false, true, true } };
		std::array<bool, 8> m_halfCarryTableSub = { { false, true, true, true, false, false, false, true } };

		uint8_t a;
		StatusFlags f;

		register16_t bc;
		register16_t de;
		register16_t hl;

		bool m_interrupt;

		int execute(uint8_t opcode);

		int execute(const Instruction& instruction) {
			cycles = 0;
			instruction.vector();
			return cycles + instruction.count;
		}

		void adjustSign(uint8_t value) { F().S = ((value & 0x80) != 0); }
		void adjustZero(uint8_t value) { F().Z = (value == 0); }

		void adjustParity(uint8_t value) {
			static const uint8_t lookup[0x10] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
			auto set = (lookup[highNibble(value)] + lookup[lowNibble(value)]);
			F().P = (set % 2) == 0;
		}

		void adjustSZP(uint8_t value) {
			adjustSign(value);
			adjustZero(value);
			adjustParity(value);
		}

		int buildAuxiliaryCarryIndex(uint8_t value, int calculation) {
			return ((A() & 0x88) >> 1) | ((value & 0x88) >> 2) | ((calculation & 0x88) >> 3);
		}

		void adjustAuxiliaryCarryAdd(uint8_t value, int calculation) {
			auto index = buildAuxiliaryCarryIndex(value, calculation);
			F().AC = m_halfCarryTableAdd[index & 0x7];
		}

		void adjustAuxiliaryCarrySub(uint8_t value, int calculation) {
			auto index = buildAuxiliaryCarryIndex(value, calculation);
			F().AC = !m_halfCarryTableSub[index & 0x7];
		}

		void postIncrement(uint8_t value) {
			adjustSZP(value);
			F().AC = (value & 0x0f) == 0;
		}

		void postDecrement(uint8_t value) {
			adjustSZP(value);
			F().AC = (value & 0x0f) != 0xf;
		}

		static Instruction INS(instruction_t method, AddressingMode mode, std::string disassembly, int cycles);
		Instruction UNKNOWN();

		void installInstructions();

		//

		register16_t fetchWord() {
			register16_t returned;
			Processor::fetchWord(returned);
			return returned;
		}

		//

		void compare(uint8_t value) {
			uint16_t subtraction = A() - value;
			adjustSZP((uint8_t)subtraction);
			adjustAuxiliaryCarrySub(value, subtraction);
			F().C = subtraction > 0xff;
		}

		void callAddress(uint16_t address) {
			register16_t saved = pc;
			saved.word += 2;
			pushWord(saved);
			pc.word = address;
		}

		void restart(uint8_t position) {
			uint16_t address = position << 3;
			pushWord(pc);
			pc.word = address;
		}

		void jmpConditional(int conditional) {
			auto destination = fetchWord();
			if (conditional)
				pc = destination;
		}

		void callConditional(int condition) {
			if (condition) {
				call();
				cycles += 6;
			}
			else {
				pc.word += 2;
			}
		}

		void returnConditional(int condition) {
			if (condition) {
				ret();
				cycles += 6;
			}
		}

		void anda(uint8_t value) {
			F().AC = (((A() | value) & 0x8) != 0);
			F().C = false;
			adjustSZP(A() &= value);
		}

		void ora(uint8_t value) {
			F().AC = F().C = false;
			adjustSZP(A() |= value);
		}

		void xra(uint8_t value) {
			F().AC = F().C = false;
			adjustSZP(A() ^= value);
		}

		void add(uint8_t value, int carry = 0) {
			register16_t sum;
			sum.word = A() + value + carry;
			adjustAuxiliaryCarryAdd(value, sum.word);
			A() = sum.low;
			F().C = sum.word > 0xff;
			adjustSZP(A());
		}

		void adc(uint8_t value) {
			add(value, F().C);
		}

		void dad(uint16_t value) {
			uint32_t sum = HL().word + value;
			F().C = sum > 0xffff;
			HL().word = (uint16_t)sum;
		}

		void sub(uint8_t value, int carry = 0) {
			register16_t difference;
			difference.word = A() - value - carry;
			adjustAuxiliaryCarrySub(value, difference.word);
			A() = difference.low;
			F().C = difference.word > 0xff;
			adjustSZP(A());
		}

		void sbb(uint8_t value) {
			sub(value, F().C);
		}

		void mov_m_r(uint8_t value) {
			m_memory.set(HL().word, value);
		}

		uint8_t mov_r_m() {
			return m_memory.get(HL().word);
		}

		//

		void ___();

		// Move, load, and store

		void mov_a_a() { }
		void mov_a_b() { A() = B(); }
		void mov_a_c() { A() = C(); }
		void mov_a_d() { A() = D(); }
		void mov_a_e() { A() = E(); }
		void mov_a_h() { A() = H(); }
		void mov_a_l() { A() = L(); }

		void mov_b_a() { B() = A(); }
		void mov_b_b() { }
		void mov_b_c() { B() = C(); }
		void mov_b_d() { B() = D(); }
		void mov_b_e() { B() = E(); }
		void mov_b_h() { B() = H(); }
		void mov_b_l() { B() = L(); }

		void mov_c_a() { C() = A(); }
		void mov_c_b() { C() = B(); }
		void mov_c_c() { }
		void mov_c_d() { C() = D(); }
		void mov_c_e() { C() = E(); }
		void mov_c_h() { C() = H(); }
		void mov_c_l() { C() = L(); }

		void mov_d_a() { D() = A(); }
		void mov_d_b() { D() = B(); }
		void mov_d_c() { D() = C(); }
		void mov_d_d() { }
		void mov_d_e() { D() = E(); }
		void mov_d_h() { D() = H(); }
		void mov_d_l() { D() = L(); }

		void mov_e_a() { E() = A(); }
		void mov_e_b() { E() = B(); }
		void mov_e_c() { E() = C(); }
		void mov_e_d() { E() = D(); }
		void mov_e_e() { }
		void mov_e_h() { E() = H(); }
		void mov_e_l() { E() = L(); }

		void mov_h_a() { H() = A(); }
		void mov_h_b() { H() = B(); }
		void mov_h_c() { H() = C(); }
		void mov_h_d() { H() = D(); }
		void mov_h_e() { H() = E(); }
		void mov_h_h() { }
		void mov_h_l() { H() = L(); }

		void mov_l_a() { L() = A(); }
		void mov_l_b() { L() = B(); }
		void mov_l_c() { L() = C(); }
		void mov_l_d() { L() = D(); }
		void mov_l_e() { L() = E(); }
		void mov_l_h() { L() = H(); }
		void mov_l_l() { }

		void mov_m_a() { mov_m_r(A()); }
		void mov_m_b() { mov_m_r(B()); }
		void mov_m_c() { mov_m_r(C()); }
		void mov_m_d() { mov_m_r(D()); }
		void mov_m_e() { mov_m_r(E()); }
		void mov_m_h() { mov_m_r(H()); }
		void mov_m_l() { mov_m_r(L()); }

		void mov_a_m() { A() = mov_r_m(); }
		void mov_b_m() { B() = mov_r_m(); }
		void mov_c_m() { C() = mov_r_m(); }
		void mov_d_m() { D() = mov_r_m(); }
		void mov_e_m() { E() = mov_r_m(); }
		void mov_h_m() { H() = mov_r_m(); }
		void mov_l_m() { L() = mov_r_m(); }

		void mvi_a() { A() = fetchByte(); }
		void mvi_b() { B() = fetchByte(); }
		void mvi_c() { C() = fetchByte(); }
		void mvi_d() { D() = fetchByte(); }
		void mvi_e() { E() = fetchByte(); }
		void mvi_h() { H() = fetchByte(); }
		void mvi_l() { L() = fetchByte(); }

		void mvi_m() {
			auto data = fetchByte();
			m_memory.set(HL().word, data);
		}

		void lxi_b() { Processor::fetchWord(BC()); }
		void lxi_d() { Processor::fetchWord(DE()); }
		void lxi_h() { Processor::fetchWord(HL()); }

		void stax_b() { m_memory.set(BC().word, A()); }
		void stax_d() { m_memory.set(DE().word, A()); }

		void ldax_b() { A() = m_memory.get(BC().word); }
		void ldax_d() { A() = m_memory.get(DE().word); }

		void sta() {
			auto destination = fetchWord();
			m_memory.set(destination.word, A());
		}

		void lda() {
			auto source = fetchWord();
			A() = m_memory.get(source.word);
		}

		void shld() {
			auto destination = fetchWord();
			m_memory.setWord(destination.word, HL());
		}

		void lhld() {
			HL() = m_memory.getWord(fetchWord().word);
		}

		void xchg() {
			std::swap(DE(), HL());
		}

		// stack ops

		void push_b() { pushWord(BC()); }
		void push_d() { pushWord(DE()); }
		void push_h() { pushWord(HL()); }

		void push_psw() {
			register16_t pair;
			pair.low = F();
			pair.high = A();
			pushWord(pair);
		}

		void pop_b() { popWord(BC()); }
		void pop_d() { popWord(DE()); }
		void pop_h() { popWord(HL()); }

		void pop_psw() {
			F() = pop();
			A() = pop();
		}

		void xhtl() {
			auto tos = m_memory.getWord(sp.word);
			m_memory.setWord(sp.word, HL());
			HL() = tos;
		}

		void sphl() {
			sp = HL();
		}

		void lxi_sp() {
			Processor::fetchWord(sp);
		}

		void inx_sp() { ++sp.word; }
		void dcx_sp() { --sp.word; }

		// jump

		void jmp() { jmpConditional(true); }

		void jc() { jmpConditional(F().C); }
		void jnc() { jmpConditional(!F().C); }

		void jz() { jmpConditional(F().Z); }
		void jnz() { jmpConditional(!F().Z); }

		void jpe() { jmpConditional(F().P); }
		void jpo() { jmpConditional(!F().P); }

		void jm() { jmpConditional(F().S); }
		void jp() { jmpConditional(!F().S); }

		void pchl() {
			pc = HL();
		}

		// call

		void call() {
			auto destination = m_memory.getWord(pc.word);
			callAddress(destination.word);
		}

		void cc() { callConditional(F().C); }
		void cnc() { callConditional(!F().C); }

		void cpe() { callConditional(F().P); }
		void cpo() { callConditional(!F().P); }

		void cz() { callConditional(F().Z); }
		void cnz() { callConditional(!F().Z); }

		void cm() { callConditional(F().S); }
		void cp() { callConditional(!F().S); }

		// return

		void ret() {
			popWord(pc);
		}

		void rc() { returnConditional(F().C); }
		void rnc() { returnConditional(!F().C); }

		void rz() { returnConditional(F().Z); }
		void rnz() { returnConditional(!F().Z); }

		void rpe() { returnConditional(F().P); }
		void rpo() { returnConditional(!F().P); }

		void rm() { returnConditional(F().S); }
		void rp() { returnConditional(!F().S); }

		// restart

		void rst_0() { restart(0); }
		void rst_1() { restart(1); }
		void rst_2() { restart(2); }
		void rst_3() { restart(3); }
		void rst_4() { restart(4); }
		void rst_5() { restart(5); }
		void rst_6() { restart(6); }
		void rst_7() { restart(7); }

		// increment and decrement

		void inr_a() { postIncrement(++A()); }
		void inr_b() { postIncrement(++B()); }
		void inr_c() { postIncrement(++C()); }
		void inr_d() { postIncrement(++D()); }
		void inr_e() { postIncrement(++E()); }
		void inr_h() { postIncrement(++H()); }
		void inr_l() { postIncrement(++L()); }

		void inr_m() {
			auto value = m_memory.get(HL().word);
			postIncrement(++value);
			m_memory.set(HL().word, value);
		}

		void dcr_a() { postDecrement(--A()); }
		void dcr_b() { postDecrement(--B()); }
		void dcr_c() { postDecrement(--C()); }
		void dcr_d() { postDecrement(--D()); }
		void dcr_e() { postDecrement(--E()); }
		void dcr_h() { postDecrement(--H()); }
		void dcr_l() { postDecrement(--L()); }

		void dcr_m() {
			auto value = m_memory.get(HL().word);
			postDecrement(--value);
			m_memory.set(HL().word, value);
		}

		void inx_b() { ++BC().word; }
		void inx_d() { ++DE().word; }
		void inx_h() { ++HL().word; }

		void dcx_b() { --BC().word; }
		void dcx_d() { --DE().word; }
		void dcx_h() { --HL().word; }

		// add

		void add_a() { add(A()); }
		void add_b() { add(B()); }
		void add_c() { add(C()); }
		void add_d() { add(D()); }
		void add_e() { add(E()); }
		void add_h() { add(H()); }
		void add_l() { add(L()); }

		void add_m() {
			auto value = m_memory.get(HL().word);
			add(value);
		}

		void adi() { add(fetchByte()); }

		void adc_a() { adc(A()); }
		void adc_b() { adc(B()); }
		void adc_c() { adc(C()); }
		void adc_d() { adc(D()); }
		void adc_e() { adc(E()); }
		void adc_h() { adc(H()); }
		void adc_l() { adc(L()); }

		void adc_m() {
			auto value = m_memory.get(HL().word);
			adc(value);
		}

		void aci() { adc(fetchByte()); }

		void dad_b() { dad(BC().word); }
		void dad_d() { dad(DE().word); }
		void dad_h() { dad(HL().word); }
		void dad_sp() { dad(sp.word); }

		// subtract

		void sub_a() { sub(A()); }
		void sub_b() { sub(B()); }
		void sub_c() { sub(C()); }
		void sub_d() { sub(D()); }
		void sub_e() { sub(E()); }
		void sub_h() { sub(H()); }
		void sub_l() { sub(L()); }

		void sub_m() {
			auto value = m_memory.get(HL().word);
			sub(value);
		}

		void sbb_a() { sbb(A()); }
		void sbb_b() { sbb(B()); }
		void sbb_c() { sbb(C()); }
		void sbb_d() { sbb(D()); }
		void sbb_e() { sbb(E()); }
		void sbb_h() { sbb(H()); }
		void sbb_l() { sbb(L()); }

		void sbb_m() {
			auto value = m_memory.get(HL().word);
			sbb(value);
		}

		void sbi() {
			auto value = fetchByte();
			sbb(value);
		}

		void sui() {
			auto value = fetchByte();
			sub(value);
		}

		// logical

		void ana_a() { anda(A()); }
		void ana_b() { anda(B()); }
		void ana_c() { anda(C()); }
		void ana_d() { anda(D()); }
		void ana_e() { anda(E()); }
		void ana_h() { anda(H()); }
		void ana_l() { anda(L()); }

		void ana_m() {
			auto value = m_memory.get(HL().word);
			anda(value);
		}

		void ani() { anda(fetchByte()); }

		void xra_a() { xra(A()); }
		void xra_b() { xra(B()); }
		void xra_c() { xra(C()); }
		void xra_d() { xra(D()); }
		void xra_e() { xra(E()); }
		void xra_h() { xra(H()); }
		void xra_l() { xra(L()); }

		void xra_m() {
			auto value = m_memory.get(HL().word);
			xra(value);
		}

		void xri() { xra(fetchByte()); }

		void ora_a() { ora(A()); }
		void ora_b() { ora(B()); }
		void ora_c() { ora(C()); }
		void ora_d() { ora(D()); }
		void ora_e() { ora(E()); }
		void ora_h() { ora(H()); }
		void ora_l() { ora(L()); }

		void ora_m() {
			auto value = m_memory.get(HL().word);
			ora(value);
		}

		void ori() { ora(fetchByte()); }

		void cmp_a() { compare(A()); }
		void cmp_b() { compare(B()); }
		void cmp_c() { compare(C()); }
		void cmp_d() { compare(D()); }
		void cmp_e() { compare(E()); }
		void cmp_h() { compare(H()); }
		void cmp_l() { compare(L()); }

		void cmp_m() {
			auto value = m_memory.get(HL().word);
			compare(value);
		}

		void cpi() { compare(fetchByte()); }

		// rotate

		void rlc() {
			auto carry = A() & 0x80;
			A() <<= 1;
			A() |= carry >> 7;
			F().C = carry != 0;
		}

		void rrc() {
			auto carry = A() & 1;
			A() >>= 1;
			A() |= carry << 7;
			F().C = carry != 0;
		}

		void ral() {
			auto carry = A() & 0x80;
			A() <<= 1;
			A() |= (uint8_t)F().C;
			F().C = carry != 0;
		}

		void rar() {
			auto carry = A() & 1;
			A() >>= 1;
			A() |= F().C << 7;
			F().C = carry != 0;
		}

		// specials

		void cma() { A() ^= 0xff; }
		void stc() { F().C = true; }
		void cmc() { F().C = !F().C; }

		void daa() {
			auto carry = F().C;
			uint8_t addition = 0;
			if (F().AC || lowNibble(A()) > 9) {
				addition = 0x6;
			}
			if (F().C || highNibble(A()) > 9 || (highNibble(A()) >= 9 && lowNibble(A()) > 9)) {
				addition |= 0x60;
				carry = true;
			}
			add(addition);
			F().C = carry;
		}

		// input/output

		void out() { m_ports.write(fetchByte(), A()); }
		void in() { A() = m_ports.read(fetchByte()); }

		// control

		void ei() { enableInterrupts(); }
		void di() { disableInterrupts(); }

		void nop() {}

		void hlt() { m_halted = true; }
	};
}
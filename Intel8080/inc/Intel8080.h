#pragma once

// Auxiliary carry logic from https://github.com/begoon/i8080-core

#include "IntelProcessor.h"
#include "InputOutput.h"

namespace EightBit {
	class Intel8080 : public IntelProcessor {
	public:
		typedef std::function<void()> instruction_t;

		enum StatusBits {
			SF = Bit7,
			ZF = Bit6,
			AC = Bit4,
			PF = Bit2,
			CF = Bit0,
		};

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

		register16_t& AF() { return af; }
		uint8_t& A() { return AF().high; }
		uint8_t& F() { return AF().low; }

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

		int interrupt(uint8_t value) {
			if (isInterruptable()) {
				di();
				return execute(value);
			}
			return 0;
		}

		virtual void initialise();
		int step();

	private:
		InputOutput& m_ports;

		std::array<Instruction, 0x100> instructions;

		register16_t af;
		register16_t bc;
		register16_t de;
		register16_t hl;

		bool m_interrupt;

		void clearFlag(int flag) { F() &= ~flag; }
		void setFlag(int flag) { F() |= flag; }

		void setFlag(int flag, int condition) { setFlag(flag, condition != 0); }
		void setFlag(int flag, uint32_t condition) { setFlag(flag, condition != 0); }
		void setFlag(int flag, bool condition) { condition ? setFlag(flag) : clearFlag(flag); }

		void clearFlag(int flag, int condition) { clearFlag(flag, condition != 0); }
		void clearFlag(int flag, uint32_t condition) { clearFlag(flag, condition != 0); }
		void clearFlag(int flag, bool condition) { condition ? clearFlag(flag) : setFlag(flag); }

		int execute(uint8_t opcode);

		int execute(const Instruction& instruction) {
			cycles = 0;
			instruction.vector();
			return cycles + instruction.count;
		}

		void adjustReservedFlags() {
			AF().low &= ~(Bit5 | Bit3);
			AF().low |= Bit1;
		}

		void adjustSign(uint8_t value) { setFlag(SF, value & SF); }
		void adjustZero(uint8_t value) { clearFlag(ZF, value);}

		void adjustParity(uint8_t value) {
			static const uint8_t lookup[0x10] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
			auto set = (lookup[highNibble(value)] + lookup[lowNibble(value)]);
			clearFlag(PF, set % 2);
		}

		void adjustSZP(uint8_t value) {
			adjustSign(value);
			adjustZero(value);
			adjustParity(value);
		}

		void adjustAuxiliaryCarryAdd(uint8_t value, int calculation) {
			setFlag(AC, calculateHalfCarryAdd(A(), value, calculation));
		}

		void adjustAuxiliaryCarrySub(uint8_t value, int calculation) {
			clearFlag(AC, calculateHalfCarrySub(A(), value, calculation));
		}

		void postIncrement(uint8_t value) {
			adjustSZP(value);
			clearFlag(AC, lowNibble(value));
		}

		void postDecrement(uint8_t value) {
			adjustSZP(value);
			setFlag(AC, lowNibble(value) != Mask4);
		}

		static Instruction INS(instruction_t method, AddressingMode mode, std::string disassembly, int cycles);
		Instruction UNKNOWN();

		void installInstructions();

		//

		void compare(uint8_t value) {
			uint16_t subtraction = A() - value;
			adjustSZP((uint8_t)subtraction);
			adjustAuxiliaryCarrySub(value, subtraction);
			setFlag(CF, subtraction & Bit8);
		}

		void anda(uint8_t value) {
			setFlag(AC, (A() | value) & Bit3);
			clearFlag(CF);
			adjustSZP(A() &= value);
		}

		void ora(uint8_t value) {
			clearFlag(AC | CF);
			adjustSZP(A() |= value);
		}

		void xra(uint8_t value) {
			clearFlag(AC | CF);
			adjustSZP(A() ^= value);
		}

		void add(uint8_t value, int carry = 0) {
			register16_t sum;
			sum.word = A() + value + carry;
			adjustAuxiliaryCarryAdd(value, sum.word);
			A() = sum.low;
			setFlag(CF, sum.word & Bit8);
			adjustSZP(A());
		}

		void adc(uint8_t value) {
			add(value, F() & CF);
		}

		void dad(uint16_t value) {
			uint32_t sum = HL().word + value;
			setFlag(CF, sum > 0xffff);
			HL().word = (uint16_t)sum;
		}

		void sub(uint8_t value, int carry = 0) {
			register16_t difference;
			difference.word = A() - value - carry;
			adjustAuxiliaryCarrySub(value, difference.word);
			A() = difference.low;
			setFlag(CF, difference.word & Bit8);
			adjustSZP(A());
		}

		void sbb(uint8_t value) {
			sub(value, F() & CF);
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
			fetchWord();
			m_memory.set(MEMPTR().word, A());
		}

		void lda() {
			fetchWord();
			A() = m_memory.get(MEMPTR().word);
		}

		void shld() {
			fetchWord();
			m_memory.setWord(MEMPTR().word, HL());
		}

		void lhld() {
			fetchWord();
			HL() = m_memory.getWord(MEMPTR().word);
		}

		void xchg() {
			std::swap(DE(), HL());
		}

		// stack ops

		void push_b() { pushWord(BC()); }
		void push_d() { pushWord(DE()); }
		void push_h() { pushWord(HL()); }
		void push_psw() { pushWord(AF()); }

		void pop_b() { popWord(BC()); }
		void pop_d() { popWord(DE()); }
		void pop_h() { popWord(HL()); }

		void pop_psw() {
			popWord(AF());
			adjustReservedFlags();
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

		void jmp() { jumpConditional(true); }

		void jc() { jumpConditional(F() & CF); }
		void jnc() { jumpConditional(!(F() & CF)); }

		void jz() { jumpConditional(F() & ZF); }
		void jnz() { jumpConditional(!(F() & ZF)); }

		void jpe() { jumpConditional(F() & PF); }
		void jpo() { jumpConditional(!(F() & PF)); }

		void jm() { jumpConditional(F() & SF); }
		void jp() { jumpConditional(!(F() & SF)); }

		void pchl() {
			pc = HL();
		}

		// call

		void callDirect() {
			fetchWord();
			call();
		}

		void cc() { if (callConditional(F() & CF)) cycles += 6; }
		void cnc() { if (callConditional(!(F() & CF))) cycles += 6; }

		void cpe() { if (callConditional(F() & PF)) cycles += 6; }
		void cpo() { if (callConditional(!(F() & PF))) cycles += 6; }

		void cz() { if (callConditional(F() & ZF)) cycles += 6; }
		void cnz() { if (callConditional(!(F() & ZF))) cycles += 6; }

		void cm() { if (callConditional(F() & SF)) cycles += 6; }
		void cp() { if (callConditional(!(F() & SF))) cycles += 6; }

		// return

		void rc() { if (returnConditional(F() & CF)) cycles += 6; }
		void rnc() { if (returnConditional(!(F() & CF))) cycles += 6; }

		void rz() { if (returnConditional(F() & ZF)) cycles += 6; }
		void rnz() { if (returnConditional(!(F() & ZF))) cycles += 6; }

		void rpe() { if (returnConditional(F() & PF)) cycles += 6; }
		void rpo() { if (returnConditional(!(F() & PF))) cycles += 6; }

		void rm() { if (returnConditional(F() & SF)) cycles += 6; }
		void rp() { if (returnConditional(!(F() & SF))) cycles += 6; }

		// restart

		void rst_0() { restart(0 << 3); }
		void rst_1() { restart(1 << 3); }
		void rst_2() { restart(2 << 3); }
		void rst_3() { restart(3 << 3); }
		void rst_4() { restart(4 << 3); }
		void rst_5() { restart(5 << 3); }
		void rst_6() { restart(6 << 3); }
		void rst_7() { restart(7 << 3); }

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
			auto carry = A() & Bit7;
			A() <<= 1;
			carry ? A() |= Bit0 : A() &= ~Bit0;
			setFlag(CF, carry);
		}

		void rrc() {
			auto carry = A() & Bit0;
			A() >>= 1;
			carry ? A() |= Bit7 : A() &= ~Bit7;
			setFlag(CF, carry);
		}

		void ral() {
			auto carry = A() & Bit7;
			A() <<= 1;
			A() |= (F() & CF);
			setFlag(CF, carry);
		}

		void rar() {
			auto carry = A() & 1;
			A() >>= 1;
			A() |= (F() & CF) << 7;
			setFlag(CF, carry);
		}

		// specials

		void cma() { A() ^= Mask8; }
		void stc() { setFlag(CF); }
		void cmc() { clearFlag(CF, F() & CF); }

		void daa() {
			auto carry = F() & CF;
			uint8_t addition = 0;
			if ((F() & AC) || lowNibble(A()) > 9) {
				addition = 0x6;
			}
			if ((F() & CF) || highNibble(A()) > 9 || (highNibble(A()) >= 9 && lowNibble(A()) > 9)) {
				addition |= 0x60;
				carry = true;
			}
			add(addition);
			setFlag(CF, carry);
		}

		// input/output

		void out() { m_ports.write(fetchByte(), A()); }
		void in() { A() = m_ports.read(fetchByte()); }

		// control

		void ei() { m_interrupt = true; }
		void di() { m_interrupt = false; }

		void nop() {}

		void hlt() { m_halted = true; }
	};
}
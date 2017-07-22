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

		virtual register16_t& AF() override { return af; }
		virtual register16_t& BC() override { return bc; }
		virtual register16_t& DE() override { return de; }
		virtual register16_t& HL() override { return hl; }

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

		int execute(uint8_t opcode);

		int execute(const Instruction& instruction) {
			cycles = 0;
			instruction.vector();
			return cycles + instruction.count;
		}

		void adjustReservedFlags() {
			F() = (F() | Bit1) & ~(Bit5 | Bit3);
		}

		static void adjustAuxiliaryCarryAdd(uint8_t& f, uint8_t before, uint8_t value, int calculation) {
			setFlag(f, AC, calculateHalfCarryAdd(before, value, calculation));
		}

		static void adjustAuxiliaryCarrySub(uint8_t& f, uint8_t before, uint8_t value, int calculation) {
			clearFlag(f, AC, calculateHalfCarrySub(before, value, calculation));
		}

		static void increment(uint8_t& f, uint8_t& operand) {
			adjustSZP<Intel8080>(f, ++operand);
			clearFlag(f, AC, lowNibble(operand));
		}

		static void decrement(uint8_t& f, uint8_t& operand) {
			adjustSZP<Intel8080>(f, --operand);
			setFlag(f, AC, lowNibble(operand) != Mask4);
		}

		static Instruction INS(instruction_t method, AddressingMode mode, std::string disassembly, int cycles);
		Instruction UNKNOWN();

		void installInstructions();

		//

		void compare(uint8_t& f, uint8_t check, uint8_t value) {
			subtract(f, check, value);
		}

		void anda(uint8_t value) {
			auto& a = A();
			auto& f = F();
			setFlag(f, AC, (a | value) & Bit3);
			clearFlag(f, CF);
			adjustSZP<Intel8080>(f, a &= value);
		}

		void ora(uint8_t value) {
			auto& f = F();
			clearFlag(f, AC | CF);
			adjustSZP<Intel8080>(f, A() |= value);
		}

		void xra(uint8_t value) {
			auto& f = F();
			clearFlag(f, AC | CF);
			adjustSZP<Intel8080>(f, A() ^= value);
		}

		void add(uint8_t value, int carry = 0) {
			auto& a = A();
			auto& f = F();
			register16_t sum;
			sum.word = a + value + carry;
			adjustAuxiliaryCarryAdd(f, a, value, sum.word);
			a = sum.low;
			setFlag(f, CF, sum.word & Bit8);
			adjustSZP<Intel8080>(f, a);
		}

		void adc(uint8_t value) {
			add(value, F() & CF);
		}

		void dad(uint16_t value) {
			auto& f = F();
			auto sum = HL().word + value;
			setFlag(f, CF, sum & Bit16);
			HL().word = sum;
		}

		void subtract(uint8_t& f, uint8_t& operand, uint8_t value, int carry = 0) {

			register16_t result;
			result.word = operand - value - carry;

			adjustAuxiliaryCarrySub(f, operand, value, result.word);

			operand = result.low;

			setFlag(f, CF, result.word & Bit8);
			adjustSZP<Intel8080>(f, operand);
		}

		void sbb(uint8_t value) {
			subtract(F(), A(), value, F() & CF);
		}

		void mov_m_r(uint8_t value) {
			m_memory.ADDRESS() = HL();
			m_memory.reference() = value;
		}

		uint8_t mov_r_m() {
			m_memory.ADDRESS() = HL();
			return m_memory.reference();
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
			m_memory.ADDRESS() = HL();
			m_memory.reference() = data;
		}

		void lxi_b() { fetchWord(BC()); }
		void lxi_d() { fetchWord(DE()); }
		void lxi_h() { fetchWord(HL()); }

		void stax_r(register16_t& destination) {
			m_memory.ADDRESS() = destination;
			m_memory.reference() = A();
		}

		void stax_b() { stax_r(BC()); }
		void stax_d() { stax_r(DE()); }

		void ldax_r(register16_t& source) {
			m_memory.ADDRESS() = source;
			A() = m_memory.reference();
		}

		void ldax_b() { ldax_r(BC()); }
		void ldax_d() { ldax_r(DE()); }

		void sta() {
			fetchWord();
			memptrReference() = A();
		}

		void lda() {
			fetchWord();
			A() = memptrReference();
		}

		void shld() {
			fetchWord();
			setWordViaMemptr(HL());
		}

		void lhld() {
			fetchWord();
			getWordViaMemptr(HL());
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
			m_memory.ADDRESS() = SP();
			MEMPTR().low = m_memory.reference();
			m_memory.reference() = L();
			L() = MEMPTR().low;
			m_memory.ADDRESS().word++;
			MEMPTR().high = m_memory.reference();
			m_memory.reference() = H();
			H() = MEMPTR().high;
		}

		void sphl() {
			SP() = HL();
		}

		void lxi_sp() {
			fetchWord(SP());
		}

		void inx_sp() { ++SP().word; }
		void dcx_sp() { --SP().word; }

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
			PC() = HL();
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

		void inr_a() { increment(F(), A()); }
		void inr_b() { increment(F(), B()); }
		void inr_c() { increment(F(), C()); }
		void inr_d() { increment(F(), D()); }
		void inr_e() { increment(F(), E()); }
		void inr_h() { increment(F(), H()); }
		void inr_l() { increment(F(), L()); }

		void inr_m() {
			m_memory.ADDRESS() = HL();
			increment(F(), m_memory.reference());
		}

		void dcr_a() { decrement(F(), A()); }
		void dcr_b() { decrement(F(), B()); }
		void dcr_c() { decrement(F(), C()); }
		void dcr_d() { decrement(F(), D()); }
		void dcr_e() { decrement(F(), E()); }
		void dcr_h() { decrement(F(), H()); }
		void dcr_l() { decrement(F(), L()); }

		void dcr_m() {
			m_memory.ADDRESS() = HL();
			decrement(F(), m_memory.reference());
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
			m_memory.ADDRESS() = HL();
			add(m_memory.reference());
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
			m_memory.ADDRESS() = HL();
			adc(m_memory.reference());
		}

		void aci() { adc(fetchByte()); }

		void dad_b() { dad(BC().word); }
		void dad_d() { dad(DE().word); }
		void dad_h() { dad(HL().word); }
		void dad_sp() { dad(SP().word); }

		// subtract

		void sub_a() { subtract(F(), A(), A()); }
		void sub_b() { subtract(F(), A(), B()); }
		void sub_c() { subtract(F(), A(), C()); }
		void sub_d() { subtract(F(), A(), D()); }
		void sub_e() { subtract(F(), A(), E()); }
		void sub_h() { subtract(F(), A(), H()); }
		void sub_l() { subtract(F(), A(), L()); }

		void sub_m() {
			m_memory.ADDRESS() = HL();
			subtract(F(), A(), m_memory.reference());
		}

		void sbb_a() { sbb(A()); }
		void sbb_b() { sbb(B()); }
		void sbb_c() { sbb(C()); }
		void sbb_d() { sbb(D()); }
		void sbb_e() { sbb(E()); }
		void sbb_h() { sbb(H()); }
		void sbb_l() { sbb(L()); }

		void sbb_m() {
			m_memory.ADDRESS() = HL();
			sbb(m_memory.reference());
		}

		void sbi() {
			sbb(fetchByte());
		}

		void sui() {
			subtract(F(), A(), fetchByte());
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
			m_memory.ADDRESS() = HL();
			anda(m_memory.reference());
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
			m_memory.ADDRESS() = HL();
			xra(m_memory.reference());
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
			m_memory.ADDRESS() = HL();
			ora(m_memory.reference());
		}

		void ori() { ora(fetchByte()); }

		void cmp_a() { compare(F(), A(), A()); }
		void cmp_b() { compare(F(), A(), B()); }
		void cmp_c() { compare(F(), A(), C()); }
		void cmp_d() { compare(F(), A(), D()); }
		void cmp_e() { compare(F(), A(), E()); }
		void cmp_h() { compare(F(), A(), H()); }
		void cmp_l() { compare(F(), A(), L()); }

		void cmp_m() {
			m_memory.ADDRESS() = HL();
			compare(F(), A(), m_memory.reference());
		}

		void cpi() { compare(F(), A(), fetchByte()); }

		// rotate

		void rlc() {
			auto& a = A();
			auto carry = a & Bit7;
			a = (a << 1) | (carry >> 7);
			setFlag(F(), CF, carry);
		}

		void rrc() {
			auto& a = A();
			auto carry = a & Bit0;
			a = (a >> 1) | (carry << 7);
			setFlag(F(), CF, carry);
		}

		void ral() {
			auto& a = A();
			auto& f = F();
			const auto carry = f & CF;
			setFlag(f, CF, a & Bit7);
			a = (a << 1) | carry;
		}

		void rar() {
			auto& a = A();
			auto& f = F();
			const auto carry = f & CF;
			setFlag(f, CF, a & Bit0);
			a = (a >> 1) | (carry << 7);
		}

		// specials

		void cma() { A() ^= Mask8; }
		void stc() { setFlag(F(), CF); }
		void cmc() { clearFlag(F(), CF, F() & CF); }

		void daa() {
			const auto& a = A();
			auto& f = F();
			auto carry = f & CF;
			uint8_t addition = 0;
			if ((f & AC) || lowNibble(a) > 9) {
				addition = 0x6;
			}
			if ((f & CF) || highNibble(a) > 9 || (highNibble(a) >= 9 && lowNibble(a) > 9)) {
				addition |= 0x60;
				carry = true;
			}
			add(addition);
			setFlag(f, CF, carry);
		}

		// input/output

		void out() { m_ports.write(fetchByte(), A()); }
		void in() { A() = m_ports.read(fetchByte()); }

		// control

		void ei() { m_interrupt = true; }
		void di() { m_interrupt = false; }

		void nop() {}

		void hlt() { halt(); }
	};
}
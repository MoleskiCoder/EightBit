#include "stdafx.h"
#include "LR35902.h"

// based on http://www.z80.info/decoding.htm
// Half carry flag help from https://github.com/oubiwann/z80

EightBit::LR35902::LR35902(Bus& memory)
: Processor(memory),
  m_ime(false),
  m_prefixCB(false) {
	MEMPTR().word = 0;
}

void EightBit::LR35902::reset() {
	Processor::reset();
	sp.word = 0xfffe;
	di();
}

void EightBit::LR35902::initialise() {

	Processor::initialise();

	AF().word = 0xffff;
	BC().word = 0xffff;
	DE().word = 0xffff;
	HL().word = 0xffff;

	MEMPTR().word = 0;

	m_prefixCB = false;
}

#pragma region Interrupt routines

void EightBit::LR35902::di() {
	IME() = false;
}

void EightBit::LR35902::ei() {
	IME() = true;
}

int EightBit::LR35902::interrupt(uint8_t value) {
	cycles = 0;
	di();
	restart(value);
	return 4;
}

#pragma endregion Interrupt routines

#pragma region Flag manipulation helpers

void EightBit::LR35902::adjustZero(uint8_t value) {
	clearFlag(ZF, value);
}

void EightBit::LR35902::postIncrement(uint8_t value) {
	adjustZero(value);
	clearFlag(NF);
	clearFlag(HC, lowNibble(value));
}

void EightBit::LR35902::postDecrement(uint8_t value) {
	adjustZero(value);
	setFlag(NF);
	clearFlag(HC, lowNibble(value + 1));
}

#pragma endregion Flag manipulation helpers

#pragma region PC manipulation: call/ret/jp/jr

void EightBit::LR35902::restart(uint8_t address) {
	pushWord(pc);
	pc.low = address;
	pc.high = 0;
}

void EightBit::LR35902::jrConditional(int conditional) {
	auto offset = (int8_t)fetchByte();
	if (conditional) {
		pc.word += offset;
		cycles++;
	}
}

void EightBit::LR35902::jrConditionalFlag(int flag) {
	switch (flag) {
	case 0:	// NZ
		jrConditional(!(F() & ZF));
		break;
	case 1:	// Z
		jrConditional(F() & ZF);
		break;
	case 2:	// NC
		jrConditional(!(F() & CF));
		break;
	case 3:	// C
		jrConditional(F() & CF);
		break;
	case 4:	// PO
	case 5:	// PE
	case 6:	// P
	case 7:	// M
		cycles -= 2;
		break;
	}
}

void EightBit::LR35902::jumpConditional(int conditional) {
	if (conditional)
		pc = MEMPTR();
}

void EightBit::LR35902::jumpConditionalFlag(int flag) {
	switch (flag) {
	case 0:	// NZ
		jumpConditional(!(F() & ZF));
		break;
	case 1:	// Z
		jumpConditional(F() & ZF);
		break;
	case 2:	// NC
		jumpConditional(!(F() & CF));
		break;
	case 3:	// C
		jumpConditional(F() & CF);
		break;
	case 4:	// GB: LD (FF00 + C),A
		m_memory.set(0xff00 + C(), A());
		cycles--; // Giving 8 cycles
		break;
	case 5:	// GB: LD (nn),A
		fetchWord();
		m_memory.ADDRESS() = MEMPTR();
		m_memory.reference() = A();
		cycles++; // Giving 16 cycles
		break;
	case 6:	// GB: LD A,(FF00 + C)
		m_memory.ADDRESS().word = 0xff00 + C();
		A() = m_memory.reference();
		cycles--; // 8 cycles
		break;
	case 7:	// GB: LD A,(nn)
		fetchWord();
		m_memory.ADDRESS() = MEMPTR();
		A() = m_memory.reference();
		cycles++; // Giving 16 cycles
		break;
	}
}

void EightBit::LR35902::ret() {
	popWord(MEMPTR());
	pc = MEMPTR();
}

void EightBit::LR35902::reti() {
	ret();
	ei();
}

void EightBit::LR35902::returnConditional(int condition) {
	if (condition) {
		ret();
		cycles += 3;
	}
}

void EightBit::LR35902::returnConditionalFlag(int flag) {
	switch (flag) {
	case 0:	// NZ
		returnConditional(!(F() & ZF));
		break;
	case 1:	// Z
		returnConditional(F() & ZF);
		break;
	case 2:	// NC
		returnConditional(!(F() & CF));
		break;
	case 3:	// C
		returnConditional(F() & CF);
		break;
	case 4:	// GB: LD (FF00 + n),A
		m_memory.set(0xff00 + fetchByte(), A());
		cycles++; // giving 12 cycles in total
		break;
	case 5: { // GB: ADD SP,dd
			auto before = sp;
			auto value = fetchByte();
			sp.word += (int8_t)value;
			clearFlag(ZF | NF);
			setFlag(CF, sp.word & Bit16);
			adjustHalfCarryAdd(before.high, value, sp.high);
		}
		cycles += 2;	// 16 cycles
		break;
	case 6:	// GB: LD A,(FF00 + n)
		A() = m_memory.get(0xff00 + fetchByte());
		cycles++;	// 12 cycles
		break;
	case 7: { // GB: LD HL,SP + dd
			auto before = sp;
			auto value = fetchByte();
			HL().word = before.word + (int8_t)value;
			clearFlag(ZF | NF);
			setFlag(CF, HL().word & Bit16);
			adjustHalfCarryAdd(before.high, value, HL().high);
		}
		cycles++;	// 12 cycles
		break;
	}
}

void EightBit::LR35902::call() {
	pushWord(pc);
	pc = MEMPTR();
}

void EightBit::LR35902::callConditional(int condition) {
	if (condition) {
		call();
		cycles += 3;
	}
}

void EightBit::LR35902::callConditionalFlag(int flag) {
	switch (flag) {
	case 0:	// NZ
		callConditional(!(F() & ZF));
		break;
	case 1:	// Z
		callConditional(F() & ZF);
		break;
	case 2:	// NC
		callConditional(!(F() & CF));
		break;
	case 3:	// C
		callConditional(F() & CF);
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		cycles -= 3; // removed from GB
		break;
	}
}

#pragma endregion PC manipulation: call/ret/jp/jr

#pragma region 16-bit arithmetic

void EightBit::LR35902::sbc(register16_t& operand, register16_t value) {

	auto before = operand;

	auto result = before.word - value.word - (F() & CF);
	operand.word = result;

	clearFlag(ZF, operand.word);
	adjustHalfCarrySub(before.high, value.high, operand.high);
	setFlag(NF);
	setFlag(CF, result & Bit16);
}

void EightBit::LR35902::adc(register16_t& operand, register16_t value) {

	auto before = operand;

	auto result = before.word + value.word + (F() & CF);
	operand.word = result;

	clearFlag(ZF, result);
	adjustHalfCarryAdd(before.high, value.high, operand.high);
	clearFlag(NF);
	setFlag(CF, result & Bit16);
}

void EightBit::LR35902::add(register16_t& operand, register16_t value) {

	auto before = operand;

	auto result = before.word + value.word;

	operand.word = result;

	clearFlag(NF);
	setFlag(CF, result & Bit16);
	adjustHalfCarryAdd(before.high, value.high, operand.high);
}

#pragma endregion 16-bit arithmetic

#pragma region ALU

void EightBit::LR35902::add(uint8_t& operand, uint8_t value, int carry) {

	register16_t result;
	result.word = operand + value + carry;

	adjustHalfCarryAdd(operand, value, result.low);

	operand = result.low;

	clearFlag(NF);
	setFlag(CF, result.word & Bit8);
	adjustZero(operand);
}

void EightBit::LR35902::adc(uint8_t& operand, uint8_t value) {
	add(operand, value, F() & CF);
}

void EightBit::LR35902::sub(uint8_t& operand, uint8_t value, int carry) {

	register16_t result;
	result.word = operand - value - carry;

	adjustHalfCarrySub(operand, value, result.low);

	operand = result.low;

	setFlag(NF);
	setFlag(CF, result.word & Bit8);
	adjustZero(operand);
}

void EightBit::LR35902::sbc(uint8_t& operand, uint8_t value) {
	sub(operand, value, F() & CF);
}

void EightBit::LR35902::andr(uint8_t& operand, uint8_t value) {
	operand &= value;
	setFlag(HC);
	clearFlag(CF | NF);
	adjustZero(operand);
}

void EightBit::LR35902::xorr(uint8_t& operand, uint8_t value) {
	operand ^= value;
	clearFlag(HC | CF | NF);
	adjustZero(operand);
}

void EightBit::LR35902::orr(uint8_t& operand, uint8_t value) {
	operand |= value;
	clearFlag(HC | CF | NF);
	adjustZero(operand);
}

void EightBit::LR35902::compare(uint8_t value) {
	auto check = A();
	sub(check, value);
}

#pragma endregion ALU

#pragma region Shift and rotate

void EightBit::LR35902::rlc(uint8_t& operand) {
	auto carry = operand & Bit7;
	operand <<= 1;
	setFlag(CF, carry);
	carry ? operand |= Bit0 : operand &= ~Bit0;
	clearFlag(NF | HC);
	adjustZero(operand);
}

void EightBit::LR35902::rrc(uint8_t& operand) {
	auto carry = operand & Bit0;
	operand >>= 1;
	carry ? operand |= Bit7 : operand &= ~Bit7;
	setFlag(CF, carry);
	clearFlag(NF | HC);
	adjustZero(operand);
}

void EightBit::LR35902::rl(uint8_t& operand) {
	auto oldCarry = F() & CF;
	auto newCarry = operand & Bit7;
	operand <<= 1;
	oldCarry ? operand |= Bit0 : operand &= ~Bit0;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustZero(operand);
}

void EightBit::LR35902::rr(uint8_t& operand) {
	auto oldCarry = F() & CF;
	auto newCarry = operand & Bit0;
	operand >>= 1;
	operand |= oldCarry << 7;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustZero(operand);
}

//

void EightBit::LR35902::sla(uint8_t& operand) {
	auto newCarry = operand & Bit7;
	operand <<= 1;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustZero(operand);
}

void EightBit::LR35902::sra(uint8_t& operand) {
	auto new7 = operand & Bit7;
	auto newCarry = operand & Bit0;
	operand >>= 1;
	operand |= new7;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustZero(operand);
}

void EightBit::LR35902::srl(uint8_t& operand) {
	auto newCarry = operand & Bit0;
	operand >>= 1;
	operand &= ~Bit7;	// clear bit 7
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustZero(operand);
}

//

void EightBit::LR35902::rlca() {
	rlc(A());
}

void EightBit::LR35902::rrca() {
	rrc(A());
}

void EightBit::LR35902::rla() {
	rl(A());
}

void EightBit::LR35902::rra() {
	rr(A());
}

#pragma endregion Shift and rotate

#pragma region BIT/SET/RES

void EightBit::LR35902::bit(int n, uint8_t& operand) {
	auto carry = F() & CF;
	uint8_t discarded = operand;
	andr(discarded, 1 << n);
	setFlag(CF, carry);
}

void EightBit::LR35902::res(int n, uint8_t& operand) {
	auto bit = 1 << n;
	operand &= ~bit;
}

void EightBit::LR35902::set(int n, uint8_t& operand) {
	auto bit = 1 << n;
	operand |= bit;
}

#pragma endregion BIT/SET/RES

#pragma region Miscellaneous instructions

void EightBit::LR35902::daa() {

	uint8_t a = A();

	auto lowAdjust = (F() & HC) | ((A() & 0xf) > 9);
	auto highAdjust = (F() & CF) | (A() > 0x99);

	if (F() & NF) {
		if (lowAdjust)
			a -= 6;
		if (highAdjust)
			a -= 0x60;
	} else {
		if (lowAdjust)
			a += 6;
		if (highAdjust)
			a += 0x60;
	}

	F() = (F() & (CF | NF)) | (A() > 0x99) | ((A() ^ a) & HC);

	adjustZero(a);

	A() = a;
}

void EightBit::LR35902::cpl() {
	A() = ~A();
	setFlag(HC | NF);
}

void EightBit::LR35902::scf() {
	setFlag(CF);
	clearFlag(HC | NF);
}

void EightBit::LR35902::ccf() {
	auto carry = F() & CF;
	clearFlag(CF, carry);
	clearFlag(NF | HC);
}

void EightBit::LR35902::swap(uint8_t& operand) {
	auto low = lowNibble(operand);
	auto high = highNibble(operand);
	operand = promoteNibble(low) | demoteNibble(high);
	adjustZero(operand);
	clearFlag(NF | HC | CF);
}

#pragma endregion Miscellaneous instructions

int EightBit::LR35902::step() {
	ExecutingInstruction.fire(*this);
	m_prefixCB = false;
	cycles = 0;
	return fetchExecute();
}

int EightBit::LR35902::execute(uint8_t opcode) {

	auto x = (opcode & 0b11000000) >> 6;
	auto y = (opcode & 0b111000) >> 3;
	auto z = (opcode & 0b111);

	auto p = (y & 0b110) >> 1;
	auto q = (y & 1);

	if (m_prefixCB)
		executeCB(x, y, z, p, q);
	else
		executeOther(x, y, z, p, q);

	if (cycles == 0)
		throw std::logic_error("Unhandled opcode");

	return cycles * 4;
}

void EightBit::LR35902::executeCB(int x, int y, int z, int p, int q) {
	switch (x) {
	case 0:	// rot[y] r[z]
		switch (y) {
		case 0:
			rlc(R(z));
			break;
		case 1:
			rrc(R(z));
			break;
		case 2:
			rl(R(z));
			break;
		case 3:
			rr(R(z));
			break;
		case 4:
			sla(R(z));
			break;
		case 5:
			sra(R(z));
			break;
		case 6:
			swap(R(z));
			break;
		case 7:
			srl(R(z));
			break;
		}
		adjustZero(R(z));
		cycles += 2;
		if (z == 6)
			cycles += 2;
		break;
	case 1: // BIT y, r[z]
			bit(y, R(z));
			cycles += 2;
			if (z == 6)
				cycles += 2;
		break;
	case 2:	// RES y, r[z]
		res(y, R(z));
		cycles += 2;
		if (z == 6)
			cycles += 2;
		break;
	case 3:	// SET y, r[z]
		set(y, R(z));
		cycles += 2;
		if (z == 6)
			cycles += 2;
		break;
	}
}

void EightBit::LR35902::executeOther(int x, int y, int z, int p, int q) {
	switch (x) {
	case 0:
		switch (z) {
		case 0:	// Relative jumps and assorted ops
			switch (y) {
			case 0:	// NOP
				cycles++;
				break;
			case 1:	// GB: LD (nn),SP
				fetchWord();
				m_memory.setWord(MEMPTR().word, sp);
				cycles += 5;
				break;
			case 2:	// GB: STOP
				stop();
				cycles++;
				break;
			case 3:	// JR d
				jrConditional(true);
				cycles += 3;
				break;
			default:	// JR cc,d
				jrConditionalFlag(y - 4);
				cycles += 2;
				break;
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0:	// LD rp,nn
				fetchWord();
				RP(p) = MEMPTR();
				cycles += 3;
				break;
			case 1:	// ADD HL,rp
				add(HL(), RP(p));
				cycles += 2;
				break;
			}
			break;
		case 2:	// Indirect loading
			switch (q) {
			case 0:
				switch (p) {
				case 0:	// LD (BC),A
					m_memory.set(BC().word, A());
					cycles += 2;
					break;
				case 1:	// LD (DE),A
					m_memory.set(DE().word, A());
					cycles += 2;
					break;
				case 2:	// GB: LDI (HL),A
					m_memory.set(HL().word++, A());
					cycles += 2;
					break;
				case 3: // GB: LDD (HL),A
					m_memory.set(HL().word--, A());
					cycles += 2;
					break;
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					A() = m_memory.get(BC().word);
					cycles += 2;
					break;
				case 1:	// LD A,(DE)
					A() = m_memory.get(DE().word);
					cycles += 2;
					break;
				case 2:	// GB: LDI A,(HL)
					A() = m_memory.get(HL().word++);
					cycles += 2;
					break;
				case 3:	// GB: LDD A,(HL)
					A() = m_memory.get(HL().word--);
					cycles += 2;
					break;
				}
				break;
			}
			break;
		case 3:	// 16-bit INC/DEC
			switch (q) {
			case 0:	// INC rp
				++RP(p).word;
				break;
			case 1:	// DEC rp
				--RP(p).word;
				break;
			}
			cycles += 2;
			break;
		case 4:	// 8-bit INC
			postIncrement(++R(y));	// INC r
			cycles++;
			if (y == 6)
				cycles += 2;
			break;
		case 5:	// 8-bit DEC
			postDecrement(--R(y));	// DEC r
			cycles++;
			if (y == 6)
				cycles += 2;
			break;
		case 6: // 8-bit load immediate
			R(y) = fetchByte();
			cycles += 2;
			break;
		case 7:	// Assorted operations on accumulator/flags
			switch (y) {
			case 0:
				rlca();
				break;
			case 1:
				rrca();
				break;
			case 2:
				rla();
				break;
			case 3:
				rra();
				break;
			case 4:
				daa();
				break;
			case 5:
				cpl();
				break;
			case 6:
				scf();
				break;
			case 7:
				ccf();
				break;
			}
			cycles++;
			break;
		}
		break;
	case 1:	// 8-bit loading
		if (z == 6 && y == 6) { 	// Exception (replaces LD (HL), (HL))
			halt();
		} else {
			R(y) = R(z);
			if ((y == 6) || (z == 6))	// M operations
				cycles++;
		}
		cycles++;
		break;
	case 2:	// Operate on accumulator and register/memory location
		switch (y) {
		case 0:	// ADD A,r
			add(A(), R(z));
			break;
		case 1:	// ADC A,r
			adc(A(), R(z));
			break;
		case 2:	// SUB r
			sub(A(), R(z));
			break;
		case 3:	// SBC A,r
			sbc(A(), R(z));
			break;
		case 4:	// AND r
			andr(A(), R(z));
			break;
		case 5:	// XOR r
			xorr(A(), R(z));
			break;
		case 6:	// OR r
			orr(A(), R(z));
			break;
		case 7:	// CP r
			compare(R(z));
			break;
		}
		cycles++;
		if (z == 6)
			cycles++;
		break;
	case 3:
		switch (z) {
		case 0:	// Conditional return
			returnConditionalFlag(y);
			cycles += 2;
			break;
		case 1:	// POP & various ops
			switch (q) {
			case 0:	// POP rp2[p]
				popWord(RP2(p));
				cycles += 3;
				break;
			case 1:
				switch (p) {
				case 0:	// RET
					ret();
					cycles += 4;
					break;
				case 1:	// GB: RETI
					reti();
					cycles += 4;
					break;
				case 2:	// JP HL
					pc = HL();
					cycles += 1;
					break;
				case 3:	// LD SP,HL
					sp = HL();
					cycles += 2;
					break;
				}
			}
			break;
		case 2:	// Conditional jump
			jumpConditionalFlag(y);
			cycles += 3;
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0:	// JP nn
				fetchWord();
				pc = MEMPTR();
				cycles += 4;
				break;
			case 1:	// CB prefix
				m_prefixCB = true;
				fetchExecute();
				break;
			case 6:	// DI
				di();
				cycles++;
				break;
			case 7:	// EI
				ei();
				cycles++;
				break;
			}
			break;
		case 4:	// Conditional call: CALL cc[y], nn
			fetchWord();
			callConditionalFlag(y);
			cycles += 3;
			break;
		case 5:	// PUSH & various ops
			switch (q) {
			case 0:	// PUSH rp2[p]
				pushWord(RP2(p));
				cycles += 4;
				break;
			case 1:
				switch (p) {
				case 0:	// CALL nn
					fetchWord();
					callConditional(true);
					cycles += 3;
					break;
				}
			}
			break;
		case 6:	// Operate on accumulator and immediate operand: alu[y] n
			switch (y) {
			case 0:	// ADD A,n
				add(A(), fetchByte());
				break;
			case 1:	// ADC A,n
				adc(A(), fetchByte());
				break;
			case 2:	// SUB n
				sub(A(), fetchByte());
				break;
			case 3:	// SBC A,n
				sbc(A(), fetchByte());
				break;
			case 4:	// AND n
				andr(A(), fetchByte());
				break;
			case 5:	// XOR n
				xorr(A(), fetchByte());
				break;
			case 6:	// OR n
				orr(A(), fetchByte());
				break;
			case 7:	// CP n
				compare(fetchByte());
				break;
			}
			cycles += 2;
			break;
		case 7:	// Restart: RST y * 8
			restart(y << 3);
			cycles += 4;
			break;
		}
		break;
	}
}
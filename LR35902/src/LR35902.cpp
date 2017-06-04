#include "stdafx.h"
#include "LR35902.h"

// based on http://www.z80.info/decoding.htm
// Half carry flag help from https://github.com/oubiwann/z80

LR35902::LR35902(Bus& memory)
: Processor(memory),
  m_ime(false),
  m_prefixCB(false) {
}

void LR35902::reset() {
	Processor::reset();
	setStackPointer(0xfffe);
	di();
}

void LR35902::initialise() {

	Processor::initialise();

	AF().word = 0xffff;
	BC().word = 0xffff;
	DE().word = 0xffff;
	HL().word = 0xffff;

	m_prefixCB = false;
}

void LR35902::di() {
	IME() = false;
}

void LR35902::ei() {
	IME() = true;
}

int LR35902::interrupt(uint8_t value) {
	di();
	restart(value);
	return 4;
}

void LR35902::adjustZero(uint8_t value) {
	clearFlag(ZF, value);
}

void LR35902::postIncrement(uint8_t value) {
	adjustZero(value);
	clearFlag(NF);
	clearFlag(HC, lowNibble(value));
}

void LR35902::postDecrement(uint8_t value) {
	adjustZero(value);
	setFlag(NF);
	clearFlag(HC, lowNibble(value + 1));
}

void LR35902::restart(uint8_t address) {
	pushWord(pc);
	pc = address;
}

void LR35902::jrConditional(int conditional) {
	auto offset = (int8_t)fetchByte();
	if (conditional) {
		pc += offset;
		cycles++;
	}
}

void LR35902::jrConditionalFlag(int flag) {
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

void LR35902::jumpConditional(int conditional) {
	auto address = fetchWord();
	if (conditional) {
		pc = address;
		cycles++;
	}
}

void LR35902::jumpConditionalFlag(int flag) {
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
		m_memory.set(fetchWord(), A());
		cycles++; // Giving 16 cycles
		break;
	case 6:	// GB: LD A,(FF00 + C)
		A() = m_memory.get(0xff00 + C());
		cycles--; // 8 cycles
		break;
	case 7:	// GB: LD A,(nn)
		A() = m_memory.get(fetchWord());
		cycles++; // Giving 16 cycles
		break;
	}
}

void LR35902::ret() {
	pc = popWord();
}

void LR35902::reti() {
	ret();
	ei();
}

void LR35902::returnConditional(int condition) {
	if (condition) {
		ret();
		cycles += 3;
	}
}

void LR35902::returnConditionalFlag(int flag) {
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
			sp += (int8_t)value;
			clearFlag(ZF | NF);
			setFlag(CF, sp & Bit16);
			adjustHalfCarryAdd(Memory::highByte(before), value, Memory::highByte(sp));
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
			uint16_t result = before + (int8_t)value;
			HL().word = result;
			clearFlag(ZF | NF);
			setFlag(CF, result & Bit16);
			adjustHalfCarryAdd(Memory::highByte(before), value, Memory::highByte(result));
		}
		cycles++;	// 12 cycles
		break;
	}
}

void LR35902::call(uint16_t address) {
	pushWord(pc + 2);
	pc = address;
}

void LR35902::callConditional(uint16_t address, int condition) {
	if (condition) {
		call(address);
		cycles += 3;
	} else {
		pc += 2;
	}
}

void LR35902::callConditionalFlag(uint16_t address, int flag) {
	switch (flag) {
	case 0:	// NZ
		callConditional(address, !(F() & ZF));
		break;
	case 1:	// Z
		callConditional(address, F() & ZF);
		break;
	case 2:	// NC
		callConditional(address, !(F() & CF));
		break;
	case 3:	// C
		callConditional(address, F() & CF);
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		cycles -= 3; // removed from GB
		break;
	}
}

uint16_t LR35902::sbc(uint16_t value) {

	auto hl = RP(HL_IDX);

	auto high = Memory::highByte(hl);
	auto highValue = Memory::highByte(value);
	auto applyCarry = F() & CF;

	uint32_t result = (int)hl - (int)value;
	if (applyCarry)
		--result;
	auto highResult = Memory::highByte(result);

	adjustZero(result);
	adjustHalfCarrySub(high, highValue, highResult);

	setFlag(NF);
	setFlag(CF, result & Bit16);

	return result;
}

uint16_t LR35902::adc(uint16_t value) {

	auto hl = RP(HL_IDX);

	auto high = Memory::highByte(hl);
	auto highValue = Memory::highByte(value);
	auto applyCarry = F() & CF;

	uint32_t result = (int)hl + (int)value;
	if (applyCarry)
		++result;
	auto highResult = Memory::highByte(result);

	adjustZero(result);
	adjustHalfCarryAdd(high, highValue, highResult);

	clearFlag(NF);
	setFlag(CF, result & Bit16);

	return result;
}

uint16_t LR35902::add(uint16_t value) {

	auto hl = RP(HL_IDX);

	auto high = Memory::highByte(hl);
	auto highValue = Memory::highByte(value);

	uint32_t result = (int)hl + (int)value;

	auto highResult = Memory::highByte(result);

	clearFlag(NF);
	setFlag(CF, result & Bit16);
	adjustHalfCarryAdd(high, highValue, highResult);

	return result;
}

void LR35902::sub(uint8_t& operand, uint8_t value, bool carry) {

	auto before = operand;

	uint16_t result = before - value;
	if (carry && (F() & CF))
		--result;

	operand = Memory::lowByte(result);

	adjustZero(operand);
	adjustHalfCarrySub(before, value, result);
	setFlag(NF);
	setFlag(CF, result & Bit8);
}

void LR35902::sbc(uint8_t& operand, uint8_t value) {
	sub(operand, value, true);
}

void LR35902::sub(uint8_t& operand, uint8_t value) {
	sub(operand, value, false);
}

void LR35902::add(uint8_t& operand, uint8_t value, bool carry) {

	auto before = operand;

	uint16_t result = before + value;
	if (carry && (F() & CF))
		++result;

	operand = Memory::lowByte(result);

	adjustZero(operand);
	adjustHalfCarryAdd(before, value, result);
	clearFlag(NF);
	setFlag(CF, result & Bit8);
}

void LR35902::adc(uint8_t& operand, uint8_t value) {
	add(operand, value, true);
}

void LR35902::add(uint8_t& operand, uint8_t value) {
	add(operand, value, false);
}

//

void LR35902::andr(uint8_t& operand, uint8_t value) {
	setFlag(HC);
	clearFlag(CF | NF);
	operand &= value;
	adjustZero(operand);
}

void LR35902::anda(uint8_t value) {
	andr(A(), value);
}

void LR35902::xora(uint8_t value) {
	clearFlag(HC | CF | NF);
	A() ^= value;
	adjustZero(A());
}

void LR35902::ora(uint8_t value) {
	clearFlag(HC | CF | NF);
	A() |= value;
	adjustZero(A());
}

void LR35902::compare(uint8_t value) {
	auto check = A();
	sub(check, value);
}

//

void LR35902::rlc(uint8_t& operand) {
	auto carry = operand & Bit7;
	operand <<= 1;
	setFlag(CF, carry);
	carry ? operand |= Bit0 : operand &= ~Bit0;
	clearFlag(NF | HC);
	adjustZero(operand);
}

void LR35902::rrc(uint8_t& operand) {
	auto carry = operand & Bit0;
	operand >>= 1;
	carry ? operand |= Bit7 : operand &= ~Bit7;
	setFlag(CF, carry);
	clearFlag(NF | HC);
	adjustZero(operand);
}

void LR35902::rl(uint8_t& operand) {
	auto oldCarry = F() & CF;
	auto newCarry = operand & Bit7;
	operand <<= 1;
	oldCarry ? operand |= Bit0 : operand &= ~Bit0;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustZero(operand);
}

void LR35902::rr(uint8_t& operand) {
	auto oldCarry = F() & CF;
	auto newCarry = operand & Bit0;
	operand >>= 1;
	operand |= oldCarry << 7;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustZero(operand);
}

//

void LR35902::sla(uint8_t& operand) {
	auto newCarry = operand & Bit7;
	operand <<= 1;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustZero(operand);
}

void LR35902::sra(uint8_t& operand) {
	auto new7 = operand & Bit7;
	auto newCarry = operand & Bit0;
	operand >>= 1;
	operand |= new7;
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustZero(operand);
}

void LR35902::srl(uint8_t& operand) {
	auto newCarry = operand & Bit0;
	operand >>= 1;
	operand &= ~Bit7;	// clear bit 7
	setFlag(CF, newCarry);
	clearFlag(NF | HC);
	adjustZero(operand);
}

//

void LR35902::rlca() {
	rlc(A());
}

void LR35902::rrca() {
	rrc(A());
}

void LR35902::rla() {
	rl(A());
}

void LR35902::rra() {
	rr(A());
}

//

void LR35902::bit(int n, uint8_t& operand) {
	auto carry = F() & CF;
	uint8_t discarded = operand;
	andr(discarded, 1 << n);
	setFlag(CF, carry);
}

void LR35902::res(int n, uint8_t& operand) {
	auto bit = 1 << n;
	operand &= ~bit;
}

void LR35902::set(int n, uint8_t& operand) {
	auto bit = 1 << n;
	operand |= bit;
}

//

void LR35902::daa() {

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

void LR35902::cpl() {
	A() = ~A();
	setFlag(HC | NF);
}

void LR35902::scf() {
	setFlag(CF);
	clearFlag(HC | NF);
}

void LR35902::ccf() {
	auto carry = F() & CF;
	clearFlag(CF, carry);
	clearFlag(NF | HC);
}

void LR35902::swap(uint8_t& operand) {
	auto low = lowNibble(operand);
	auto high = highNibble(operand);
	operand = promoteNibble(low) | demoteNibble(high);
	adjustZero(operand);
	clearFlag(NF | HC | CF);
}

int LR35902::step() {
	ExecutingInstruction.fire(*this);
	m_prefixCB = false;
	return fetchExecute();
}

int LR35902::execute(uint8_t opcode) {

	auto x = (opcode & 0b11000000) >> 6;
	auto y = (opcode & 0b111000) >> 3;
	auto z = (opcode & 0b111);

	auto p = (y & 0b110) >> 1;
	auto q = (y & 1);

	cycles = 0;

	if (m_prefixCB)
		executeCB(x, y, z, p, q);
	else
		executeOther(x, y, z, p, q);

	if (cycles == 0)
		throw std::logic_error("Unhandled opcode");

	return cycles * 4;
}

void LR35902::executeCB(int x, int y, int z, int p, int q) {
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

void LR35902::executeOther(int x, int y, int z, int p, int q) {
	switch (x) {
	case 0:
		switch (z) {
		case 0:	// Relative jumps and assorted ops
			switch (y) {
			case 0:	// NOP
				cycles++;
				break;
			case 1:	// GB: LD (nn),SP
				m_memory.setWord(fetchWord(), sp);
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
				RP(p) = fetchWord();
				cycles += 3;
				break;
			case 1:	// ADD HL,rp
				RP(HL_IDX) = add(RP(p));
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
				++RP(p);
				break;
			case 1:	// DEC rp
				--RP(p);
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
			anda(R(z));
			break;
		case 5:	// XOR r
			xora(R(z));
			break;
		case 6:	// OR r
			ora(R(z));
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
				RP2(p) = popWord();
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
					pc = HL().word;
					cycles += 1;
					break;
				case 3:	// LD SP,HL
					sp = HL().word;
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
				pc = fetchWord();
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
			callConditionalFlag(getWord(pc), y);
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
					callConditional(getWord(pc), true);
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
				anda(fetchByte());
				break;
			case 5:	// XOR n
				xora(fetchByte());
				break;
			case 6:	// OR n
				ora(fetchByte());
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
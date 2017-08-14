#include "stdafx.h"
#include "LR35902.h"

// based on http://www.z80.info/decoding.htm
// Half carry flag help from https://github.com/oubiwann/z80

EightBit::LR35902::LR35902(Bus& memory)
: IntelProcessor(memory),
  m_bus(memory),
  m_ime(false),
  m_prefixCB(false) {
}

void EightBit::LR35902::reset() {
	IntelProcessor::reset();
	SP().word = 0xfffe;
	di();
}

void EightBit::LR35902::initialise() {

	IntelProcessor::initialise();

	AF().word = 0xffff;

	BC().word = 0xffff;
	DE().word = 0xffff;
	HL().word = 0xffff;

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

void EightBit::LR35902::increment(uint8_t& f, uint8_t& operand) {
	clearFlag(f, NF);
	adjustZero<LR35902>(f, ++operand);
	clearFlag(f, HC, lowNibble(operand));
}

void EightBit::LR35902::decrement(uint8_t& f, uint8_t& operand) {
	setFlag(f, NF);
	clearFlag(f, HC, lowNibble(operand));
	adjustZero<LR35902>(f, --operand);
}

#pragma endregion Flag manipulation helpers

#pragma region PC manipulation: call/ret/jp/jr

bool EightBit::LR35902::jrConditionalFlag(uint8_t& f, int flag) {
	switch (flag) {
	case 0:	// NZ
		return jrConditional(!(f & ZF));
	case 1:	// Z
		return jrConditional(f & ZF);
	case 2:	// NC
		return jrConditional(!(f & CF));
	case 3:	// C
		return jrConditional(f & CF);
	default:
		__assume(0);
	}
	throw std::logic_error("Unhandled JR conditional");
}

bool EightBit::LR35902::jumpConditionalFlag(uint8_t& f, int flag) {
	switch (flag) {
	case 0:	// NZ
		return jumpConditional(!(f & ZF));
	case 1:	// Z
		return jumpConditional(f & ZF);
	case 2:	// NC
		return jumpConditional(!(f & CF));
	case 3:	// C
		return jumpConditional(f & CF);
	default:
		__assume(0);
	}
	throw std::logic_error("Unhandled JP conditional");
}

void EightBit::LR35902::reti() {
	ret();
	ei();
}

bool EightBit::LR35902::returnConditionalFlag(uint8_t& f, int flag) {
	switch (flag) {
	case 0:	// NZ
		return returnConditional(!(f & ZF));
	case 1:	// Z
		return returnConditional(f & ZF);
	case 2:	// NC
		return returnConditional(!(f & CF));
	case 3:	// C
		return returnConditional(f & CF);
	default:
		__assume(0);
	}
	throw std::logic_error("Unhandled RET conditional");
}

bool EightBit::LR35902::callConditionalFlag(uint8_t& f, int flag) {
	switch (flag) {
	case 0:	// NZ
		return callConditional(!(f & ZF));
	case 1:	// Z
		return callConditional(f & ZF);
	case 2:	// NC
		return callConditional(!(f & CF));
	case 3:	// C
		return callConditional(f & CF);
	default:
		__assume(0);
	}
	throw std::logic_error("Unhandled CALL conditional");
}

#pragma endregion PC manipulation: call/ret/jp/jr

#pragma region 16-bit arithmetic

void EightBit::LR35902::add(uint8_t& f, register16_t& operand, register16_t value) {

	MEMPTR() = operand;

	const auto result = MEMPTR().word + value.word;

	operand.word = result;

	clearFlag(f, NF);
	setFlag(f, CF, result & Bit16);
	adjustHalfCarryAdd(f, MEMPTR().high, value.high, operand.high);
}

#pragma endregion 16-bit arithmetic

#pragma region ALU

void EightBit::LR35902::add(uint8_t& f, uint8_t& operand, uint8_t value, int carry) {

	register16_t result;
	result.word = operand + value + carry;

	adjustHalfCarryAdd(f, operand, value, result.low);

	operand = result.low;

	clearFlag(f, NF);
	setFlag(f, CF, result.word & Bit8);
	adjustZero<LR35902>(f, operand);
}

void EightBit::LR35902::adc(uint8_t& f, uint8_t& operand, uint8_t value) {
	add(f, operand, value, (f & CF) >> 4);
}

void EightBit::LR35902::subtract(uint8_t& f, uint8_t& operand, uint8_t value, int carry) {

	register16_t result;
	result.word = operand - value - carry;

	adjustHalfCarrySub(f, operand, value, result.low);

	operand = result.low;

	setFlag(f, NF);
	setFlag(f, CF, result.word & Bit8);
	adjustZero<LR35902>(f, operand);
}

void EightBit::LR35902::sbc(uint8_t& f, uint8_t& operand, uint8_t value) {
	subtract(f, operand, value, (f & CF) >> 4);
}

void EightBit::LR35902::andr(uint8_t& f, uint8_t& operand, uint8_t value) {
	setFlag(f, HC);
	clearFlag(f, CF | NF);
	adjustZero<LR35902>(f, operand &= value);
}

void EightBit::LR35902::xorr(uint8_t& f, uint8_t& operand, uint8_t value) {
	clearFlag(f, HC | CF | NF);
	adjustZero<LR35902>(f, operand ^= value);
}

void EightBit::LR35902::orr(uint8_t& f, uint8_t& operand, uint8_t value) {
	clearFlag(f, HC | CF | NF);
	adjustZero<LR35902>(f, operand |= value);
}

void EightBit::LR35902::compare(uint8_t& f, uint8_t check, uint8_t value) {
	subtract(f, check, value);
}

#pragma endregion ALU

#pragma region Shift and rotate

uint8_t EightBit::LR35902::rlc(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	setFlag(f, CF, operand & Bit7);
	operand = _rotl8(operand, 1);
	adjustZero<LR35902>(f, operand);
	return operand;
}

uint8_t EightBit::LR35902::rrc(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	setFlag(f, CF, operand & Bit0);
	operand = _rotr8(operand, 1);
	adjustZero<LR35902>(f, operand);
	return operand;
}

uint8_t EightBit::LR35902::rl(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	const auto carry = (f & CF) >> 4;
	setFlag(f, CF, operand & Bit7);
	operand = (operand << 1) | carry;
	adjustZero<LR35902>(f, operand);
	return operand;
}

uint8_t EightBit::LR35902::rr(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	const auto carry = (f & CF) >> 4;
	setFlag(f, CF, operand & Bit0);
	operand = (operand >> 1) | (carry << 7);
	adjustZero<LR35902>(f, operand);
	return operand;
}

//

uint8_t EightBit::LR35902::sla(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	setFlag(f, CF, operand & Bit7);
	operand <<= 1;
	adjustZero<LR35902>(f, operand);
	return operand;
}

uint8_t EightBit::LR35902::sra(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	setFlag(f, CF, operand & Bit0);
	operand = (operand >> 1) | operand & Bit7;
	adjustZero<LR35902>(f, operand);
	return operand;
}

uint8_t EightBit::LR35902::srl(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	setFlag(f, CF, operand & Bit0);
	operand = (operand >> 1) & ~Bit7;
	adjustZero<LR35902>(f, operand);
	return operand;
}

#pragma endregion Shift and rotate

#pragma region BIT/SET/RES

uint8_t EightBit::LR35902::bit(uint8_t& f, int n, uint8_t operand) {
	auto carry = f & CF;
	uint8_t discarded = operand;
	andr(f, discarded, 1 << n);
	setFlag(f, CF, carry);
	return operand;
}

uint8_t EightBit::LR35902::res(int n, uint8_t operand) {
	return operand & ~(1 << n);
}

uint8_t EightBit::LR35902::set(int n, uint8_t operand) {
	return operand | (1 << n);
}

#pragma endregion BIT/SET/RES

#pragma region Miscellaneous instructions

void EightBit::LR35902::daa(uint8_t& a, uint8_t& f) {

	int updated = a;

	if (f & NF) {
		if (f & HC)
			updated = (updated - 6) & Mask8;
		if (f & CF)
			updated -= 0x60;
	} else {
		if ((f & HC) || lowNibble(updated) > 9)
			updated += 6;
		if ((f & CF) || updated > 0x9F)
			updated += 0x60;
	}

	clearFlag(f, HC | ZF);
	setFlag(f, CF, (f & CF) || (updated & Bit8));
	a = updated & Mask8;

	adjustZero<LR35902>(f, a);
}

void EightBit::LR35902::cpl(uint8_t& a, uint8_t& f) {
	setFlag(f, HC | NF);
	a = ~a;
}

void EightBit::LR35902::scf(uint8_t& a, uint8_t& f) {
	setFlag(f, CF);
	clearFlag(f, HC | NF);
}

void EightBit::LR35902::ccf(uint8_t& a, uint8_t& f) {
	clearFlag(f, NF | HC);
	clearFlag(f, CF, f & CF);
}

uint8_t EightBit::LR35902::swap(uint8_t& f, uint8_t operand) {
	auto low = lowNibble(operand);
	auto high = highNibble(operand);
	operand = promoteNibble(low) | high;
	adjustZero<LR35902>(f, operand);
	clearFlag(f, NF | HC | CF);
	return operand;
}

#pragma endregion Miscellaneous instructions

int EightBit::LR35902::step() {
	ExecutingInstruction.fire(*this);
	m_prefixCB = false;
	cycles = 0;
	return fetchExecute();
}

int EightBit::LR35902::execute(uint8_t opcode) {

	const auto& decoded = getDecodedOpcode(opcode);

	auto x = decoded.x;
	auto y = decoded.y;
	auto z = decoded.z;

	auto p = decoded.p;
	auto q = decoded.q;

	if (m_prefixCB)
		executeCB(x, y, z, p, q);
	else
		executeOther(x, y, z, p, q);

	if (cycles == 0)
		throw std::logic_error("Unhandled opcode");

	return cycles * 4;
}

void EightBit::LR35902::executeCB(int x, int y, int z, int p, int q) {
	auto& a = A();
	auto& f = F();
	switch (x) {
	case 0:	{ // rot[y] r[z]
		auto operand = R(z, a);
		switch (y) {
		case 0:
			operand = rlc(f, operand);
			break;
		case 1:
			operand = rrc(f, operand);
			break;
		case 2:
			operand = rl(f, operand);
			break;
		case 3:
			operand = rr(f, operand);
			break;
		case 4:
			operand = sla(f, operand);
			break;
		case 5:
			operand = sra(f, operand);
			break;
		case 6:	// GB: SWAP r
			operand = swap(f, operand);
			break;
		case 7:
			operand = srl(f, operand);
			break;
		default:
			__assume(0);
		}
		cycles += 2;
		R(z, a, operand);
		if (z == 6)
			cycles += 2;
		break;
	} case 1:	// BIT y, r[z]
		bit(f, y, R(z, a));
		cycles += 2;
		if (z == 6)
			cycles += 2;
		break;
	case 2:	// RES y, r[z]
		R(z, a, res(y, R(z, a)));
		cycles += 2;
		if (z == 6)
			cycles += 2;
		break;
	case 3:	// SET y, r[z]
		R(z, a, set(y, R(z, a)));
		cycles += 2;
		if (z == 6)
			cycles += 2;
		break;
	default:
		__assume(0);
	}
}

void EightBit::LR35902::executeOther(int x, int y, int z, int p, int q) {
	auto& a = A();
	auto& f = F();
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
				setWordViaMemptr(SP());
				cycles += 5;
				break;
			case 2:	// GB: STOP
				stop();
				cycles++;
				break;
			case 3:	// JR d
				jr(fetchByte());
				cycles += 4;
				break;
			case 4: // JR cc,d
			case 5:
			case 6:
			case 7:
				if (jrConditionalFlag(f, y - 4))
					cycles++;
				cycles += 2;
				break;
			default:
				__assume(0);
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0: // LD rp,nn
				Processor::fetchWord(RP(p));
				cycles += 3;
				break;
			case 1:	// ADD HL,rp
				add(f, HL(), RP(p));
				cycles += 2;
				break;
			default:
				__assume(0);
			}
			break;
		case 2:	// Indirect loading
			switch (q) {
			case 0:
				switch (p) {
				case 0:	// LD (BC),A
					m_memory.write(BC().word, a);
					cycles += 2;
					break;
				case 1:	// LD (DE),A
					m_memory.write(DE().word, a);
					cycles += 2;
					break;
				case 2:	// GB: LDI (HL),A
					m_memory.write(HL().word++, a);
					cycles += 2;
					break;
				case 3: // GB: LDD (HL),A
					m_memory.write(HL().word--, a);
					cycles += 2;
					break;
				default:
					__assume(0);
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					a = m_memory.read(BC().word);
					cycles += 2;
					break;
				case 1:	// LD A,(DE)
					a = m_memory.read(DE().word);
					cycles += 2;
					break;
				case 2:	// GB: LDI A,(HL)
					a = m_memory.read(HL().word++);
					cycles += 2;
					break;
				case 3:	// GB: LDD A,(HL)
					a = m_memory.read(HL().word--);
					cycles += 2;
					break;
				default:
					__assume(0);
				}
				break;
			default:
				__assume(0);
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
			default:
				__assume(0);
			}
			cycles += 2;
			break;
		case 4: { // 8-bit INC
			auto operand = R(y, a);
			increment(f, operand);
			R(y, a, operand);
			cycles++;
			if (y == 6)
				cycles += 2;
			break;
		} case 5: {	// 8-bit DEC
			auto operand = R(y, a);
			decrement(f, operand);
			R(y, a, operand);
			cycles++;
			if (y == 6)
				cycles += 2;
			break;
		} case 6:	// 8-bit load immediate
			R(y, a, fetchByte());	// LD r,n
			cycles += 2;
			break;
		case 7:	// Assorted operations on accumulator/flags
			switch (y) {
			case 0:
				a = rlc(f, a);
				break;
			case 1:
				a = rrc(f, a);
				break;
			case 2:
				a = rl(f, a);
				break;
			case 3:
				a = rr(f, a);
				break;
			case 4:
				daa(a, f);
				break;
			case 5:
				cpl(a, f);
				break;
			case 6:
				scf(a, f);
				break;
			case 7:
				ccf(a, f);
				break;
			default:
				__assume(0);
			}
			cycles++;
			break;
		default:
			__assume(0);
		}
		break;
	case 1:	// 8-bit loading
		if (z == 6 && y == 6) { 	// Exception (replaces LD (HL), (HL))
			halt();
		} else {
			R(y, a, R(z, a));
			if ((y == 6) || (z == 6)) // M operations
				cycles++;
		}
		cycles++;
		break;
	case 2:	// Operate on accumulator and register/memory location
		switch (y) {
		case 0:	// ADD A,r
			add(f, a, R(z, a));
			break;
		case 1:	// ADC A,r
			adc(f, a, R(z, a));
			break;
		case 2:	// SUB r
			subtract(f, a, R(z, a));
			break;
		case 3:	// SBC A,r
			sbc(f, a, R(z, a));
			break;
		case 4:	// AND r
			andr(f, a, R(z, a));
			break;
		case 5:	// XOR r
			xorr(f, a, R(z, a));
			break;
		case 6:	// OR r
			orr(f, a, R(z, a));
			break;
		case 7:	// CP r
			compare(f, a, R(z, a));
			break;
		default:
			__assume(0);
		}
		cycles++;
		if (z == 6)
			cycles++;
		break;
	case 3:
		switch (z) {
		case 0:	// Conditional return
			switch (y) {
			case 0:
			case 1:
			case 2:
			case 3:
				if (returnConditionalFlag(f, y))
					cycles += 3;
				cycles += 2;
				break;
			case 4:	// GB: LD (FF00 + n),A
				m_bus.writeRegister(fetchByte(), a);
				cycles += 3;
				break;
			case 5: { // GB: ADD SP,dd
					auto before = SP();
					auto value = fetchByte();
					auto result = SP().word + (int8_t)value;
					SP().word = result;
					clearFlag(f, ZF | NF);
					setFlag(f, CF, result & Bit16);
					adjustHalfCarryAdd(f, before.high, value, SP().high);
				}
				cycles += 4;
				break;
			case 6:	// GB: LD A,(FF00 + n)
				a = m_bus.readRegister(fetchByte());
				cycles += 3;
				break;
			case 7: { // GB: LD HL,SP + dd
					auto before = HL();
					auto value = fetchByte();
					auto result = SP().word + (int8_t)value;
					HL().word = result;
					clearFlag(f, ZF | NF);
					setFlag(f, CF, result & Bit16);
					adjustHalfCarryAdd(f, before.high, value, HL().high);
				}
				cycles += 3;
				break;
			default:
				__assume(0);
			}
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
					PC() = HL();
					cycles++;
					break;
				case 3:	// LD SP,HL
					SP() = HL();
					cycles += 2;
					break;
				default:
					__assume(0);
				}
				break;
			default:
				__assume(0);
			}
			break;
		case 2:	// Conditional jump
			switch (y) {
			case 0:
			case 1:
			case 2:
			case 3:
				jumpConditionalFlag(f, y);
				cycles += 3;
				break;
			case 4:	// GB: LD (FF00 + C),A
				m_bus.writeRegister(C(), a);
				cycles += 2;
				break;
			case 5:	// GB: LD (nn),A
				fetchWord();
				m_bus.write(MEMPTR().word, a);
				cycles += 4;
				break;
			case 6:	// GB: LD A,(FF00 + C)
				a = m_bus.readRegister(C());
				cycles += 2;
				break;
			case 7:	// GB: LD A,(nn)
				fetchWord();
				a = m_bus.read(MEMPTR().word);
				cycles += 4;
				break;
			default:
				__assume(0);
			}
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0:	// JP nn
				fetchWord();
				jump();
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
			if (callConditionalFlag(f, y))
				cycles += 3;
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
					call();
					cycles += 6;
					break;
				}
				break;
			default:
				__assume(0);
			}
			break;
		case 6:	// Operate on accumulator and immediate operand: alu[y] n
			switch (y) {
			case 0:	// ADD A,n
				add(f, a, fetchByte());
				break;
			case 1:	// ADC A,n
				adc(f, a, fetchByte());
				break;
			case 2:	// SUB n
				subtract(f, a, fetchByte());
				break;
			case 3:	// SBC A,n
				sbc(f, a, fetchByte());
				break;
			case 4:	// AND n
				andr(f, a, fetchByte());
				break;
			case 5:	// XOR n
				xorr(f, a, fetchByte());
				break;
			case 6:	// OR n
				orr(f, a, fetchByte());
				break;
			case 7:	// CP n
				compare(f, a, fetchByte());
				break;
			default:
				__assume(0);
			}
			cycles += 2;
			break;
		case 7:	// Restart: RST y * 8
			restart(y << 3);
			cycles += 4;
			break;
		default:
			__assume(0);
		}
		break;
	}
}
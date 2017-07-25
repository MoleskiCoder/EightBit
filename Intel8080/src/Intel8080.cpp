#include "stdafx.h"
#include "Intel8080.h"

#include "Memory.h"
#include "Disassembler.h"

EightBit::Intel8080::Intel8080(Memory& memory, InputOutput& ports)
: IntelProcessor(memory),
  m_interrupt(false),
  m_ports(ports) {
	bc.word = de.word = hl.word = 0;
}

void EightBit::Intel8080::initialise() {
	IntelProcessor::initialise();
	AF().word = BC().word = DE().word = HL().word = 0;
}

#pragma region Interrupt routines

void EightBit::Intel8080::di() {
	m_interrupt = false;
}

void EightBit::Intel8080::ei() {
	m_interrupt = true;
}

int EightBit::Intel8080::interrupt(uint8_t value) {
	if (isInterruptable()) {
		di();
		return execute(value);
	}
	return 0;
}

bool EightBit::Intel8080::isInterruptable() const {
	return m_interrupt;
}

#pragma endregion Interrupt routines

#pragma region Flag manipulation helpers

void EightBit::Intel8080::increment(uint8_t& f, uint8_t& operand) {
	adjustSZP<Intel8080>(f, ++operand);
	clearFlag(f, AC, lowNibble(operand));
}

void EightBit::Intel8080::decrement(uint8_t& f, uint8_t& operand) {
	adjustSZP<Intel8080>(f, --operand);
	setFlag(f, AC, lowNibble(operand) != Mask4);
}

#pragma endregion Flag manipulation helpers

#pragma region PC manipulation: call/ret/jp/jr

bool EightBit::Intel8080::jumpConditionalFlag(uint8_t& f, int flag) {
	switch (flag) {
	case 0:	// NZ
		return jumpConditional(!(f & ZF));
	case 1:	// Z
		return jumpConditional(f & ZF);
	case 2:	// NC
		return jumpConditional(!(f & CF));
	case 3:	// C
		return jumpConditional(f & CF);
	case 4:	// PO
		return jumpConditional(!(f & PF));
	case 5:	// PE
		return jumpConditional(f & PF);
	case 6:	// P
		return jumpConditional(!(f & SF));
	case 7:	// M
		return jumpConditional(f & SF);
	default:
		__assume(0);
	}
	throw std::logic_error("Unhandled JP conditional");
}

bool EightBit::Intel8080::returnConditionalFlag(uint8_t& f, int flag) {
	switch (flag) {
	case 0:	// NZ
		return returnConditional(!(f & ZF));
	case 1:	// Z
		return returnConditional(f & ZF);
	case 2:	// NC
		return returnConditional(!(f & CF));
	case 3:	// C
		return returnConditional(f & CF);
	case 4:	// PO
		return returnConditional(!(f & PF));
	case 5:	// PE
		return returnConditional(f & PF);
	case 6:	// P
		return returnConditional(!(f & SF));
	case 7:	// M
		return returnConditional(f & SF);
	default:
		__assume(0);
	}
	throw std::logic_error("Unhandled RET conditional");
}

bool EightBit::Intel8080::callConditionalFlag(uint8_t& f, int flag) {
	switch (flag) {
	case 0:	// NZ
		return callConditional(!(f & ZF));
	case 1:	// Z
		return callConditional(f & ZF);
	case 2:	// NC
		return callConditional(!(f & CF));
	case 3:	// C
		return callConditional(f & CF);
	case 4:	// PO
		return callConditional(!(f & PF));
	case 5:	// PE
		return callConditional(f & PF);
	case 6:	// P
		return callConditional(!(f & SF));
	case 7:	// M
		return callConditional(f & SF);
	default:
		__assume(0);
	}
	throw std::logic_error("Unhandled CALL conditional");
}

#pragma endregion PC manipulation: call/ret/jp/jr

#pragma region 16-bit arithmetic

void EightBit::Intel8080::add(uint8_t& f, register16_t& operand, register16_t value) {
	const auto result = operand.word + value.word;
	setFlag(f, CF, result & Bit16);
	operand.word = result;
}

#pragma endregion 16-bit arithmetic

#pragma region ALU

void EightBit::Intel8080::add(uint8_t& f, uint8_t& operand, uint8_t value, int carry) {

	register16_t result;
	result.word = operand + value + carry;

	adjustAuxiliaryCarryAdd(f, operand, value, result.word);

	operand = result.low;

	setFlag(f, CF, result.word & Bit8);
	adjustSZP<Intel8080>(f, operand);
}

void EightBit::Intel8080::adc(uint8_t& f, uint8_t& operand, uint8_t value) {
	add(f, operand, value, f & CF);
}

void EightBit::Intel8080::subtract(uint8_t& f, uint8_t& operand, uint8_t value, int carry) {

	register16_t result;
	result.word = operand - value - carry;

	adjustAuxiliaryCarrySub(f, operand, value, result.word);

	operand = result.low;

	setFlag(f, CF, result.word & Bit8);
	adjustSZP<Intel8080>(f, operand);
}

void EightBit::Intel8080::sbb(uint8_t& f, uint8_t& operand, uint8_t value) {
	subtract(f, operand, value, f & CF);
}

void EightBit::Intel8080::andr(uint8_t& f, uint8_t& operand, uint8_t value) {
	setFlag(f, AC, (operand | value) & Bit3);
	clearFlag(f, CF);
	adjustSZP<Intel8080>(f, operand &= value);
}

void EightBit::Intel8080::xorr(uint8_t& f, uint8_t& operand, uint8_t value) {
	clearFlag(f, AC | CF);
	adjustSZP<Intel8080>(f, operand ^= value);
}

void EightBit::Intel8080::orr(uint8_t& f, uint8_t& operand, uint8_t value) {
	clearFlag(f, AC | CF);
	adjustSZP<Intel8080>(f, operand |= value);
}

void EightBit::Intel8080::compare(uint8_t& f, uint8_t check, uint8_t value) {
	subtract(f, check, value);
}

#pragma endregion ALU

#pragma region Shift and rotate

void EightBit::Intel8080::rlc() {
	auto& a = A();
	auto carry = a & Bit7;
	a = (a << 1) | (carry >> 7);
	setFlag(F(), CF, carry);
}

void EightBit::Intel8080::rrc() {
	auto& a = A();
	auto carry = a & Bit0;
	a = (a >> 1) | (carry << 7);
	setFlag(F(), CF, carry);
}

void EightBit::Intel8080::ral() {
	auto& a = A();
	auto& f = F();
	const auto carry = f & CF;
	setFlag(f, CF, a & Bit7);
	a = (a << 1) | carry;
}

void EightBit::Intel8080::rar() {
	auto& a = A();
	auto& f = F();
	const auto carry = f & CF;
	setFlag(f, CF, a & Bit0);
	a = (a >> 1) | (carry << 7);
}

#pragma endregion Shift and rotate

#pragma region Miscellaneous instructions

void EightBit::Intel8080::daa() {
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
	add(f, A(), addition);
	setFlag(f, CF, carry);
}

void EightBit::Intel8080::cma() {
	A() = ~A();
}

void EightBit::Intel8080::stc() {
	setFlag(F(), CF);
}

void EightBit::Intel8080::cmc() {
	clearFlag(F(), CF, F() & CF);
}

void EightBit::Intel8080::xhtl() {
	m_memory.ADDRESS() = SP();
	MEMPTR().low = m_memory.reference();
	m_memory.reference() = L();
	L() = MEMPTR().low;
	m_memory.ADDRESS().word++;
	MEMPTR().high = m_memory.reference();
	m_memory.reference() = H();
	H() = MEMPTR().high;
}

#pragma endregion Miscellaneous instructions

#pragma region I/O instructions

void EightBit::Intel8080::out() {
	m_ports.write(fetchByte(), A());
}

void EightBit::Intel8080::in() {
	A() = m_ports.read(fetchByte());
}

#pragma endregion I/O instructions

int EightBit::Intel8080::step() {
	ExecutingInstruction.fire(*this);
	cycles = 0;
	return execute(fetchByte());
}

int EightBit::Intel8080::execute(uint8_t opcode) {

	const auto& decoded = getDecodedOpcode(opcode);

	auto x = decoded.x;
	auto y = decoded.y;
	auto z = decoded.z;

	auto p = decoded.p;
	auto q = decoded.q;

	execute(x, y, z, p, q);

	if (cycles == 0)
		throw std::logic_error("Unhandled opcode");

	return cycles;
}

void EightBit::Intel8080::execute(int x, int y, int z, int p, int q) {
	auto& a = A();
	auto& f = F();
	switch (x) {
	case 0:
		switch (z) {
		case 0:	// Relative jumps and assorted ops
			switch (y) {
			case 0:	// NOP
				cycles += 4;
				break;
			case 1:	// EX AF AF'
				break;
			case 2:	// DJNZ d
				break;
			case 3:	// JR d
				break;
			case 4: // JR cc,d
			case 5:
			case 6:
			case 7:
				break;
			default:
				__assume(0);
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0: // LD rp,nn
				Processor::fetchWord(RP(p));
				cycles += 10;
				break;
			case 1:	// ADD HL,rp
				add(f, HL(), RP(p));
				cycles += 11;
				break;
			}
			break;
		case 2:	// Indirect loading
			switch (q) {
			case 0:
				switch (p) {
				case 0:	// LD (BC),A
					MEMPTR() = BC();
					MEMPTR().high = memptrReference() = a;
					cycles += 7;
					break;
				case 1:	// LD (DE),A
					MEMPTR() = DE();
					MEMPTR().high = memptrReference() = a;
					cycles += 7;
					break;
				case 2:	// LD (nn),HL
					fetchWord();
					setWordViaMemptr(HL());
					cycles += 16;
					break;
				case 3: // LD (nn),A
					fetchWord();
					MEMPTR().high = memptrReference() = a;
					cycles += 13;
					break;
				default:
					__assume(0);
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					MEMPTR() = BC();
					a = memptrReference();
					cycles += 7;
					break;
				case 1:	// LD A,(DE)
					MEMPTR() = DE();
					a = memptrReference();
					cycles += 7;
					break;
				case 2:	// LD HL,(nn)
					fetchWord();
					getWordViaMemptr(HL());
					cycles += 16;
					break;
				case 3:	// LD A,(nn)
					fetchWord();
					a = memptrReference();
					cycles += 13;
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
			cycles += 6;
			break;
		case 4:	// 8-bit INC
			increment(f, R(y));	// INC r
			cycles += 4;
			break;
		case 5:	// 8-bit DEC
			decrement(f, R(y));	// DEC r
			cycles += 4;
			if (y == 6)
				cycles += 7;
			break;
		case 6: { // 8-bit load immediate
			auto& r = R(y);	// LD r,n
			r = fetchByte();
			cycles += 7;
			if (y == 6)
				cycles += 3;
			break;
		}
		case 7:	// Assorted operations on accumulator/flags
			switch (y) {
			case 0:
				rlc();
				break;
			case 1:
				rrc();
				break;
			case 2:
				ral();
				break;
			case 3:
				rar();
				break;
			case 4:
				daa();
				break;
			case 5:
				cma();
				break;
			case 6:
				stc();
				break;
			case 7:
				cmc();
				break;
			default:
				__assume(0);
			}
			cycles += 4;
			break;
		default:
			__assume(0);
		}
		break;
	case 1:	// 8-bit loading
		if (z == 6 && y == 6) { 	// Exception (replaces LD (HL), (HL))
			halt();
		} else {
			R(y) = R(z);
			if ((y == 6) || (z == 6))	// M operations
				cycles += 3;
		}
		cycles += 4;
		break;
	case 2:	// Operate on accumulator and register/memory location
		switch (y) {
		case 0:	// ADD A,r
			add(f, a, R(z));
			break;
		case 1:	// ADC A,r
			adc(f, a, R(z));
			break;
		case 2:	// SUB r
			subtract(f, a, R(z));
			break;
		case 3:	// SBC A,r
			sbb(f, a, R(z));
			break;
		case 4:	// AND r
			andr(f, a, R(z));
			break;
		case 5:	// XOR r
			xorr(f, a, R(z));
			break;
		case 6:	// OR r
			orr(f, a, R(z));
			break;
		case 7:	// CP r
			compare(f, a, R(z));
			break;
		default:
			__assume(0);
		}
		cycles += 4;
		if (z == 6)
			cycles += 3;
		break;
	case 3:
		switch (z) {
		case 0:	// Conditional return
			if (returnConditionalFlag(f, y))
				cycles += 6;
			cycles += 5;
			break;
		case 1:	// POP & various ops
			switch (q) {
			case 0:	// POP rp2[p]
				popWord(RP2(p));
				cycles += 10;
				break;
			case 1:
				switch (p) {
				case 0:	// RET
					ret();
					cycles += 10;
					break;
				case 1:	// EXX
					break;
				case 2:	// JP HL
					PC() = HL();
					cycles += 4;
					break;
				case 3:	// LD SP,HL
					SP() = HL();
					cycles += 4;
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
			jumpConditionalFlag(f, y);
			cycles += 10;
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0:	// JP nn
				fetchWord();
				jump();
				cycles += 10;
				break;
			case 1:	// CB prefix
				break;
			case 2:	// OUT (n),A
				out();
				cycles += 11;
				break;
			case 3:	// IN A,(n)
				in();
				cycles += 11;
				break;
			case 4:	// EX (SP),HL
				xhtl();
				cycles += 19;
				break;
			case 5:	// EX DE,HL
				std::swap(DE(), HL());
				cycles += 4;
				break;
			case 6:	// DI
				di();
				cycles += 4;
				break;
			case 7:	// EI
				ei();
				cycles += 4;
				break;
			default:
				__assume(0);
			}
			break;
		case 4:	// Conditional call: CALL cc[y], nn
			if (callConditionalFlag(f, y))
				cycles += 7;
			cycles += 10;
			break;
		case 5:	// PUSH & various ops
			switch (q) {
			case 0:	// PUSH rp2[p]
				pushWord(RP2(p));
				cycles += 11;
				break;
			case 1:
				switch (p) {
				case 0:	// CALL nn
					fetchWord();
					call();
					cycles += 17;
					break;
				case 1:	// DD prefix
					break;
				case 2:	// ED prefix
					break;
				case 3:	// FD prefix
					break;
				default:
					__assume(0);
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
				sbb(f, a, fetchByte());
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
			cycles += 7;
			break;
		case 7:	// Restart: RST y * 8
			restart(y << 3);
			cycles += 11;
			break;
		default:
			__assume(0);
		}
		break;
	}
}

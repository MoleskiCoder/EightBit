#include "stdafx.h"
#include "Intel8080.h"

#include "Memory.h"
#include "Disassembler.h"

EightBit::Intel8080::Intel8080(Memory& memory, InputOutput& ports)
: IntelProcessor(memory),
  m_interrupt(false),
  m_ports(ports) {
	bc.word = de.word = hl.word = 0;
	installInstructions();
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

void EightBit::Intel8080::dad(uint16_t value) {
	auto& f = F();
	auto sum = HL().word + value;
	setFlag(f, CF, sum & Bit16);
	HL().word = sum;
}

#pragma endregion 16-bit arithmetic

#pragma region ALU

void EightBit::Intel8080::add(uint8_t value, int carry) {
	auto& a = A();
	auto& f = F();
	register16_t sum;
	sum.word = a + value + carry;
	adjustAuxiliaryCarryAdd(f, a, value, sum.word);
	a = sum.low;
	setFlag(f, CF, sum.word & Bit8);
	adjustSZP<Intel8080>(f, a);
}

void EightBit::Intel8080::adc(uint8_t value) {
	add(value, F() & CF);
}

void EightBit::Intel8080::subtract(uint8_t& f, uint8_t& operand, uint8_t value, int carry) {

	register16_t result;
	result.word = operand - value - carry;

	adjustAuxiliaryCarrySub(f, operand, value, result.word);

	operand = result.low;

	setFlag(f, CF, result.word & Bit8);
	adjustSZP<Intel8080>(f, operand);
}

void EightBit::Intel8080::sbb(uint8_t value) {
	subtract(F(), A(), value, F() & CF);
}

void EightBit::Intel8080::anda(uint8_t value) {
	auto& a = A();
	auto& f = F();
	setFlag(f, AC, (a | value) & Bit3);
	clearFlag(f, CF);
	adjustSZP<Intel8080>(f, a &= value);
}

void EightBit::Intel8080::xra(uint8_t value) {
	auto& f = F();
	clearFlag(f, AC | CF);
	adjustSZP<Intel8080>(f, A() ^= value);
}

void EightBit::Intel8080::ora(uint8_t value) {
	auto& f = F();
	clearFlag(f, AC | CF);
	adjustSZP<Intel8080>(f, A() |= value);
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
	add(addition);
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
				fetchWord(RP(p));
				cycles += 10;
				break;
			case 1:	// ADD HL,rp
				dad(RP(p).word);
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
			add(R(z));
			break;
		case 1:	// ADC A,r
			adc(R(z));
			break;
		case 2:	// SUB r
			subtract(f, a, R(z));
			break;
		case 3:	// SBC A,r
			sbb(R(z));
			break;
		case 4:	// AND r
			anda(R(z));
			break;
		case 5:	// XOR r
			xra(R(z));
			break;
		case 6:	// OR r
			ora(R(z));
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
				adjustReservedFlags();
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
				add(fetchByte());
				break;
			case 1:	// ADC A,n
				adc(fetchByte());
				break;
			case 2:	// SUB n
				subtract(f, a, fetchByte());
				break;
			case 3:	// SBC A,n
				sbb(fetchByte());
				break;
			case 4:	// AND n
				anda(fetchByte());
				break;
			case 5:	// XOR n
				xra(fetchByte());
				break;
			case 6:	// OR n
				ora(fetchByte());
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

//

EightBit::Intel8080::Instruction EightBit::Intel8080::INS(instruction_t method, AddressingMode mode, std::string disassembly, int cycles) {
	Intel8080::Instruction returnValue;
	returnValue.vector = method;
	returnValue.mode = mode;
	returnValue.disassembly = disassembly;
	returnValue.count = cycles;
	return returnValue;
}

EightBit::Intel8080::Instruction EightBit::Intel8080::UNKNOWN() {
	Intel8080::Instruction returnValue;
	returnValue.vector = std::bind(&Intel8080::___, this);
	returnValue.mode = Unknown;
	returnValue.disassembly = "";
	returnValue.count = 0;
	return returnValue;
}

#define BIND(method)	std::bind(&Intel8080:: method, this)

void EightBit::Intel8080::installInstructions() {
	instructions = {
		////	0											1											2											3											4											5												6											7											8											9											A											B											C											D											E											F
		/* 0 */	INS(BIND(nop), Implied, "NOP", 4),			INS(BIND(nop), Absolute, "LXI B,", 10),		INS(BIND(nop), Implied, "STAX B", 7),		INS(BIND(nop), Implied, "INX B", 5),		INS(BIND(nop), Implied, "INR B", 5),		INS(BIND(nop), Implied, "DCR B", 5),			INS(BIND(nop), Immediate, "MVI B,", 7),		INS(BIND(nop), Implied, "RLC", 4),			UNKNOWN(),									INS(BIND(nop), Implied, "DAD B", 10),		INS(BIND(nop), Implied, "LDAX B", 7),		INS(BIND(nop), Implied, "DCX B", 5),		INS(BIND(nop), Implied, "INR C", 5),		INS(BIND(nop), Implied, "DCR C", 5),		INS(BIND(nop), Immediate, "MVI C,", 7),		INS(BIND(nop), Implied, "RRC", 4),			//	0
		/* 1 */	UNKNOWN(),									INS(BIND(nop), Absolute, "LXI D,", 10),		INS(BIND(nop), Implied, "STAX D", 7),		INS(BIND(nop), Implied, "INX D", 5),		INS(BIND(nop), Implied, "INR D", 5),		INS(BIND(nop), Implied, "DCR D", 5),			INS(BIND(nop), Immediate, "MVI D,", 7),		INS(BIND(nop), Implied, "RAL", 4),			UNKNOWN(),									INS(BIND(nop), Implied, "DAD D", 10),		INS(BIND(nop), Implied, "LDAX D", 7),		INS(BIND(nop), Implied, "DCX D", 5),		INS(BIND(nop), Implied, "INR E", 5),		INS(BIND(nop), Implied, "DCR E", 5),		INS(BIND(nop), Immediate, "MVI E,", 7),		INS(BIND(nop), Implied, "RAR", 4),			//	1
		/* 2 */	UNKNOWN(),									INS(BIND(nop), Absolute, "LXI H,", 10),		INS(BIND(nop), Absolute, "SHLD", 16),		INS(BIND(nop), Implied, "INX H", 5),		INS(BIND(nop), Implied, "INR H", 5),		INS(BIND(nop), Implied, "DCR H", 5),			INS(BIND(nop), Immediate, "MVI H,",7),		INS(BIND(nop), Implied, "DAA", 4),			UNKNOWN(),									INS(BIND(nop), Implied, "DAD H", 10),		INS(BIND(nop), Absolute, "LHLD ", 16),		INS(BIND(nop), Implied, "DCX H", 5),		INS(BIND(nop), Implied, "INR L", 5),		INS(BIND(nop), Implied, "DCR L", 5),		INS(BIND(nop), Immediate, "MVI L,", 7),		INS(BIND(nop), Implied, "CMA", 4),			//	2
		/* 3 */	UNKNOWN(),									INS(BIND(nop), Absolute, "LXI SP,", 10),	INS(BIND(nop), Absolute, "STA ", 13),		INS(BIND(nop), Implied, "INX SP", 5),		INS(BIND(nop), Implied, "INR M", 10),		INS(BIND(nop), Implied, "DCR M", 10),			INS(BIND(nop), Immediate, "MVI M,", 10),	INS(BIND(nop), Implied, "STC", 4),			UNKNOWN(),									INS(BIND(nop), Implied, "DAD SP", 10),		INS(BIND(nop), Absolute, "LDA ", 13),		INS(BIND(nop), Implied, "DCX SP", 5),		INS(BIND(nop), Implied, "INR A", 5),		INS(BIND(nop), Implied, "DCR A", 5),		INS(BIND(nop), Immediate, "MVI A,", 7),		INS(BIND(nop), Implied, "CMC", 4),			//	3

		/* 4 */	INS(BIND(nop), Implied, "MOV B,B", 5),		INS(BIND(nop), Implied, "MOV B,C", 5),		INS(BIND(nop), Implied, "MOV B,D", 5),		INS(BIND(nop), Implied, "MOV B,E", 5),		INS(BIND(nop), Implied, "MOV B,H", 5),		INS(BIND(nop), Implied, "MOV B,L", 5),			INS(BIND(nop), Implied, "MOV B,M", 7),		INS(BIND(nop), Implied, "MOV B,A", 5),		INS(BIND(nop), Implied, "MOV C,B", 5),		INS(BIND(nop), Implied, "MOV C,C", 5),		INS(BIND(nop), Implied, "MOV C,D", 5),		INS(BIND(nop), Implied, "MOV C,E", 5),		INS(BIND(nop), Implied, "MOV C,H", 5),		INS(BIND(nop), Implied, "MOV C,L", 5),		INS(BIND(nop), Implied, "MOV C,M", 7),		INS(BIND(nop), Implied, "MOV C,A", 5),		//	4
		/* 5 */	INS(BIND(nop), Implied, "MOV D,B", 5),		INS(BIND(nop), Implied, "MOV D,C", 5),		INS(BIND(nop), Implied, "MOV D,D", 5),		INS(BIND(nop), Implied, "MOV D,E", 5),		INS(BIND(nop), Implied, "MOV D,H", 5),		INS(BIND(nop), Implied, "MOV D,L", 5),			INS(BIND(nop), Implied, "MOV D,M", 7),		INS(BIND(nop), Implied, "MOV D,A", 5),		INS(BIND(nop), Implied, "MOV E,B", 5),		INS(BIND(nop), Implied, "MOV E,C", 5),		INS(BIND(nop), Implied, "MOV E,D", 5),		INS(BIND(nop), Implied, "MOV E,E", 5),		INS(BIND(nop), Implied, "MOV E,H", 5),		INS(BIND(nop), Implied, "MOV E,L", 5),		INS(BIND(nop), Implied, "MOV E,M", 7),		INS(BIND(nop), Implied, "MOV E,A", 5),		//	5
		/* 6 */	INS(BIND(nop), Implied, "MOV H,B", 5),		INS(BIND(nop), Implied, "MOV H,C", 5),		INS(BIND(nop), Implied, "MOV H,D", 5),		INS(BIND(nop), Implied, "MOV H,E", 5),		INS(BIND(nop), Implied, "MOV H,H", 5),		INS(BIND(nop), Implied, "MOV H,L", 5),			INS(BIND(nop), Implied, "MOV H,M", 7),		INS(BIND(nop), Implied, "MOV H,A", 5),		INS(BIND(nop), Implied, "MOV L,B", 5),		INS(BIND(nop), Implied, "MOV L,C", 5),		INS(BIND(nop), Implied, "MOV L,D", 5),		INS(BIND(nop), Implied, "MOV L,E", 5),		INS(BIND(nop), Implied, "MOV L,H", 5),		INS(BIND(nop), Implied, "MOV L,L", 5),		INS(BIND(nop), Implied, "MOV L,M", 7),		INS(BIND(nop), Implied, "MOV L,A", 5),		//	6
		/* 7 */	INS(BIND(nop), Implied, "MOV M,B", 7),		INS(BIND(nop), Implied, "MOV M,C", 7),		INS(BIND(nop), Implied, "MOV M,D", 7),		INS(BIND(nop), Implied, "MOV M,E", 7),		INS(BIND(nop), Implied, "MOV M,H", 7),		INS(BIND(nop), Implied, "MOV M,L", 7),			INS(BIND(nop), Implied, "HLT", 7),			INS(BIND(nop), Implied, "MOV M,A", 7),		INS(BIND(nop), Implied, "MOV A,B", 5),		INS(BIND(nop), Implied, "MOV A,C", 5),		INS(BIND(nop), Implied, "MOV A,D", 5),		INS(BIND(nop), Implied, "MOV A,E", 5),		INS(BIND(nop), Implied, "MOV A,H", 5),		INS(BIND(nop), Implied, "MOV A,L", 5),		INS(BIND(nop), Implied, "MOV A,M", 7),		INS(BIND(nop), Implied, "MOV A,A", 5),		//	7

		/* 8 */	INS(BIND(nop), Implied, "ADD B", 4),		INS(BIND(nop), Implied, "ADD C", 4),		INS(BIND(nop), Implied, "ADD D", 4),		INS(BIND(nop), Implied, "ADD E", 4),		INS(BIND(nop), Implied, "ADD H", 4),		INS(BIND(nop), Implied, "ADD L", 4),			INS(BIND(nop), Implied, "ADD M", 7),		INS(BIND(nop), Implied, "ADD A", 4),		INS(BIND(nop), Implied, "ADC B", 4),		INS(BIND(nop), Implied, "ADC C", 4),		INS(BIND(nop), Implied, "ADC D", 4),		INS(BIND(nop), Implied, "ADC E", 4),		INS(BIND(nop), Implied, "ADC H", 4),		INS(BIND(nop), Implied, "ADC L", 4),		INS(BIND(nop), Implied, "ADC M", 4),		INS(BIND(nop), Implied, "ADC A", 4),		//	8
		/* 9 */	INS(BIND(nop), Implied, "SUB B", 4),		INS(BIND(nop), Implied, "SUB C", 4),		INS(BIND(nop), Implied, "SUB D", 4),		INS(BIND(nop), Implied, "SUB E", 4),		INS(BIND(nop), Implied, "SUB H", 4),		INS(BIND(nop), Implied, "SUB L", 4),			INS(BIND(nop), Implied, "SUB M", 7),		INS(BIND(nop), Implied, "SUB A", 4),		INS(BIND(nop), Implied, "SBB B", 4),		INS(BIND(nop), Implied, "SBB C", 4),		INS(BIND(nop), Implied, "SBB D", 4),		INS(BIND(nop), Implied, "SBB E", 4),		INS(BIND(nop), Implied, "SBB H", 4),		INS(BIND(nop), Implied, "SBB L", 4),		INS(BIND(nop), Implied, "SBB M", 4),		INS(BIND(nop), Implied, "SBB A", 4),		//	9
		/* A */	INS(BIND(nop), Implied, "ANA B", 4),		INS(BIND(nop), Implied, "ANA C", 4),		INS(BIND(nop), Implied, "ANA D", 4),		INS(BIND(nop), Implied, "ANA E", 4),		INS(BIND(nop), Implied, "ANA H", 4),		INS(BIND(nop), Implied, "ANA L", 4),			INS(BIND(nop), Implied, "ANA M", 7),		INS(BIND(nop), Implied, "ANA A", 4),		INS(BIND(nop), Implied, "XRA B", 4),		INS(BIND(nop), Implied, "XRA C", 4),		INS(BIND(nop), Implied, "XRA D", 4),		INS(BIND(nop), Implied, "XRA E", 4),		INS(BIND(nop), Implied, "XRA H", 4),		INS(BIND(nop), Implied, "XRA L", 4),		INS(BIND(nop), Implied, "XRA M", 4),		INS(BIND(nop), Implied, "XRA A", 4),		//	A
		/* B */	INS(BIND(nop), Implied, "ORA B", 4),		INS(BIND(nop), Implied, "ORA C", 4),		INS(BIND(nop), Implied, "ORA D", 4),		INS(BIND(nop), Implied, "ORA E", 4),		INS(BIND(nop), Implied, "ORA H", 4),		INS(BIND(nop), Implied, "ORA L", 4),			INS(BIND(nop), Implied, "ORA M", 7),		INS(BIND(nop), Implied, "ORA A", 4),		INS(BIND(nop), Implied, "CMP B", 4),		INS(BIND(nop), Implied, "CMP C", 4),		INS(BIND(nop), Implied, "CMP D", 4),		INS(BIND(nop), Implied, "CMP E", 4),		INS(BIND(nop), Implied, "CMP H", 4),		INS(BIND(nop), Implied, "CMP L", 4),		INS(BIND(nop), Implied, "CMP M", 4),		INS(BIND(nop), Implied, "CMP A", 4),		//	B

		/* C */	INS(BIND(nop), Implied, "RNZ", 5),			INS(BIND(nop), Implied, "POP B", 10),		INS(BIND(nop), Absolute, "JNZ ", 10),		INS(BIND(nop), Absolute, "JMP ", 10),		INS(BIND(nop), Absolute, "CNZ ", 11),		INS(BIND(nop), Implied, "PUSH B", 11),			INS(BIND(nop), Immediate, "ADI ", 7),		INS(BIND(nop), Implied, "RST 0", 11),		INS(BIND(nop), Implied, "RZ", 11),			INS(BIND(nop), Implied, "RET", 10),			INS(BIND(nop), Absolute, "JZ ", 10),		UNKNOWN(),									INS(BIND(nop), Absolute, "CZ ", 11),		INS(BIND(nop), Absolute, "CALL ", 17),		INS(BIND(nop), Immediate, "ACI ", 7),		INS(BIND(nop), Implied, "RST 1", 11),		//	C
		/* D */	INS(BIND(nop), Implied, "RNC", 5),			INS(BIND(nop), Implied, "POP D", 10),		INS(BIND(nop), Absolute, "JNC ", 10),		INS(BIND(nop), Immediate, "OUT ", 10),		INS(BIND(nop), Absolute, "CNC ", 11),		INS(BIND(nop), Implied, "PUSH D", 11),			INS(BIND(nop), Immediate, "SUI ", 7),		INS(BIND(nop), Implied, "RST 2", 11),		INS(BIND(nop), Implied, "RC", 11),			UNKNOWN(),									INS(BIND(nop), Absolute, "JC ", 10),		INS(BIND(nop), Immediate, "IN ", 10),		INS(BIND(nop), Absolute, "CC ", 11),		UNKNOWN(),									INS(BIND(nop), Immediate, "SBI ", 7),		INS(BIND(nop), Implied, "RST 3", 11),		//	D
		/* E */	INS(BIND(nop), Implied, "RPO", 5),			INS(BIND(nop), Implied, "POP H", 10),		INS(BIND(nop), Absolute, "JPO ", 10),		INS(BIND(nop), Implied, "XHTL", 18),		INS(BIND(nop), Absolute, "CPO ", 11),		INS(BIND(nop), Implied, "PUSH H", 11),			INS(BIND(nop), Immediate, "ANI ", 7),		INS(BIND(nop), Implied, "RST 4", 11),		INS(BIND(nop), Implied, "RPE", 11),			INS(BIND(nop), Implied, "PCHL", 5),			INS(BIND(nop), Absolute, "JPE ", 10),		INS(BIND(nop), Implied, "XCHG", 4),			INS(BIND(nop), Absolute, "CPE ", 11),		UNKNOWN(),									INS(BIND(nop), Immediate, "XRI ", 7),		INS(BIND(nop), Implied, "RST 5", 11),		//	E
		/* F */	INS(BIND(nop), Implied, "RP", 5),			INS(BIND(nop), Implied, "POP PSW", 10),		INS(BIND(nop), Absolute, "JP ", 10),		INS(BIND(nop), Implied, "DI ", 4),			INS(BIND(nop), Absolute, "CP ", 11),		INS(BIND(nop), Implied, "PUSH PSW", 11),		INS(BIND(nop), Immediate, "ORI ", 7),		INS(BIND(nop), Implied, "RST 6", 11),		INS(BIND(nop), Implied, "RM", 11),			INS(BIND(nop), Implied, "SPHL", 5),			INS(BIND(nop), Absolute, "JM ", 10),		INS(BIND(nop), Implied, "EI", 4),			INS(BIND(nop), Absolute, "CM ", 11),		UNKNOWN(),									INS(BIND(nop), Immediate, "CPI ", 7),		INS(BIND(nop), Implied, "RST 7", 11),		//	F
	};
}

void EightBit::Intel8080::___() {
	m_memory.ADDRESS().word = PC().word - 1;
	auto opcode = m_memory.reference();
	auto message = Disassembler::invalid(opcode);
	throw std::domain_error(message);
}
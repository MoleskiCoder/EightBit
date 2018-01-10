#include "stdafx.h"
#include "Intel8080.h"

EightBit::Intel8080::Intel8080(Bus& bus, InputOutput& ports)
: IntelProcessor(bus),
  m_ports(ports) {
}

EightBit::register16_t& EightBit::Intel8080::AF() {
	auto& f = af.low;
	f = (f | Bit1) & ~(Bit5 | Bit3);
	return af;
}

EightBit::register16_t& EightBit::Intel8080::BC() {
	return bc;
}

EightBit::register16_t& EightBit::Intel8080::DE() {
	return de;
}

EightBit::register16_t& EightBit::Intel8080::HL() {
	return hl;
}

void EightBit::Intel8080::reset() {
	IntelProcessor::reset();
	di();
}

void EightBit::Intel8080::di() {
	m_interruptEnable = false;
}

void EightBit::Intel8080::ei() {
	m_interruptEnable = true;
}

void EightBit::Intel8080::increment(uint8_t& f, uint8_t& operand) {
	adjustSZP<Intel8080>(f, ++operand);
	clearFlag(f, AC, lowNibble(operand));
}

void EightBit::Intel8080::decrement(uint8_t& f, uint8_t& operand) {
	adjustSZP<Intel8080>(f, --operand);
	setFlag(f, AC, lowNibble(operand) != Mask4);
}

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
		UNREACHABLE;
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
		UNREACHABLE;
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
		UNREACHABLE;
	}
	throw std::logic_error("Unhandled CALL conditional");
}

void EightBit::Intel8080::add(uint8_t& f, register16_t& operand, register16_t value) {
	const auto result = operand.word + value.word;
	operand.word = result;
	setFlag(f, CF, result & Bit16);
}

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

void EightBit::Intel8080::rlc(uint8_t& f, uint8_t& operand) {
	auto carry = operand & Bit7;
	operand = (operand << 1) | (carry >> 7);
	setFlag(f, CF, carry);
}

void EightBit::Intel8080::rrc(uint8_t& f, uint8_t& operand) {
	auto carry = operand & Bit0;
	operand = (operand >> 1) | (carry << 7);
	setFlag(f, CF, carry);
}

void EightBit::Intel8080::rl(uint8_t& f, uint8_t& operand) {
	const auto carry = f & CF;
	setFlag(f, CF, operand & Bit7);
	operand = (operand << 1) | carry;
}

void EightBit::Intel8080::rr(uint8_t& f, uint8_t& operand) {
	const auto carry = f & CF;
	setFlag(f, CF, operand & Bit0);
	operand = (operand >> 1) | (carry << 7);
}

void EightBit::Intel8080::daa(uint8_t& a, uint8_t& f) {
	const auto& before = a;
	auto carry = f & CF;
	uint8_t addition = 0;
	if ((f & AC) || lowNibble(before) > 9) {
		addition = 0x6;
	}
	if ((f & CF) || highNibble(before) > 9 || (highNibble(before) >= 9 && lowNibble(before) > 9)) {
		addition |= 0x60;
		carry = true;
	}
	add(f, a, addition);
	setFlag(f, CF, carry);
}

void EightBit::Intel8080::cma(uint8_t& a, uint8_t& f) {
	a = ~a;
}

void EightBit::Intel8080::stc(uint8_t& a, uint8_t& f) {
	setFlag(f, CF);
}

void EightBit::Intel8080::cmc(uint8_t& a, uint8_t& f) {
	clearFlag(f, CF, f & CF);
}

void EightBit::Intel8080::xhtl(register16_t& operand) {
	MEMPTR().low = getByte(SP());
	setByte(operand.low);
	operand.low = MEMPTR().low;
	BUS().ADDRESS().word++;
	MEMPTR().high = getByte();
	setByte(operand.high);
	operand.high = MEMPTR().high;
}

void EightBit::Intel8080::writePort(uint8_t port, uint8_t data) {
	BUS().ADDRESS().low = port;
	BUS().ADDRESS().high = data;
	MEMPTR() = BUS().ADDRESS();
	BUS().placeDATA(data);
	writePort();
	MEMPTR().low++;
}

void EightBit::Intel8080::writePort() {
	m_ports.write(BUS().ADDRESS().low, BUS().DATA());
}

void EightBit::Intel8080::readPort(uint8_t port, uint8_t& a) {
	BUS().ADDRESS().low = port;
	BUS().ADDRESS().high = a;
	MEMPTR() = BUS().ADDRESS();
	readPort();
	a = BUS().DATA();
	MEMPTR().low++;
}

void EightBit::Intel8080::readPort() {
	BUS().placeDATA(m_ports.read(BUS().ADDRESS().low));
}

int EightBit::Intel8080::step() {
	ExecutingInstruction.fire(*this);
	resetCycles();
	if (LIKELY(powered())) {
		if (UNLIKELY(lowered(INT()))) {
			raise(HALT());
			raise(INT());
			if (m_interruptEnable) {
				di();
				return execute(BUS().DATA());
			}
		}
		if (UNLIKELY(lowered(HALT())))
			return execute(0);	// NOP
		return execute(fetchByte());
	}
	return cycles();
}

int EightBit::Intel8080::execute(uint8_t opcode) {

	const auto& decoded = getDecodedOpcode(opcode);

	const auto x = decoded.x;
	const auto y = decoded.y;
	const auto z = decoded.z;

	const auto p = decoded.p;
	const auto q = decoded.q;

	auto& af = AF();
	auto& a = af.high;
	auto& f = af.low;

	execute(a, f, x, y, z, p, q);

	if (UNLIKELY(cycles() == 0))
		throw std::logic_error("Unhandled opcode");

	return cycles();
}

void EightBit::Intel8080::execute(uint8_t& a, uint8_t& f, int x, int y, int z, int p, int q) {
	switch (x) {
	case 0:
		switch (z) {
		case 0:	// Relative jumps and assorted ops
			switch (y) {
			case 0:	// NOP
				addCycles(4);
				break;
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0: // LD rp,nn
				fetchWord(RP(p));
				addCycles(10);
				break;
			case 1:	// ADD HL,rp
				add(f, HL(), RP(p));
				addCycles(11);
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 2:	// Indirect loading
			switch (q) {
			case 0:
				switch (p) {
				case 0:	// LD (BC),A
					MEMPTR() = BC();
					setByte(MEMPTR().word++, a);
					MEMPTR().high = a;
					addCycles(7);
					break;
				case 1:	// LD (DE),A
					MEMPTR() = DE();
					setByte(MEMPTR().word++, a);
					MEMPTR().high = a;
					addCycles(7);
					break;
				case 2:	// LD (nn),HL
					fetchWord();
					setWord(HL());
					addCycles(16);
					break;
				case 3: // LD (nn),A
					fetchWord();
					setByte(MEMPTR().word++, a);
					MEMPTR().high = a;
					addCycles(13);
					break;
				default:
					UNREACHABLE;
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					MEMPTR() = BC();
					a = getByte(MEMPTR().word++);
					addCycles(7);
					break;
				case 1:	// LD A,(DE)
					MEMPTR() = DE();
					a = getByte(MEMPTR().word++);
					addCycles(7);
					break;
				case 2:	// LD HL,(nn)
					fetchWord();
					getWord(HL());
					addCycles(16);
					break;
				case 3:	// LD A,(nn)
					fetchWord();
					a = getByte(MEMPTR().word++);
					addCycles(13);
					break;
				default:
					UNREACHABLE;
				}
				break;
			default:
				UNREACHABLE;
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
				UNREACHABLE;
			}
			addCycles(6);
			break;
		case 4: { // 8-bit INC
			auto operand = R(y, a);
			increment(f, operand);
			R(y, a, operand);
			addCycles(4);
			break;
		} case 5: {	// 8-bit DEC
			auto operand = R(y, a);
			decrement(f, operand);
			R(y, a, operand);
			addCycles(4);
			if (UNLIKELY(y == 6))
				addCycles(7);
			break;
		} case 6:	// 8-bit load immediate
			R(y, a, fetchByte());
			addCycles(7);
			if (UNLIKELY(y == 6))
				addCycles(3);
			break;
		case 7:	// Assorted operations on accumulator/flags
			switch (y) {
			case 0:
				rlc(f, a);
				break;
			case 1:
				rrc(f, a);
				break;
			case 2:
				rl(f, a);
				break;
			case 3:
				rr(f, a);
				break;
			case 4:
				daa(a, f);
				break;
			case 5:
				cma(a, f);
				break;
			case 6:
				stc(a, f);
				break;
			case 7:
				cmc(a, f);
				break;
			default:
				UNREACHABLE;
			}
			addCycles(4);
			break;
		default:
			UNREACHABLE;
		}
		break;
	case 1:	// 8-bit loading
		if (UNLIKELY(z == 6 && y == 6)) { 	// Exception (replaces LD (HL), (HL))
			halt();
		} else {
			R(y, a, R(z, a));
			if (UNLIKELY((y == 6) || (z == 6)))	// M operations
				addCycles(3);
		}
		addCycles(4);
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
			sbb(f, a, R(z, a));
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
			UNREACHABLE;
		}
		addCycles(4);
		if (UNLIKELY(z == 6))
			addCycles(3);
		break;
	case 3:
		switch (z) {
		case 0:	// Conditional return
			if (returnConditionalFlag(f, y))
				addCycles(6);
			addCycles(5);
			break;
		case 1:	// POP & various ops
			switch (q) {
			case 0:	// POP rp2[p]
				popWord(RP2(p));
				addCycles(10);
				break;
			case 1:
				switch (p) {
				case 0:	// RET
					ret();
					addCycles(10);
					break;
				case 2:	// JP HL
					PC() = HL();
					addCycles(4);
					break;
				case 3:	// LD SP,HL
					SP() = HL();
					addCycles(4);
					break;
				}
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 2:	// Conditional jump
			jumpConditionalFlag(f, y);
			addCycles(10);
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0:	// JP nn
				fetchWord();
				jump();
				addCycles(10);
				break;
			case 2:	// OUT (n),A
				writePort(fetchByte(), a);
				addCycles(11);
				break;
			case 3:	// IN A,(n)
				readPort(fetchByte(), a);
				addCycles(11);
				break;
			case 4:	// EX (SP),HL
				xhtl(HL());
				addCycles(19);
				break;
			case 5:	// EX DE,HL
				std::swap(DE(), HL());
				addCycles(4);
				break;
			case 6:	// DI
				di();
				addCycles(4);
				break;
			case 7:	// EI
				ei();
				addCycles(4);
				break;
			}
			break;
		case 4:	// Conditional call: CALL cc[y], nn
			if (callConditionalFlag(f, y))
				addCycles(7);
			addCycles(10);
			break;
		case 5:	// PUSH & various ops
			switch (q) {
			case 0:	// PUSH rp2[p]
				pushWord(RP2(p));
				addCycles(11);
				break;
			case 1:
				switch (p) {
				case 0:	// CALL nn
					fetchWord();
					call();
					addCycles(17);
					break;
				}
				break;
			default:
				UNREACHABLE;
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
				UNREACHABLE;
			}
			addCycles(7);
			break;
		case 7:	// Restart: RST y * 8
			restart(y << 3);
			addCycles(11);
			break;
		default:
			UNREACHABLE;
		}
		break;
	}
}

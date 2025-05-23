#include "stdafx.h"
#include "../inc/Intel8080.h"

EightBit::Intel8080::Intel8080(Bus& bus)
: IntelProcessor(bus) {

	RaisedPOWER.connect([this](EventArgs) {

		releaseMemory();
		releaseIO();

		releaseRead();
		releaseWrite();

		di();
	});
}

DEFINE_PIN_LEVEL_CHANGERS(DBIN, Intel8080);
DEFINE_PIN_LEVEL_CHANGERS(WR, Intel8080);

const EightBit::register16_t& EightBit::Intel8080::AF() const noexcept {
	auto* processor = const_cast<Intel8080*>(this);
	processor->af.low = (af.low | Bit1) & ~(Bit5 | Bit3);
	return af;
}

const EightBit::register16_t& EightBit::Intel8080::BC() const noexcept {
	return bc;
}

const EightBit::register16_t& EightBit::Intel8080::DE() const noexcept {
	return de;
}

const EightBit::register16_t& EightBit::Intel8080::HL() const noexcept {
	return hl;
}

void EightBit::Intel8080::memoryWrite() noexcept {
	requestMemory();
	IntelProcessor::memoryWrite();
	releaseMemory();
}

uint8_t EightBit::Intel8080::memoryRead() noexcept {
	requestMemory();
	const auto returned = IntelProcessor::memoryRead();
	releaseMemory();
	return returned;
}

void EightBit::Intel8080::busWrite() noexcept {
	requestWrite();
	IntelProcessor::busWrite();
	releaseWrite();
}

uint8_t EightBit::Intel8080::busRead() noexcept {
	requestRead();
	const auto returned = IntelProcessor::busRead();
	releaseRead();
	return returned;
}

void EightBit::Intel8080::handleRESET() noexcept {
	IntelProcessor::handleRESET();
	di();
	tick(3);
}

void EightBit::Intel8080::handleINT() noexcept {
	IntelProcessor::handleINT();
	raiseHALT();
	if (m_interruptEnable) {
		di();
		Processor::execute(BUS().DATA());
	}
}

void EightBit::Intel8080::di() {
	m_interruptEnable = false;
}

void EightBit::Intel8080::ei() {
	m_interruptEnable = true;
}

void EightBit::Intel8080::increment(uint8_t& operand) {
	F() = adjustSZP<Intel8080>(F(), ++operand);
	F() = clearBit(F(), AC, lowNibble(operand));
}

void EightBit::Intel8080::decrement(uint8_t& operand) {
	F() = adjustSZP<Intel8080>(F(), --operand);
	F() = setBit(F(), AC, lowNibble(operand) != Mask4);
}

int EightBit::Intel8080::jumpConditionalFlag(const int flag) {
	switch (flag) {
	case 0:	// NZ
		return jumpConditional(!(F() & ZF));
	case 1:	// Z
		return jumpConditional(F() & ZF);
	case 2:	// NC
		return jumpConditional(!(F() & CF));
	case 3:	// C
		return jumpConditional(F() & CF);
	case 4:	// PO
		return jumpConditional(!(F() & PF));
	case 5:	// PE
		return jumpConditional(F() & PF);
	case 6:	// P
		return jumpConditional(!(F() & SF));
	case 7:	// M
		return jumpConditional(F() & SF);
	default:
		UNREACHABLE;
	}
}

int EightBit::Intel8080::returnConditionalFlag(const int flag) {
	switch (flag) {
	case 0:	// NZ
		return returnConditional(!(F() & ZF));
	case 1:	// Z
		return returnConditional(F() & ZF);
	case 2:	// NC
		return returnConditional(!(F() & CF));
	case 3:	// C
		return returnConditional(F() & CF);
	case 4:	// PO
		return returnConditional(!(F() & PF));
	case 5:	// PE
		return returnConditional(F() & PF);
	case 6:	// P
		return returnConditional(!(F() & SF));
	case 7:	// M
		return returnConditional(F() & SF);
	default:
		UNREACHABLE;
	}
}

int EightBit::Intel8080::callConditionalFlag(const int flag) {
	switch (flag) {
	case 0:	// NZ
		return callConditional(!(F() & ZF));
	case 1:	// Z
		return callConditional(F() & ZF);
	case 2:	// NC
		return callConditional(!(F() & CF));
	case 3:	// C
		return callConditional(F() & CF);
	case 4:	// PO
		return callConditional(!(F() & PF));
	case 5:	// PE
		return callConditional(F() & PF);
	case 6:	// P
		return callConditional(!(F() & SF));
	case 7:	// M
		return callConditional(F() & SF);
	default:
		UNREACHABLE;
	}
}

void EightBit::Intel8080::add(const register16_t value) {
	const auto result = HL().word + value.word;
	HL() = result;
	F() = setBit(F(), CF, result & Bit16);
}

void EightBit::Intel8080::add(const uint8_t value, const int carry) {

	const register16_t result = A() + value + carry;

	adjustAuxiliaryCarryAdd(F(), A(), value, result.word);

	A() = result.low;

	F() = setBit(F(), CF, result.word & Bit8);
	F() = adjustSZP<Intel8080>(F(), A());
}

void EightBit::Intel8080::adc(const uint8_t value) {
	add(value, F() & CF);
}

void EightBit::Intel8080::subtract(uint8_t& operand, const uint8_t value, const int carry) {

	const register16_t result = operand - value - carry;

	adjustAuxiliaryCarrySub(F(), operand, value, result.word);

	operand = result.low;

	F() = setBit(F(), CF, result.word & Bit8);
	F() = adjustSZP<Intel8080>(F(), operand);
}

void EightBit::Intel8080::sbb(const uint8_t value) {
	subtract(A(), value, F() & CF);
}

void EightBit::Intel8080::andr(const uint8_t value) {
	F() = setBit(F(), AC, (A() | value) & Bit3);
	F() = clearBit(F(), CF);
	F() = adjustSZP<Intel8080>(F(), A() &= value);
}

void EightBit::Intel8080::xorr(const uint8_t value) {
	F() = clearBit(F(), AC | CF);
	F() = adjustSZP<Intel8080>(F(), A() ^= value);
}

void EightBit::Intel8080::orr(const uint8_t value) {
	F() = clearBit(F(), AC | CF);
	F() = adjustSZP<Intel8080>(F(), A() |= value);
}

void EightBit::Intel8080::compare(const uint8_t value) {
	auto original = A();
	subtract(original, value);
}

void EightBit::Intel8080::rlc() {
	const auto carry = A() & Bit7;
	A() = (A() << 1) | (carry >> 7);
	F() = setBit(F(), CF, carry);
}

void EightBit::Intel8080::rrc() {
	const auto carry = A() & Bit0;
	A() = (A() >> 1) | (carry << 7);
	F() = setBit(F(), CF, carry);
}

void EightBit::Intel8080::rl() {
	const auto carry = F() & CF;
	F() = setBit(F(), CF, A() & Bit7);
	A() = (A() << 1) | carry;
}

void EightBit::Intel8080::rr() {
	const auto carry = F() & CF;
	F() = setBit(F(), CF, A() & Bit0);
	A() = (A() >> 1) | (carry << 7);
}

void EightBit::Intel8080::daa() {
	const auto& before = A();
	auto carry = F() & CF;
	uint8_t addition = 0;
	if ((F() & AC) || lowNibble(before) > 9) {
		addition = 0x6;
	}
	if ((F() & CF) || highNibble(before) > 9 || (highNibble(before) >= 9 && lowNibble(before) > 9)) {
		addition |= 0x60;
		carry = true;
	}
	add(addition);
	F() = setBit(F(), CF, carry);
}

void EightBit::Intel8080::cma() {
	A() = ~A();
}

void EightBit::Intel8080::stc() {
	F() = setBit(F(), CF);
}

void EightBit::Intel8080::cmc() {
	F() = clearBit(F(), CF, F() & CF);
}

void EightBit::Intel8080::xhtl(register16_t& exchange) {
	MEMPTR().low = IntelProcessor::memoryRead(SP());
	++BUS().ADDRESS();
	MEMPTR().high = memoryRead();
	IntelProcessor::memoryWrite(exchange.high);
	exchange.high = MEMPTR().high;
	--BUS().ADDRESS();
	IntelProcessor::memoryWrite(exchange.low);
	exchange.low = MEMPTR().low;
}

void EightBit::Intel8080::portWrite(const uint8_t port) {
	BUS().ADDRESS() = { port, port };
	BUS().DATA() = A();
	portWrite();
}

void EightBit::Intel8080::portWrite() {
	requestIO();
	busWrite();
	releaseIO();;
}

uint8_t EightBit::Intel8080::portRead(const uint8_t port) {
	BUS().ADDRESS() = { port, port };
	return portRead();
}

uint8_t EightBit::Intel8080::portRead() {
	requestIO();
	const auto returned = busRead();
	releaseIO();;
	return returned;
}

int EightBit::Intel8080::step() noexcept {
	ExecutingInstruction.fire(*this);
	resetCycles();
	if (LIKELY(powered())) {
		if (UNLIKELY(lowered(RESET()))) {
			handleRESET();
		} else if (UNLIKELY(lowered(INT()))) {
			handleINT();
		} else if (UNLIKELY(lowered(HALT()))) {
			Processor::execute(0);	// NOP
		} else {
			Processor::execute(fetchByte());
		}
	}
	ExecutedInstruction.fire(*this);
	ASSUME(cycles() > 0);
	return cycles();
}

void EightBit::Intel8080::execute() noexcept {

	const auto& decoded = getDecodedOpcode(opcode());

	const auto x = decoded.x;
	const auto y = decoded.y;
	const auto z = decoded.z;

	const auto p = decoded.p;
	const auto q = decoded.q;

	execute(x, y, z, p, q);
}

void EightBit::Intel8080::execute(const int x, const int y, const int z, const int p, const int q) {
	switch (x) {
	case 0:
		switch (z) {
		case 0:	// Relative jumps and assorted ops
			switch (y) {
			case 0:	// NOP
				tick(4);
				break;
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0: // LD rp,nn
				RP(p) = fetchWord();
				tick(10);
				break;
			case 1:	// ADD HL,rp
				add(RP(p));
				tick(11);
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
					IntelProcessor::memoryWrite(BC(), A());
					tick(7);
					break;
				case 1:	// LD (DE),A
					IntelProcessor::memoryWrite(DE(), A());
					tick(7);
					break;
				case 2:	// LD (nn),HL
					BUS().ADDRESS() = fetchWord();
					setWord(HL());
					tick(16);
					break;
				case 3: // LD (nn),A
					BUS().ADDRESS() = fetchWord();
					IntelProcessor::memoryWrite(A());
					tick(13);
					break;
				default:
					UNREACHABLE;
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					A() = IntelProcessor::memoryRead(BC());
					tick(7);
					break;
				case 1:	// LD A,(DE)
					A() = IntelProcessor::memoryRead(DE());
					tick(7);
					break;
				case 2:	// LD HL,(nn)
					BUS().ADDRESS() = fetchWord();
					HL() = getWord();
					tick(16);
					break;
				case 3:	// LD A,(nn)
					BUS().ADDRESS() = fetchWord();
					A() = memoryRead();
					tick(13);
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
				++RP(p);
				break;
			case 1:	// DEC rp
				--RP(p);
				break;
			default:
				UNREACHABLE;
			}
			tick(6);
			break;
		case 4: { // 8-bit INC
			auto operand = R(y);
			increment(operand);
			R(y, operand);
			tick(4);
			break;
		} case 5: {	// 8-bit DEC
			auto operand = R(y);
			decrement(operand);
			R(y, operand);
			tick(4);
			if (UNLIKELY(y == 6))
				tick(7);
			break;
		} case 6:	// 8-bit load immediate
			R(y, fetchByte());
			tick(7);
			if (UNLIKELY(y == 6))
				tick(3);
			break;
		case 7:	// Assorted operations on accumulator/flags
			switch (y) {
			case 0:
				rlc();
				break;
			case 1:
				rrc();
				break;
			case 2:
				rl();
				break;
			case 3:
				rr();
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
				UNREACHABLE;
			}
			tick(4);
			break;
		default:
			UNREACHABLE;
		}
		break;
	case 1:	// 8-bit loading
		if (UNLIKELY(z == 6 && y == 6)) { 	// Exception (replaces LD (HL), (HL))
			lowerHALT();
		} else {
			R(y, R(z));
			if (UNLIKELY((y == 6) || (z == 6)))	// M operations
				tick(3);
		}
		tick(4);
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
			subtract(A(), R(z));
			break;
		case 3:	// SBC A,r
			sbb(R(z));
			break;
		case 4:	// AND r
			andr(R(z));
			break;
		case 5:	// XOR r
			xorr(R(z));
			break;
		case 6:	// OR r
			orr(R(z));
			break;
		case 7:	// CP r
			compare(R(z));
			break;
		default:
			UNREACHABLE;
		}
		tick(4);
		if (UNLIKELY(z == 6))
			tick(3);
		break;
	case 3:
		switch (z) {
		case 0:	// Conditional return
			if (returnConditionalFlag(y))
				tick(6);
			tick(5);
			break;
		case 1:	// POP & various ops
			switch (q) {
			case 0:	// POP rp2[p]
				RP2(p) = popWord();
				tick(10);
				break;
			case 1:
				switch (p) {
				case 0:	// RET
					ret();
					tick(10);
					break;
				case 2:	// JP HL
					jump(HL());
					tick(4);
					break;
				case 3:	// LD SP,HL
					SP() = HL();
					tick(4);
					break;
				}
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 2:	// Conditional jump
			jumpConditionalFlag(y);
			tick(10);
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0: // JP nn
				jump(fetchWord());
				tick(10);
				break;
			case 2:	// OUT (n),A
				portWrite(fetchByte());
				tick(11);
				break;
			case 3:	// IN A,(n)
				A() = portRead(fetchByte());
				tick(11);
				break;
			case 4:	// EX (SP),HL
				xhtl(HL());
				tick(19);
				break;
			case 5:	// EX DE,HL
				std::swap(DE(), HL());
				tick(4);
				break;
			case 6:	// DI
				di();
				tick(4);
				break;
			case 7:	// EI
				ei();
				tick(4);
				break;
			}
			break;
		case 4:	// Conditional call: CALL cc[y], nn
			if (callConditionalFlag(y))
				tick(7);
			tick(10);
			break;
		case 5:	// PUSH & various ops
			switch (q) {
			case 0:	// PUSH rp2[p]
				pushWord(RP2(p));
				tick(11);
				break;
			case 1:
				switch (p) {
				case 0:	// CALL nn
					call(fetchWord());
					tick(17);
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
				add(fetchByte());
				break;
			case 1:	// ADC A,n
				adc(fetchByte());
				break;
			case 2:	// SUB n
				subtract(A(), fetchByte());
				break;
			case 3:	// SBC A,n
				sbb(fetchByte());
				break;
			case 4:	// AND n
				andr(fetchByte());
				break;
			case 5:	// XOR n
				xorr(fetchByte());
				break;
			case 6:	// OR n
				orr(fetchByte());
				break;
			case 7:	// CP n
				compare(fetchByte());
				break;
			default:
				UNREACHABLE;
			}
			tick(7);
			break;
		case 7:	// Restart: RST y * 8
			restart(y << 3);
			tick(11);
			break;
		default:
			UNREACHABLE;
		}
		break;
	}
}

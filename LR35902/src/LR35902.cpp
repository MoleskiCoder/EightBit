#include "stdafx.h"
#include "../inc/LR35902.h"
#include "../inc/GameBoyBus.h"

// based on http://www.z80.info/decoding.htm

EightBit::GameBoy::LR35902::LR35902(Bus& memory)
: IntelProcessor(memory),
  m_bus(memory) {
	MachineTicked.connect([this](EventArgs) {
		m_bus.IO().incrementTimers();
		m_bus.IO().transferDma();
	});
}

const EightBit::register16_t& EightBit::GameBoy::LR35902::AF() const noexcept {
	auto* gb = const_cast<LR35902*>(this);
	gb->af.low = higherNibble(af.low);
	return af;
}

const EightBit::register16_t& EightBit::GameBoy::LR35902::BC() const noexcept {
	return bc;
}

const EightBit::register16_t& EightBit::GameBoy::LR35902::DE() const noexcept {
	return de;
}

const EightBit::register16_t& EightBit::GameBoy::LR35902::HL() const noexcept {
	return hl;
}

void EightBit::GameBoy::LR35902::handleRESET() noexcept {
	IntelProcessor::handleRESET();
	di();
	SP() = Mask16 - 1;
	tickMachine(4);
}

void EightBit::GameBoy::LR35902::handleINT() noexcept {
	IntelProcessor::handleINT();
	raiseHALT();
	di();
	restart(BUS().DATA());
}

void EightBit::GameBoy::LR35902::memoryWrite() noexcept {
	tickMachine();
	IntelProcessor::memoryWrite();
}

uint8_t EightBit::GameBoy::LR35902::memoryRead() noexcept {
	tickMachine();
	return IntelProcessor::memoryRead();
}

void EightBit::GameBoy::LR35902::pushWord(register16_t value) noexcept {
	tickMachine();
	IntelProcessor::pushWord(value);
}

void EightBit::GameBoy::LR35902::jr(int8_t offset) noexcept {
	IntelProcessor::jr(offset);
	tickMachine();
}

int EightBit::GameBoy::LR35902::jumpConditional(const int condition) noexcept {
	if (IntelProcessor::jumpConditional(condition))
		tickMachine();
	return condition;
}

int EightBit::GameBoy::LR35902::returnConditional(const int condition) noexcept {
	IntelProcessor::returnConditional(condition);
	tickMachine();
	return condition;
}

int EightBit::GameBoy::LR35902::jrConditional(const int condition) noexcept {
	if (!IntelProcessor::jrConditional(condition))
		tickMachine();
	return condition;
}

void EightBit::GameBoy::LR35902::ret() noexcept {
	IntelProcessor::ret();
	tickMachine();
}

void EightBit::GameBoy::LR35902::di() noexcept {
	IME() = false;
}

void EightBit::GameBoy::LR35902::ei() noexcept {
	IME() = true;
}

uint8_t EightBit::GameBoy::LR35902::increment(uint8_t& f, const uint8_t operand) noexcept {
	f = clearBit(f, NF);
	const uint8_t result = operand + 1;
	f = adjustZero<LR35902>(f, result);
	f = clearBit(f, HC, lowNibble(result));
	return result;
}

uint8_t EightBit::GameBoy::LR35902::decrement(uint8_t& f, const uint8_t operand) noexcept {
	f = setBit(f, NF);
	f = clearBit(f, HC, lowNibble(operand));
	const uint8_t result = operand - 1;
	f = adjustZero<LR35902>(f, result);
	return result;
}

uint8_t EightBit::GameBoy::LR35902::R(const int r) {
	switch (r) {
	case 0:
		return B();
	case 1:
		return C();
	case 2:
		return D();
	case 3:
		return E();
	case 4:
		return H();
	case 5:
		return L();
	case 6:
		return IntelProcessor::memoryRead(HL());
	case 7:
		return A();
	default:
		UNREACHABLE;
	}
}

void EightBit::GameBoy::LR35902::R(const int r, const uint8_t value) {
	switch (r) {
	case 0:
		B() = value;
		break;
	case 1:
		C() = value;
		break;
	case 2:
		D() = value;
		break;
	case 3:
		E() = value;
		break;
	case 4:
		H() = value;
		break;
	case 5:
		L() = value;
		break;
	case 6:
		IntelProcessor::memoryWrite(HL(), value);
		break;
	case 7:
		A() = value;
		break;
	default:
		UNREACHABLE;
	}
}

EightBit::register16_t& EightBit::GameBoy::LR35902::RP(const int rp) noexcept {
	switch (rp) {
	case 0b00:
		return BC();
	case 0b01:
		return DE();
	case 0b10:
		return HL();
	case 0b11:
		return SP();
	default:
		UNREACHABLE;
	}
}

EightBit::register16_t& EightBit::GameBoy::LR35902::RP2(const int rp) noexcept {
	switch (rp) {
	case 0b00:
		return BC();
	case 0b01:
		return DE();
	case 0b10:
		return HL();
	case 0b11:
		return AF();
	default:
		UNREACHABLE;
	}
}

bool EightBit::GameBoy::LR35902::convertCondition(const uint8_t f, int flag) noexcept {
	ASSUME(flag >= 0);
	ASSUME(flag <= 7);
	switch (flag) {
	case 0:
		return !(f & ZF);
	case 1:
		return f & ZF;
	case 2:
		return !(f & CF);
	case 3:
		return f & CF;
	default:
		UNREACHABLE;
	}
}

void EightBit::GameBoy::LR35902::returnConditionalFlag(const uint8_t f, const int flag) {
	returnConditional(convertCondition(f, flag));
}

void EightBit::GameBoy::LR35902::jrConditionalFlag(const uint8_t f, const int flag) {
	jrConditional(convertCondition(f, flag));
}

void EightBit::GameBoy::LR35902::jumpConditionalFlag(uint8_t f, const int flag) {
	jumpConditional(convertCondition(f, flag));
}

void EightBit::GameBoy::LR35902::callConditionalFlag(uint8_t f, const int flag) {
	callConditional(convertCondition(f, flag));
}

void EightBit::GameBoy::LR35902::reti() {
	ret();
	ei();
}

EightBit::register16_t EightBit::GameBoy::LR35902::add(uint8_t& f, const register16_t operand, const register16_t value) {

	tickMachine();

	const int addition = operand.word + value.word;
	const register16_t result = addition;

	f = clearBit(f, NF);
	f = setBit(f, CF, addition & Bit16);
	f = adjustHalfCarryAdd(f, operand.high, value.high, result.high);

	MEMPTR() = operand + 1;

	return result;
}

uint8_t EightBit::GameBoy::LR35902::add(uint8_t& f, const uint8_t operand, const uint8_t value, const int carry) noexcept {

	const register16_t addition = operand + value + carry;
	const auto result = addition.low;

	f = adjustHalfCarryAdd(f, operand, value, result);

	f = clearBit(f, NF);
	f = setBit(f, CF, addition.high & Bit0);
	f = adjustZero<LR35902>(f, result);

	return result;
}

uint8_t EightBit::GameBoy::LR35902::adc(uint8_t& f, const uint8_t operand, const uint8_t value) noexcept {
	return add(f, operand, value, (f & CF) >> 4);
}

uint8_t EightBit::GameBoy::LR35902::subtract(uint8_t& f, const uint8_t operand, const uint8_t value, const int carry) noexcept {

	const register16_t subtraction = operand - value - carry;
	const auto result = subtraction.low;

	f = adjustHalfCarrySub(f, operand, value, result);

	f = setBit(f, NF);
	f = setBit(f, CF, subtraction.high & Bit0);
	f = adjustZero<LR35902>(f, result);

	return result;
}

uint8_t EightBit::GameBoy::LR35902::sbc(uint8_t& f, const uint8_t operand, const uint8_t value) noexcept {
	 return subtract(f, operand, value, (f & CF) >> 4);
}

uint8_t EightBit::GameBoy::LR35902::andr(uint8_t& f, const uint8_t operand, const uint8_t value) noexcept {
	f = setBit(f, HC);
	f = clearBit(f, CF | NF);
	const uint8_t result = operand & value;
	f = adjustZero<LR35902>(f, result);
	return result;
}

uint8_t EightBit::GameBoy::LR35902::xorr(uint8_t& f, const uint8_t operand, const uint8_t value) noexcept {
	f = clearBit(f, HC | CF | NF);
	const uint8_t result = operand ^ value;
	f = adjustZero<LR35902>(f, result);
	return result;
}

uint8_t EightBit::GameBoy::LR35902::orr(uint8_t& f, const uint8_t operand, const uint8_t value) noexcept {
	f = clearBit(f, HC | CF | NF);
	const uint8_t result = operand | value;
	f = adjustZero<LR35902>(f, result);
	return result;
}

void EightBit::GameBoy::LR35902::compare(uint8_t& f, uint8_t operand, const uint8_t value) noexcept {
	subtract(f, operand, value);
}

uint8_t EightBit::GameBoy::LR35902::rlc(uint8_t& f, const uint8_t operand) noexcept {
	f = clearBit(f, NF | HC | ZF);
	const auto carry = operand & Bit7;
	f = setBit(f, CF, carry);
	return (operand << 1) | (carry >> 7);
}

uint8_t EightBit::GameBoy::LR35902::rrc(uint8_t& f, const uint8_t operand) noexcept {
	f = clearBit(f, NF | HC | ZF);
	const auto carry = operand & Bit0;
	f = setBit(f, CF, carry);
	return (operand >> 1) | (carry << 7);
}

uint8_t EightBit::GameBoy::LR35902::rl(uint8_t& f, const uint8_t operand) noexcept {
	f = clearBit(f, NF | HC | ZF);
	const auto carry = f & CF;
	f = setBit(f, CF, operand & Bit7);
	return (operand << 1) | (carry >> 4);	// CF at Bit4
}

uint8_t EightBit::GameBoy::LR35902::rr(uint8_t& f, const uint8_t operand) noexcept {
	f = clearBit(f, NF | HC | ZF);
	const auto carry = f & CF;
	f = setBit(f, CF, operand & Bit0);
	return (operand >> 1) | (carry << 3);	// CF at Bit4
}

uint8_t EightBit::GameBoy::LR35902::sla(uint8_t& f, const uint8_t operand) noexcept {
	f = clearBit(f, NF | HC | ZF);
	f = setBit(f, CF, operand & Bit7);
	return operand << 1;
}

uint8_t EightBit::GameBoy::LR35902::sra(uint8_t& f, const uint8_t operand) noexcept {
	f = clearBit(f, NF | HC | ZF);
	f = setBit(f, CF, operand & Bit0);
	return (operand >> 1) | (operand & Bit7);
}

uint8_t EightBit::GameBoy::LR35902::swap(uint8_t& f, const uint8_t operand) noexcept {
	f = clearBit(f, NF | HC | CF);
	return promoteNibble(operand) | demoteNibble(operand);
}

uint8_t EightBit::GameBoy::LR35902::srl(uint8_t& f, const uint8_t operand) noexcept {
	f = clearBit(f, NF | HC | ZF);
	f = setBit(f, CF, operand & Bit0);
	return (operand >> 1) & ~Bit7;
}

void EightBit::GameBoy::LR35902::bit(uint8_t& f, const int n, const uint8_t operand) noexcept {
	ASSUME(n >= 0);
	ASSUME(n <= 7);
	const auto carry = f & CF;
	andr(f, operand, Chip::bit(n));
	f = setBit(f, CF, carry);
}

uint8_t EightBit::GameBoy::LR35902::res(const int n, const uint8_t operand) noexcept {
	ASSUME(n >= 0);
	ASSUME(n <= 7);
	return clearBit(operand, Chip::bit(n));
}

uint8_t EightBit::GameBoy::LR35902::set(const int n, const uint8_t operand) noexcept {
	ASSUME(n >= 0);
	ASSUME(n <= 7);
	return setBit(operand, Chip::bit(n));
}

uint8_t EightBit::GameBoy::LR35902::daa(uint8_t& f, uint8_t operand) noexcept {

	int updated = operand;

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

	f = clearBit(f, HC | ZF);
	f = setBit(f, CF, (f & CF) || (updated & Bit8));
	const uint8_t result = updated & Mask8;

	f = adjustZero<LR35902>(f, result);

	return result;
}

uint8_t EightBit::GameBoy::LR35902::cpl(uint8_t& f, const uint8_t operand) noexcept {
	f = setBit(f, HC | NF);
	return ~operand;
}

void EightBit::GameBoy::LR35902::scf(uint8_t& f, const uint8_t operand) noexcept {
	f = setBit(f, CF);
	f = clearBit(f, HC | NF);
}

void EightBit::GameBoy::LR35902::ccf(uint8_t& f, const uint8_t operand) noexcept {
	f = clearBit(f, NF | HC);
	f = clearBit(f, CF, f & CF);
}

uint8_t EightBit::GameBoy::LR35902::enabledInterrupts() noexcept {
	return BUS().peek(IoRegisters::BASE + IoRegisters::IE);
}

uint8_t EightBit::GameBoy::LR35902::flaggedInterrupts() noexcept {
	return m_bus.IO().peek(IoRegisters::IF);
}

uint8_t EightBit::GameBoy::LR35902::maskedInterrupts() noexcept {
	return enabledInterrupts() & flaggedInterrupts();
}

int EightBit::GameBoy::LR35902::step() noexcept {

	ExecutingInstruction.fire(*this);
	m_prefixCB = false;
	resetCycles();
	if (LIKELY(powered())) {

		const auto masked = maskedInterrupts();
		if (masked) {
			if (IME()) {
				m_bus.IO().poke(IoRegisters::IF, 0);
				lowerINT();
				const int index = EightBit::findFirstSet(masked);
				BUS().DATA() = 0x38 + (index << 3);
			} else {
				raiseHALT();
			}
		}

		if (UNLIKELY(lowered(RESET()))) {
			handleRESET();
		} else if (UNLIKELY(lowered(INT()))) {
			handleINT();
		} else if (UNLIKELY(lowered(HALT()))) {
			IntelProcessor::memoryRead(PC());
			Processor::execute(0);	// NOP
		} else {
			Processor::execute(fetchByte());
		}
	}
	ExecutedInstruction.fire(*this);
	assert(cycles() % 4 == 0);
	return cycles();
}

int EightBit::GameBoy::LR35902::execute() noexcept {

	const auto& decoded = getDecodedOpcode(opcode());

	const auto x = decoded.x;
	const auto y = decoded.y;
	const auto z = decoded.z;

	const auto p = decoded.p;
	const auto q = decoded.q;

	if (LIKELY(!m_prefixCB))
		executeOther(x, y, z, p, q);
	else
		executeCB(x, y, z, p, q);

	assert(cycles() > 0);

	return cycles();
}

void EightBit::GameBoy::LR35902::executeCB(const int x, const int y, const int z, int, int) {
	switch (x) {
	case 0:	{ // rot[y] r[z]
		auto operand = R(z);
		switch (y) {
		case 0:
			operand = rlc(F(), operand);
			break;
		case 1:
			operand = rrc(F(), operand);
			break;
		case 2:
			operand = rl(F(), operand);
			break;
		case 3:
			operand = rr(F(), operand);
			break;
		case 4:
			operand = sla(F(), operand);
			break;
		case 5:
			operand = sra(F(), operand);
			break;
		case 6:	// GB: SWAP r
			operand = swap(F(), operand);
			break;
		case 7:
			operand = srl(F(), operand);
			break;
		default:
			UNREACHABLE;
		}
		R(z, operand);
		F() = adjustZero<LR35902>(F(), operand);
		break;
	} case 1:	// BIT y, r[z]
		bit(F(), y, R(z));
		break;
	case 2:	// RES y, r[z]
		R(z, res(y, R(z)));
		break;
	case 3:	// SET y, r[z]
		R(z, set(y, R(z)));
		break;
	default:
		UNREACHABLE;
	}
}

void EightBit::GameBoy::LR35902::executeOther(const int x, const int y, const int z, const int p, const int q) {
	switch (x) {
	case 0:
		switch (z) {
		case 0:	// Relative jumps and assorted ops
			switch (y) {
			case 0:	// NOP
				break;
			case 1:	// GB: LD (nn),SP
				BUS().ADDRESS() = fetchWord();
				setWord(SP());
				break;
			case 2:	// GB: STOP
				stop();
				break;
			case 3:	// JR d
				jr(fetchByte());
				break;
			case 4: // JR cc,d
			case 5:
			case 6:
			case 7:
				jrConditionalFlag(F(), y - 4);
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0: // LD rp,nn
				RP(p) = fetchWord();
				break;
			case 1:	// ADD HL,rp
				HL() = add(F(), HL(), RP(p));
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
					break;
				case 1:	// LD (DE),A
					IntelProcessor::memoryWrite(DE(), A());
					break;
				case 2:	// GB: LDI (HL),A
					IntelProcessor::memoryWrite(HL()++, A());
					break;
				case 3: // GB: LDD (HL),A
					IntelProcessor::memoryWrite(HL()--, A());
					break;
				default:
					UNREACHABLE;
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					A() = IntelProcessor::memoryRead(BC());
					break;
				case 1:	// LD A,(DE)
					A() = IntelProcessor::memoryRead(DE());
					break;
				case 2:	// GB: LDI A,(HL)
					A() = IntelProcessor::memoryRead(HL()++);
					break;
				case 3:	// GB: LDD A,(HL)
					A() = IntelProcessor::memoryRead(HL()--);
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
			tickMachine();
			break;
		case 4: { // 8-bit INC
			auto operand = R(y);
			R(y, increment(F(), operand));
			break;
		} case 5: {	// 8-bit DEC
			auto operand = R(y);
			R(y, decrement(F(), operand));
			break;
		} case 6:	// 8-bit load immediate
			R(y, fetchByte());	// LD r,n
			break;
		case 7:	// Assorted operations on accumulator/flags
			switch (y) {
			case 0:
				A() = rlc(F(), A());
				break;
			case 1:
				A() = rrc(F(), A());
				break;
			case 2:
				A() = rl(F(), A());
				break;
			case 3:
				A() = rr(F(), A());
				break;
			case 4:
				A() = daa(F(), A());
				break;
			case 5:
				A() = cpl(F(), A());
				break;
			case 6:
				scf(F(), A());
				break;
			case 7:
				ccf(F(), A());
				break;
			default:
				UNREACHABLE;
			}
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
		}
		break;
	case 2:	// Operate on accumulator and register/memory location
		switch (y) {
		case 0:	// ADD A,r
			A() = add(F(), A(), R(z));
			break;
		case 1:	// ADC A,r
			A() = adc(F(), A(), R(z));
			break;
		case 2:	// SUB r
			A() = subtract(F(), A(), R(z));
			break;
		case 3:	// SBC A,r
			A() = sbc(F(), A(), R(z));
			break;
		case 4:	// AND r
			A() = andr(F(), A(), R(z));
			break;
		case 5:	// XOR r
			A() = xorr(F(), A(), R(z));
			break;
		case 6:	// OR r
			A() = orr(F(), A(), R(z));
			break;
		case 7:	// CP r
			compare(F(), A(), R(z));
			break;
		default:
			UNREACHABLE;
		}
		break;
	case 3:
		switch (z) {
		case 0:	// Conditional return
			switch (y) {
			case 0:
			case 1:
			case 2:
			case 3:
				returnConditionalFlag(F(), y);
				break;
			case 4:	// GB: LD (FF00 + n),A
				IntelProcessor::memoryWrite(IoRegisters::BASE + fetchByte(), A());
				break;
			case 5: { // GB: ADD SP,dd
					const auto before = SP().word;
					const int8_t value = fetchByte();
					tickMachine(2);
					const auto result = before + value;
					SP() = result;
					const auto carried = before ^ value ^ (result & Mask16);
					F() = clearBit(F(), ZF | NF);
					F() = setBit(F(), CF, carried & Bit8);
					F() = setBit(F(), HC, carried & Bit4);
				}
				break;
			case 6:	// GB: LD A,(FF00 + n)
				A() = IntelProcessor::memoryRead(IoRegisters::BASE + fetchByte());
				break;
			case 7: { // GB: LD HL,SP + dd
					const auto before = SP().word;
					const int8_t value = fetchByte();
					tickMachine();
					const auto result = before + value;
					HL() = result;
					const auto carried = before ^ value ^ (result & Mask16);
					F() = clearBit(F(), ZF | NF);
					F() = setBit(F(), CF, carried & Bit8);
					F() = setBit(F(), HC, carried & Bit4);
				}
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 1:	// POP & various ops
			switch (q) {
			case 0:	// POP rp2[p]
				RP2(p) = popWord();
				break;
			case 1:
				switch (p) {
				case 0:	// RET
					ret();
					break;
				case 1:	// GB: RETI
					reti();
					break;
				case 2:	// JP HL
					jump(HL());
					break;
				case 3:	// LD SP,HL
					SP() = HL();
					tickMachine();
					break;
				default:
					UNREACHABLE;
				}
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 2:	// Conditional jump
			switch (y) {
			case 0:
			case 1:
			case 2:
			case 3:
				jumpConditionalFlag(F(), y);
				break;
			case 4:	// GB: LD (FF00 + C),A
				IntelProcessor::memoryWrite(IoRegisters::BASE + C(), A());
				break;
			case 5:	// GB: LD (nn),A
				MEMPTR() = BUS().ADDRESS() = fetchWord();
					IntelProcessor::memoryWrite(A());
				break;
			case 6:	// GB: LD A,(FF00 + C)
				A() = IntelProcessor::memoryRead(IoRegisters::BASE + C());
				break;
			case 7:	// GB: LD A,(nn)
				MEMPTR() = BUS().ADDRESS() = fetchWord();
				A() = memoryRead();
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0:	// JP nn
				jump(MEMPTR() = fetchWord());
				tickMachine();
				break;
			case 1:	// CB prefix
				m_prefixCB = true;
				Processor::execute(fetchByte());
				break;
			case 6:	// DI
				di();
				break;
			case 7:	// EI
				ei();
				break;
			}
			break;
		case 4:	// Conditional call: CALL cc[y], nn
			callConditionalFlag(F(), y);
			break;
		case 5:	// PUSH & various ops
			switch (q) {
			case 0:	// PUSH rp2[p]
				pushWord(RP2(p));
				break;
			case 1:
				switch (p) {
				case 0:	// CALL nn
					call(MEMPTR() = fetchWord());
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
				A() = add(F(), A(), fetchByte());
				break;
			case 1:	// ADC A,n
				A() = adc(F(), A(), fetchByte());
				break;
			case 2:	// SUB n
				A() = subtract(F(), A(), fetchByte());
				break;
			case 3:	// SBC A,n
				A() = sbc(F(), A(), fetchByte());
				break;
			case 4:	// AND n
				A() = andr(F(), A(), fetchByte());
				break;
			case 5:	// XOR n
				A() = xorr(F(), A(), fetchByte());
				break;
			case 6:	// OR n
				A() = orr(F(), A(), fetchByte());
				break;
			case 7:	// CP n
				compare(F(), A(), fetchByte());
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 7:	// Restart: RST y * 8
			restart(y << 3);
			break;
		default:
			UNREACHABLE;
		}
		break;
	}
}

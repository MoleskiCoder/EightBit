#include "stdafx.h"
#include "LR35902.h"
#include "GameBoyBus.h"

// based on http://www.z80.info/decoding.htm

EightBit::GameBoy::LR35902::LR35902(Bus& memory)
: IntelProcessor(memory),
  m_bus(memory) {
}

EightBit::register16_t& EightBit::GameBoy::LR35902::AF() {
	af.low = higherNibble(af.low);
	return af;
}

EightBit::register16_t& EightBit::GameBoy::LR35902::BC() {
	return bc;
}

EightBit::register16_t& EightBit::GameBoy::LR35902::DE() {
	return de;
}

EightBit::register16_t& EightBit::GameBoy::LR35902::HL() {
	return hl;
}

void EightBit::GameBoy::LR35902::handleRESET() {
	IntelProcessor::handleRESET();
	di();
	SP() = Mask16 - 1;
	tick(4);
}

void EightBit::GameBoy::LR35902::handleINT() {
	IntelProcessor::handleINT();
	raiseHALT();
	di();
	restart(BUS().DATA());
	tick(4);
}

void EightBit::GameBoy::LR35902::di() {
	IME() = false;
}

void EightBit::GameBoy::LR35902::ei() {
	IME() = true;
}

void EightBit::GameBoy::LR35902::increment(uint8_t& operand) {
	clearFlag(F(), NF);
	adjustZero<LR35902>(F(), ++operand);
	clearFlag(F(), HC, lowNibble(operand));
}

void EightBit::GameBoy::LR35902::decrement(uint8_t& operand) {
	setFlag(F(), NF);
	clearFlag(F(), HC, lowNibble(operand));
	adjustZero<LR35902>(F(), --operand);
}

bool EightBit::GameBoy::LR35902::jrConditionalFlag(const int flag) {
	switch (flag) {
	case 0:	// NZ
		return jrConditional(!(F() & ZF));
	case 1:	// Z
		return jrConditional(F() & ZF);
	case 2:	// NC
		return jrConditional(!(F() & CF));
	case 3:	// C
		return jrConditional(F() & CF);
	default:
		UNREACHABLE;
	}
}

bool EightBit::GameBoy::LR35902::jumpConditionalFlag(const int flag) {
	switch (flag) {
	case 0:	// NZ
		return jumpConditional(!(F() & ZF));
	case 1:	// Z
		return jumpConditional(F() & ZF);
	case 2:	// NC
		return jumpConditional(!(F() & CF));
	case 3:	// C
		return jumpConditional(F() & CF);
	default:
		UNREACHABLE;
	}
}

void EightBit::GameBoy::LR35902::reti() {
	ret();
	ei();
}

bool EightBit::GameBoy::LR35902::returnConditionalFlag(const int flag) {
	switch (flag) {
	case 0:	// NZ
		return returnConditional(!(F() & ZF));
	case 1:	// Z
		return returnConditional(F() & ZF);
	case 2:	// NC
		return returnConditional(!(F() & CF));
	case 3:	// C
		return returnConditional(F() & CF);
	default:
		UNREACHABLE;
	}
}

bool EightBit::GameBoy::LR35902::callConditionalFlag(const int flag) {
	switch (flag) {
	case 0:	// NZ
		return callConditional(!(F() & ZF));
	case 1:	// Z
		return callConditional(F() & ZF);
	case 2:	// NC
		return callConditional(!(F() & CF));
	case 3:	// C
		return callConditional(F() & CF);
	default:
		UNREACHABLE;
	}
}

void EightBit::GameBoy::LR35902::add(register16_t& operand, const register16_t value) {

	MEMPTR() = operand;

	const auto result = MEMPTR().word + value.word;

	operand.word = result;

	clearFlag(F(), NF);
	setFlag(F(), CF, result & Bit16);
	adjustHalfCarryAdd(MEMPTR().high, value.high, operand.high);
}

void EightBit::GameBoy::LR35902::add(uint8_t& operand, const uint8_t value, const int carry) {

	const register16_t result = operand + value + carry;

	adjustHalfCarryAdd(operand, value, result.low);

	operand = result.low;

	clearFlag(F(), NF);
	setFlag(F(), CF, result.word & Bit8);
	adjustZero<LR35902>(F(), operand);
}

void EightBit::GameBoy::LR35902::adc(uint8_t& operand, const uint8_t value) {
	add(operand, value, (F() & CF) >> 4);
}

void EightBit::GameBoy::LR35902::subtract(uint8_t& operand, const uint8_t value, const int carry) {

	const register16_t result = operand - value - carry;

	adjustHalfCarrySub(operand, value, result.low);

	operand = result.low;

	setFlag(F(), NF);
	setFlag(F(), CF, result.word & Bit8);
	adjustZero<LR35902>(F(), operand);
}

void EightBit::GameBoy::LR35902::sbc(const uint8_t value) {
	subtract(A(), value, (F() & CF) >> 4);
}

void EightBit::GameBoy::LR35902::andr(uint8_t& operand, const uint8_t value) {
	setFlag(F(), HC);
	clearFlag(F(), CF | NF);
	adjustZero<LR35902>(F(), operand &= value);
}

void EightBit::GameBoy::LR35902::xorr(const uint8_t value) {
	clearFlag(F(), HC | CF | NF);
	adjustZero<LR35902>(F(), A() ^= value);
}

void EightBit::GameBoy::LR35902::orr(const uint8_t value) {
	clearFlag(F(), HC | CF | NF);
	adjustZero<LR35902>(F(), A() |= value);
}

void EightBit::GameBoy::LR35902::compare(uint8_t check, const uint8_t value) {
	subtract(check, value);
}

uint8_t EightBit::GameBoy::LR35902::rlc(const uint8_t operand) {
	clearFlag(F(), NF | HC | ZF);
	const auto carry = operand & Bit7;
	setFlag(F(), CF, carry);
	return (operand << 1) | (carry >> 7);
}

uint8_t EightBit::GameBoy::LR35902::rrc(const uint8_t operand) {
	clearFlag(F(), NF | HC | ZF);
	const auto carry = operand & Bit0;
	setFlag(F(), CF, carry);
	return (operand >> 1) | (carry << 7);
}

uint8_t EightBit::GameBoy::LR35902::rl(const uint8_t operand) {
	clearFlag(F(), NF | HC | ZF);
	const auto carry = F() & CF;
	setFlag(F(), CF, operand & Bit7);
	return (operand << 1) | (carry >> 4);	// CF at Bit4
}

uint8_t EightBit::GameBoy::LR35902::rr(const uint8_t operand) {
	clearFlag(F(), NF | HC | ZF);
	const auto carry = F() & CF;
	setFlag(F(), CF, operand & Bit0);
	return (operand >> 1) | (carry << 3);	// CF at Bit4
}

uint8_t EightBit::GameBoy::LR35902::sla(const uint8_t operand) {
	clearFlag(F(), NF | HC | ZF);
	setFlag(F(), CF, operand & Bit7);
	return operand << 1;
}

uint8_t EightBit::GameBoy::LR35902::sra(const uint8_t operand) {
	clearFlag(F(), NF | HC | ZF);
	setFlag(F(), CF, operand & Bit0);
	return (operand >> 1) | (operand & Bit7);
}

uint8_t EightBit::GameBoy::LR35902::swap(const uint8_t operand) {
	clearFlag(F(), NF | HC | CF);
	return promoteNibble(operand) | demoteNibble(operand);
}

uint8_t EightBit::GameBoy::LR35902::srl(const uint8_t operand) {
	clearFlag(F(), NF | HC | ZF);
	setFlag(F(), CF, operand & Bit0);
	return (operand >> 1) & ~Bit7;
}

uint8_t EightBit::GameBoy::LR35902::bit(const int n, const uint8_t operand) {
	const auto carry = F() & CF;
	uint8_t discarded = operand;
	andr(discarded, 1 << n);
	setFlag(F(), CF, carry);
	return operand;
}

uint8_t EightBit::GameBoy::LR35902::res(const int n, const uint8_t operand) {
	return operand & ~(1 << n);
}

uint8_t EightBit::GameBoy::LR35902::set(const int n, const uint8_t operand) {
	return operand | (1 << n);
}

void EightBit::GameBoy::LR35902::daa() {

	int updated = A();

	if (F() & NF) {
		if (F() & HC)
			updated = (updated - 6) & Mask8;
		if (F() & CF)
			updated -= 0x60;
	} else {
		if ((F() & HC) || lowNibble(updated) > 9)
			updated += 6;
		if ((F() & CF) || updated > 0x9F)
			updated += 0x60;
	}

	clearFlag(F(), HC | ZF);
	setFlag(F(), CF, (F() & CF) || (updated & Bit8));
	A() = updated & Mask8;

	adjustZero<LR35902>(F(), A());
}

void EightBit::GameBoy::LR35902::cpl() {
	setFlag(F(), HC | NF);
	A() = ~A();
}

void EightBit::GameBoy::LR35902::scf() {
	setFlag(F(), CF);
	clearFlag(F(), HC | NF);
}

void EightBit::GameBoy::LR35902::ccf() {
	clearFlag(F(), NF | HC);
	clearFlag(F(), CF, F() & CF);
}

int EightBit::GameBoy::LR35902::step() {

	ExecutingInstruction.fire(*this);
	m_prefixCB = false;
	resetCycles();
	if (LIKELY(powered())) {

		const auto interruptEnable = BUS().peek(IoRegisters::BASE + IoRegisters::IE);
		const auto interruptFlags = m_bus.IO().peek(IoRegisters::IF);

		const auto masked = interruptEnable & interruptFlags;
		if (masked) {
			if (IME()) {
				m_bus.IO().poke(IoRegisters::IF, 0);
				lowerINT();
				const int index = EightBit::findFirstSet(masked);
				BUS().DATA() = 0x38 + (index << 3);
			} else {
				if (halted())
					proceed();
			}
		}

		if (UNLIKELY(lowered(RESET()))) {
			handleRESET();
		} else if (UNLIKELY(lowered(INT()))) {
			handleINT();
		} else if (UNLIKELY(lowered(HALT()))) {
			Processor::execute(0);	// NOP
		} else {
			Processor::execute(fetchByte());
		}

		m_bus.IO().checkTimers(clockCycles());
		m_bus.IO().transferDma();

	}
	ExecutedInstruction.fire(*this);
	return clockCycles();
}

int EightBit::GameBoy::LR35902::execute() {

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

	if (UNLIKELY(cycles() == 0))
		throw std::logic_error("Unhandled opcode");

	return clockCycles();
}

void EightBit::GameBoy::LR35902::executeCB(const int x, const int y, const int z, int, int) {
	switch (x) {
	case 0:	{ // rot[y] r[z]
		auto operand = R(z);
		switch (y) {
		case 0:
			operand = rlc(operand);
			break;
		case 1:
			operand = rrc(operand);
			break;
		case 2:
			operand = rl(operand);
			break;
		case 3:
			operand = rr(operand);
			break;
		case 4:
			operand = sla(operand);
			break;
		case 5:
			operand = sra(operand);
			break;
		case 6:	// GB: SWAP r
			operand = swap(operand);
			break;
		case 7:
			operand = srl(operand);
			break;
		default:
			UNREACHABLE;
		}
		tick(2);
		R(z, operand);
		adjustZero<LR35902>(F(), operand);
		if (UNLIKELY(z == 6))
			tick(2);
		break;
	} case 1:	// BIT y, r[z]
		bit(y, R(z));
		tick(2);
		if (UNLIKELY(z == 6))
			tick(2);
		break;
	case 2:	// RES y, r[z]
		R(z, res(y, R(z)));
		tick(2);
		if (UNLIKELY(z == 6))
			tick(2);
		break;
	case 3:	// SET y, r[z]
		R(z, set(y, R(z)));
		tick(2);
		if (UNLIKELY(z == 6))
			tick(2);
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
				tick();
				break;
			case 1:	// GB: LD (nn),SP
				BUS().ADDRESS() = fetchWord();
				setWord(SP());
				tick(5);
				break;
			case 2:	// GB: STOP
				stop();
				tick();
				break;
			case 3:	// JR d
				jr(fetchByte());
				tick(4);
				break;
			case 4: // JR cc,d
			case 5:
			case 6:
			case 7:
				if (jrConditionalFlag(y - 4))
					tick();
				tick(2);
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0: // LD rp,nn
				RP(p) = fetchWord();
				tick(3);
				break;
			case 1:	// ADD HL,rp
				add(HL(), RP(p));
				tick(2);
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
					BUS().write(BC(), A());
					tick(2);
					break;
				case 1:	// LD (DE),A
					BUS().write(DE(), A());
					tick(2);
					break;
				case 2:	// GB: LDI (HL),A
					BUS().write(HL()++, A());
					tick(2);
					break;
				case 3: // GB: LDD (HL),A
					BUS().write(HL()--, A());
					tick(2);
					break;
				default:
					UNREACHABLE;
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					A() = BUS().read(BC());
					tick(2);
					break;
				case 1:	// LD A,(DE)
					A() = BUS().read(DE());
					tick(2);
					break;
				case 2:	// GB: LDI A,(HL)
					A() = BUS().read(HL()++);
					tick(2);
					break;
				case 3:	// GB: LDD A,(HL)
					A() = BUS().read(HL()--);
					tick(2);
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
			tick(2);
			break;
		case 4: { // 8-bit INC
			auto operand = R(y);
			increment(operand);
			R(y, operand);
			tick();
			if (UNLIKELY(y == 6))
				tick(2);
			break;
		} case 5: {	// 8-bit DEC
			auto operand = R(y);
			decrement(operand);
			R(y, operand);
			tick();
			if (UNLIKELY(y == 6))
				tick(2);
			break;
		} case 6:	// 8-bit load immediate
			R(y, fetchByte());	// LD r,n
			tick(2);
			break;
		case 7:	// Assorted operations on accumulator/flags
			switch (y) {
			case 0:
				A() = rlc(A());
				break;
			case 1:
				A() = rrc(A());
				break;
			case 2:
				A() = rl(A());
				break;
			case 3:
				A() = rr(A());
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
			default:
				UNREACHABLE;
			}
			tick();
			break;
		default:
			UNREACHABLE;
		}
		break;
	case 1:	// 8-bit loading
		if (UNLIKELY(z == 6 && y == 6)) { 	// Exception (replaces LD (HL), (HL))
			halt();
		} else {
			R(y, R(z));
			if (UNLIKELY((y == 6) || (z == 6))) // M operations
				tick();
		}
		tick();
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
			subtract(A(), R(z));
			break;
		case 3:	// SBC A,r
			sbc(R(z));
			break;
		case 4:	// AND r
			andr(A(), R(z));
			break;
		case 5:	// XOR r
			xorr(R(z));
			break;
		case 6:	// OR r
			orr(R(z));
			break;
		case 7:	// CP r
			compare(A(), R(z));
			break;
		default:
			UNREACHABLE;
		}
		tick();
		if (UNLIKELY(z == 6))
			tick();
		break;
	case 3:
		switch (z) {
		case 0:	// Conditional return
			switch (y) {
			case 0:
			case 1:
			case 2:
			case 3:
				if (returnConditionalFlag(y))
					tick(3);
				tick(2);
				break;
			case 4:	// GB: LD (FF00 + n),A
				BUS().write(IoRegisters::BASE + fetchByte(), A());
				tick(3);
				break;
			case 5: { // GB: ADD SP,dd
					const auto before = SP().word;
					const int8_t value = fetchByte();
					const auto result = before + value;
					SP() = result;
					const auto carried = before ^ value ^ (result & Mask16);
					clearFlag(F(), ZF | NF);
					setFlag(F(), CF, carried & Bit8);
					setFlag(F(), HC, carried & Bit4);
				}
				tick(4);
				break;
			case 6:	// GB: LD A,(FF00 + n)
				A() = BUS().read(IoRegisters::BASE + fetchByte());
				tick(3);
				break;
			case 7: { // GB: LD HL,SP + dd
					const auto before = SP().word;
					const int8_t value = fetchByte();
					const auto result = before + value;
					HL() = result;
					const auto carried = before ^ value ^ (result & Mask16);
					clearFlag(F(), ZF | NF);
					setFlag(F(), CF, carried & Bit8);
					setFlag(F(), HC, carried & Bit4);
				}
				tick(3);
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 1:	// POP & various ops
			switch (q) {
			case 0:	// POP rp2[p]
				RP2(p) = popWord();
				tick(3);
				break;
			case 1:
				switch (p) {
				case 0:	// RET
					ret();
					tick(4);
					break;
				case 1:	// GB: RETI
					reti();
					tick(4);
					break;
				case 2:	// JP HL
					jump(HL());
					tick();
					break;
				case 3:	// LD SP,HL
					SP() = HL();
					tick(2);
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
				jumpConditionalFlag(y);
				tick(3);
				break;
			case 4:	// GB: LD (FF00 + C),A
				BUS().write(IoRegisters::BASE + C(), A());
				tick(2);
				break;
			case 5:	// GB: LD (nn),A
				BUS().ADDRESS() = MEMPTR() = fetchWord();
				BUS().write(A());
				tick(4);
				break;
			case 6:	// GB: LD A,(FF00 + C)
				A() = BUS().read(IoRegisters::BASE + C());
				tick(2);
				break;
			case 7:	// GB: LD A,(nn)
				BUS().ADDRESS() = MEMPTR() = fetchWord();
				A() = BUS().read();
				tick(4);
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0:	// JP nn
				jump(MEMPTR() = fetchWord());
				tick(4);
				break;
			case 1:	// CB prefix
				m_prefixCB = true;
				Processor::execute(fetchByte());
				break;
			case 6:	// DI
				di();
				tick();
				break;
			case 7:	// EI
				ei();
				tick();
				break;
			}
			break;
		case 4:	// Conditional call: CALL cc[y], nn
			if (callConditionalFlag(y))
				tick(3);
			tick(3);
			break;
		case 5:	// PUSH & various ops
			switch (q) {
			case 0:	// PUSH rp2[p]
				pushWord(RP2(p));
				tick(4);
				break;
			case 1:
				switch (p) {
				case 0:	// CALL nn
					call(MEMPTR() = fetchWord());
					tick(6);
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
				add(A(), fetchByte());
				break;
			case 1:	// ADC A,n
				adc(A(), fetchByte());
				break;
			case 2:	// SUB n
				subtract(A(), fetchByte());
				break;
			case 3:	// SBC A,n
				sbc(fetchByte());
				break;
			case 4:	// AND n
				andr(A(), fetchByte());
				break;
			case 5:	// XOR n
				xorr(fetchByte());
				break;
			case 6:	// OR n
				orr(fetchByte());
				break;
			case 7:	// CP n
				compare(A(), fetchByte());
				break;
			default:
				UNREACHABLE;
			}
			tick(2);
			break;
		case 7:	// Restart: RST y * 8
			restart(y << 3);
			tick(4);
			break;
		default:
			UNREACHABLE;
		}
		break;
	}
}

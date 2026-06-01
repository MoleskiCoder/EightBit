#include "stdafx.h"
#include "../inc/Intel8080.h"

EightBit::Intel8080::Intel8080(Bus& bus)
: IntelProcessor(bus) {

	RaisedPOWER.connect([this](EventArgs) {

		releaseMemory();
		releaseIO();

		releaseRead();
		releaseWrite();

		disableInterrupts();
	});
}

DEFINE_PIN_LEVEL_CHANGERS(DBIN, Intel8080);
DEFINE_PIN_LEVEL_CHANGERS(WR, Intel8080);

EightBit::register16_t& EightBit::Intel8080::AF() noexcept {
	m_af.low = (m_af.low | Bit1) & ~(Bit5 | Bit3);
	return m_af;
}

EightBit::register16_t& EightBit::Intel8080::BC() noexcept {
	return m_bc;
}

EightBit::register16_t& EightBit::Intel8080::DE() noexcept {
	return m_de;
}

EightBit::register16_t& EightBit::Intel8080::HL() noexcept {
	return m_hl;
}

void EightBit::Intel8080::memoryWrite() noexcept {
	memoryUpdate();
}

void EightBit::Intel8080::memoryRead() noexcept {
	ReadingMemory.fire();
		requestMemory();
			IntelProcessor::memoryRead();
		releaseMemory();
	ReadMemory.fire();
}

void EightBit::Intel8080::memoryUpdate(int ticks) noexcept {
	WritingMemory.fire();
		tick(ticks);
		requestMemory();
			IntelProcessor::memoryWrite();
		releaseMemory();
	WrittenMemory.fire();
}

void EightBit::Intel8080::handleRESET() noexcept {
	IntelProcessor::handleRESET();
	disableInterrupts();
	SP() = AF() = Mask16;
}

void EightBit::Intel8080::handleINT() noexcept {
	IntelProcessor::handleINT();
	Processor::execute(BUS().DATA());
}

void EightBit::Intel8080::disableInterrupts() noexcept {
	m_interruptEnable = false;
}

void EightBit::Intel8080::enableInterrupts() noexcept {
	m_interruptEnable = true;
}

uint8_t& EightBit::Intel8080::R(const int r, MemoryMapping::AccessLevel access) noexcept {
	switch (r) {
	case 0b000:
		return B();
	case 0b001:
		return C();
	case 0b010:
		return D();
	case 0b011:
		return E();
	case 0b100:
		return H();
	case 0b101:
		return L();
	case 0b110:
		BUS().ADDRESS() = HL();
		switch (access) {
		case MemoryMapping::AccessLevel::ReadOnly:
			memoryRead();
			break;
		case MemoryMapping::AccessLevel::WriteOnly:
			break;
		default:
			UNREACHABLE;
			break;
		}
		return BUS().DATA();
	case 0b111:
		return A();
	default:
		UNREACHABLE;
	}
}

EightBit::register16_t& EightBit::Intel8080::RP(const int rp) {
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

EightBit::register16_t& EightBit::Intel8080::RP2(const int rp) {
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

uint8_t EightBit::Intel8080::increment(uint8_t operand) {
	const uint8_t result = operand + 1;
	adjustSZP(result);
	clearBit(AC, lowNibble(result));
	return result;
}

uint8_t EightBit::Intel8080::decrement(uint8_t operand) {
	const uint8_t result = operand - 1;
	adjustSZP(result);
	setBit(AC, lowNibble(result) != Mask4);
	return result;
}

bool EightBit::Intel8080::convertCondition(int flag) noexcept {
	switch (flag) {
	case 0:
		return zero() == 0;
	case 1:
		return zero() != 0;
	case 2:
		return carry() == 0;
	case 3:
		return carry() != 0;
	case 4:
		return parity() == 0;
	case 5:
		return parity() != 0;
	case 6:
		return sign() == 0;
	case 7:
		return sign() != 0;
	default:
		UNREACHABLE;
	}
}

void EightBit::Intel8080::add(const register16_t value) {
	const auto result = HL().joined + value.joined;
	HL() = result;
	F() = setBit(F(), CF, result & Bit16);
}

void EightBit::Intel8080::add(const uint8_t value, const int carry) {

	const register16_t result = A() + value + carry;

	adjustAuxiliaryCarryAdd(A(), value, result.joined);

	A() = result.low;

	setBit(CF, result.joined & Bit8);
	adjustSZP(A());
}

void EightBit::Intel8080::adc(const uint8_t value) {
	add(value, carry());
}

uint8_t EightBit::Intel8080::subtract(const uint8_t value, const int carry) {

	const auto operand = A();

	const register16_t subtraction = operand - value - carry;
	const auto result = subtraction.low;

	adjustAuxiliaryCarrySub(operand, value, subtraction.joined);

	setBit(CF, subtraction.joined & Bit8);
	adjustSZP(result);

	return result;
}

void EightBit::Intel8080::sub(uint8_t value, int carry) noexcept {
	A() = subtract(value, carry);
}

void EightBit::Intel8080::sbb(const uint8_t value) {
	sub(value, carry());
}

void EightBit::Intel8080::andr(const uint8_t value) {
	setBit(AC, (A() | value) & Bit3);
	clearBit(CF);
	adjustSZP(A() &= value);
}

void EightBit::Intel8080::xorr(const uint8_t value) {
	clearBit(AC | CF);
	adjustSZP(A() ^= value);
}

void EightBit::Intel8080::orr(const uint8_t value) {
	clearBit(AC | CF);
	adjustSZP(A() |= value);
}

void EightBit::Intel8080::compare(const uint8_t value) {
	subtract(value);
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
	const auto carrying = carry();
	F() = setBit(F(), CF, A() & Bit7);
	A() = (A() << 1) | carrying;
}

void EightBit::Intel8080::rr() {
	const auto carrying = carry();
	F() = setBit(F(), CF, A() & Bit0);
	A() = (A() >> 1) | (carrying << 7);
}

void EightBit::Intel8080::daa() {
	const auto& before = A();
	auto carry = F() & CF;
	uint8_t addition = 0;
	if (auxiliaryCarry() || lowNibble(before) > 9) {
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
	BUS().ADDRESS() = SP();
	getInto(MEMPTR());
	BUS().DATA() = exchange.high;
	exchange.high = MEMPTR().high;
	memoryUpdate(2);
	--BUS().ADDRESS();
	BUS().DATA() = exchange.low;
	exchange.low = MEMPTR().low;
	memoryUpdate();
	tick(15);	// 2(memUpdate) + 1(memUpdate) + 15 = 18 total
}

void EightBit::Intel8080::writePort(const uint8_t port) noexcept {
	BUS().ADDRESS() = { port, port };
	BUS().DATA() = A();
	writePort();
}

void EightBit::Intel8080::writePort() noexcept {
	requestIO();
	busWrite();
	releaseIO();;
}

void EightBit::Intel8080::readPort(const uint8_t port) noexcept {
	BUS().ADDRESS() = { port, port };
	readPort();
}

void EightBit::Intel8080::readPort() noexcept {
	requestIO();
	busRead();
	releaseIO();
}

void EightBit::Intel8080::poweredStep() noexcept {
	if (UNLIKELY(lowered(RESET()))) {
		handleRESET();
	} else if (UNLIKELY(lowered(INT()) && m_interruptEnable)) {
		handleINT();
	} else if (UNLIKELY(lowered(HALT()))) {
		IntelProcessor::execute(0);	// NOP
	} else {
		IntelProcessor::execute(fetchInstruction());
	}
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
				fetchInto(RP(p));
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
					fetchShortAddress();
					setShort(HL());
					tick(16);
					break;
				case 3: // LD (nn),A
					fetchInto(MEMPTR());
					BUS().ADDRESS() = MEMPTR();
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
					readMemoryIndirect(BC());
					A() = BUS().DATA();
					tick(7);
					break;
				case 1:	// LD A,(DE)
					readMemoryIndirect(DE());
					A() = BUS().DATA();
					tick(7);
					break;
				case 2:	// LD HL,(nn)
					fetchShortAddress();
					getShort();
					HL() = intermediate();
					tick(16);
					break;
				case 3:	// LD A,(nn)
					fetchInto(MEMPTR());
					readMemoryIndirect();
					A() = BUS().DATA();
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
			IntelProcessor::R(y, increment(operand));
			tick(5);
			if (UNLIKELY(y == 6))	// INC M: memRead(0) + memUpdate(1) + 5 + 4 = 10
				tick(4);
			break;
		} case 5: {	// 8-bit DEC
			auto operand = R(y);
			IntelProcessor::R(y, decrement(operand));
			tick(5);
			if (UNLIKELY(y == 6))	// DEC M: memRead(0) + memUpdate(1) + 5 + 4 = 10
				tick(4);
			break;
		} case 6:	// 8-bit load immediate
			fetchByte();
			IntelProcessor::R(y, BUS().DATA());
			tick(7);
			if (UNLIKELY(y == 6))	// LD M,n: memUpdate(1) + 7 + 2 = 10
				tick(2);
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
			tick(4);
		} else {
			IntelProcessor::R(y, R(z));
			tick(5);
			if (UNLIKELY(y == 6))		// MOV M,r: memUpdate(1) + 5 + 1 = 7
				tick(1);
			else if (UNLIKELY(z == 6))	// MOV r,M: memRead(0) + 5 + 2 = 7
				tick(2);
		}
		break;
	case 2: { // Operate on accumulator and register/memory location
		const auto value = R(z);
		switch (y) {
		case 0:	// ADD A,r
			add(value);
			break;
		case 1:	// ADC A,r
			adc(value);
			break;
		case 2:	// SUB r
			sub(value);
			break;
		case 3:	// SBC A,r
			sbb(value);
			break;
		case 4:	// AND r
			andr(value);
			break;
		case 5:	// XOR r
			xorr(value);
			break;
		case 6:	// OR r
			orr(value);
			break;
		case 7:	// CP r
			compare(value);
			break;
		default:
			UNREACHABLE;
		}
		tick(4);
		if (UNLIKELY(z == 6))	// M operations take 3 extra cycles
			tick(3);
		break;
	}
	case 3:
		switch (z) {
		case 0: {	// Conditional return
			const bool condition = convertCondition(y);
			if (condition) {
				ret();
				tick(11);
			} else {
				tick(5);
			}
			break;
		}
		case 1:	// POP & various ops
			switch (q) {
			case 0:	// POP rp2[p]
				popInto(RP2(p));
				tick(10);
				break;
			case 1:
				switch (p) {
				case 0:	// RET
					ret();
					tick(10);
					break;
				case 2:	// JP HL
					Processor::jump(HL());
					tick(5);
					break;
				case 3:	// LD SP,HL
					SP() = HL();
					tick(5);
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
				fetchInto(MEMPTR());
				jump();
				tick(10);
				break;
			case 2:	// OUT (n),A
				fetchByte();
				writePort(BUS().DATA());
				tick(10);
				break;
			case 3:	// IN A,(n)
				fetchByte();
				readPort(BUS().DATA());
				A() = BUS().DATA();
				tick(10);
				break;
			case 4:	// EX (SP),HL
				xhtl(HL());
				break;
			case 5:	// EX DE,HL
				std::swap(DE(), HL());
				tick(5);
				break;
			case 6:	// DI
				disableInterrupts();
				tick(4);
				break;
			case 7:	// EI
				enableInterrupts();
				tick(4);
				break;
			}
			break;
		case 4: {	// Conditional call: CALL cc[y], nn
			const bool condition = convertCondition(y);
			fetchInto(MEMPTR());
			if (condition) {
				call();
				tick(17);
			} else {
				tick(11);
			}
			break;
		}
		case 5:	// PUSH & various ops
			switch (q) {
			case 0:	// PUSH rp2[p]
				pushShort(RP2(p));
				tick(11);
				break;
			case 1:
				switch (p) {
				case 0:	// CALL nn
					callIndirect();
					tick(17);
					break;
				}
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 6: { // Operate on accumulator and immediate operand: alu[y] n
			fetchByte();
			auto operand = BUS().DATA();
			switch (y) {
			case 0:	// ADD A,n
				add(operand);
				break;
			case 1:	// ADC A,n
				adc(operand);
				break;
			case 2:	// SUB n
				sub(operand);
				break;
			case 3:	// SBC A,n
				sbb(operand);
				break;
			case 4:	// AND n
				andr(operand);
				break;
			case 5:	// XOR n
				xorr(operand);
				break;
			case 6:	// OR n
				orr(operand);
				break;
			case 7:	// CP n
				compare(operand);
				break;
			default:
				UNREACHABLE;
			}
			tick(7);
			break;
		}
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

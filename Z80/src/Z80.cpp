#include "stdafx.h"
#include "Z80.h"

// based on http://www.z80.info/decoding.htm

EightBit::Z80::Z80(Bus& bus, InputOutput& ports)
: IntelProcessor(bus),
  m_ports(ports) {
}

EightBit::register16_t& EightBit::Z80::AF() {
	return m_accumulatorFlags[m_accumulatorFlagsSet];
}

EightBit::register16_t& EightBit::Z80::BC() {
	return m_registers[m_registerSet][BC_IDX];
}

EightBit::register16_t& EightBit::Z80::DE() {
	return m_registers[m_registerSet][DE_IDX];
}

EightBit::register16_t& EightBit::Z80::HL() {
	return m_registers[m_registerSet][HL_IDX];
}

void EightBit::Z80::powerOn() {

	IntelProcessor::powerOn();

	raise(NMI());
	raise(M1());

	di();
	IM() = 0;

	REFRESH() = 0;
	IV() = Mask8;

	exxAF();
	AF() = Mask16;

	exx();
	IX() = IY() = BC() = DE() = HL() = Mask16;

	m_prefixCB = m_prefixDD = m_prefixED = m_prefixFD = false;
}

void EightBit::Z80::handleRESET() {
	IntelProcessor::handleRESET();
	di();
	addCycles(3);
}

void EightBit::Z80::handleNMI() {
	raise(NMI());
	raise(HALT());
	IFF1() = false;
	restart(0x66);
	addCycles(13);
}

void EightBit::Z80::handleINT() {
	IntelProcessor::handleINT();
	raise(HALT());
	if (IFF1()) {
		di();
		switch (IM()) {
		case 0:		// i8080 equivalent
			execute(BUS().DATA());
			break;
		case 1:
			restart(7 << 3);
			addCycles(13);
			break;
		case 2:
			call(MEMPTR() = { BUS().DATA(), IV() });
			addCycles(19);
			break;
		default:
			UNREACHABLE;
		}
	}
}

void EightBit::Z80::di() {
	IFF1() = IFF2() = false;
}

void EightBit::Z80::ei() {
	IFF1() = IFF2() = true;
}

void EightBit::Z80::increment(uint8_t& operand) {
	clearFlag(F(), NF);
	adjustSZXY<Z80>(F(), ++operand);
	setFlag(F(), VF, operand == Bit7);
	clearFlag(F(), HC, lowNibble(operand));
}

void EightBit::Z80::decrement(uint8_t& operand) {
	setFlag(F(), NF);
	clearFlag(F(), HC, lowNibble(operand));
	adjustSZXY<Z80>(F(), --operand);
	setFlag(F(), VF, operand == Mask7);
}

bool EightBit::Z80::jrConditionalFlag(const int flag) {
	ASSUME(flag >= 0);
	ASSUME(flag <= 3);
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

bool EightBit::Z80::jumpConditionalFlag(const int flag) {
	ASSUME(flag >= 0);
	ASSUME(flag <= 7);
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

void EightBit::Z80::retn() {
	ret();
	IFF1() = IFF2();
}

void EightBit::Z80::reti() {
	retn();
}

bool EightBit::Z80::returnConditionalFlag(const int flag) {
	ASSUME(flag >= 0);
	ASSUME(flag <= 7);
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

bool EightBit::Z80::callConditionalFlag(const int flag) {
	ASSUME(flag >= 0);
	ASSUME(flag <= 7);
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

void EightBit::Z80::sbc(const register16_t value) {

	MEMPTR() = HL2();

	const auto beforeNegative = MEMPTR().high & SF;
	const auto valueNegative = value.high & SF;

	const auto result = MEMPTR().word - value.word - (F() & CF);
	HL2() = result;

	const auto afterNegative = HL2().high & SF;

	setFlag(F(), SF, afterNegative);
	clearFlag(F(), ZF, HL2().word);
	adjustHalfCarrySub(F(), MEMPTR().high, value.high, HL2().high);
	adjustOverflowSub(F(), beforeNegative, valueNegative, afterNegative);
	setFlag(F(), NF);
	setFlag(F(), CF, result & Bit16);
	adjustXY<Z80>(F(), HL2().high);

	++MEMPTR();
}

void EightBit::Z80::adc(const register16_t value) {

	MEMPTR() = HL2();

	const auto beforeNegative = MEMPTR().high & SF;
	const auto valueNegative = value.high & SF;

	const auto result = MEMPTR().word + value.word + (F() & CF);
	HL2() = result;

	const auto afterNegative = HL2().high & SF;

	setFlag(F(), SF, afterNegative);
	clearFlag(F(), ZF, HL2().word);
	adjustHalfCarryAdd(F(), MEMPTR().high, value.high, HL2().high);
	adjustOverflowAdd(F(), beforeNegative, valueNegative, afterNegative);
	clearFlag(F(), NF);
	setFlag(F(), CF, result & Bit16);
	adjustXY<Z80>(F(), HL2().high);

	++MEMPTR();
}

void EightBit::Z80::add(const register16_t value) {

	MEMPTR() = HL2();

	const auto result = MEMPTR().word + value.word;

	HL2() = result;

	clearFlag(F(), NF);
	setFlag(F(), CF, result & Bit16);
	adjustHalfCarryAdd(F(), MEMPTR().high, value.high, HL2().high);
	adjustXY<Z80>(F(), HL2().high);

	++MEMPTR();
}

void EightBit::Z80::add(const uint8_t value, const int carry) {

	const register16_t result = A() + value + carry;

	adjustHalfCarryAdd(F(), A(), value, result.low);
	adjustOverflowAdd(F(), A(), value, result.low);

	clearFlag(F(), NF);
	setFlag(F(), CF, result.word & Bit8);
	adjustSZXY<Z80>(F(), A() = result.low);
}

void EightBit::Z80::adc(const uint8_t value) {
	add(value, F() & CF);
}

void EightBit::Z80::subtract(uint8_t& operand, const uint8_t value, const int carry) {

	const register16_t result = operand - value - carry;

	adjustHalfCarrySub(F(), operand, value, result.low);
	adjustOverflowSub(F(), operand, value, result.low);

	setFlag(F(), NF);
	setFlag(F(), CF, result.word & Bit8);
	adjustSZ<Z80>(F(), operand = result.low);
}

void EightBit::Z80::sub(const uint8_t value, const int carry) {
	subtract(A(), value, carry);
	adjustXY<Z80>(F(), A());
}

void EightBit::Z80::sbc(const uint8_t value) {
	sub(value, F() & CF);
}

void EightBit::Z80::andr(const uint8_t value) {
	setFlag(F(), HC);
	clearFlag(F(), CF | NF);
	adjustSZPXY<Z80>(F(), A() &= value);
}

void EightBit::Z80::xorr(const uint8_t value) {
	clearFlag(F(), HC | CF | NF);
	adjustSZPXY<Z80>(F(), A() ^= value);
}

void EightBit::Z80::orr(const uint8_t value) {
	clearFlag(F(), HC | CF | NF);
	adjustSZPXY<Z80>(F(), A() |= value);
}

void EightBit::Z80::compare(const uint8_t value) {
	auto original = A();
	subtract(original, value);
	adjustXY<Z80>(F(), value);
}

void EightBit::Z80::rlc(uint8_t& operand) {
	clearFlag(F(), NF | HC);
	const auto carry = operand & Bit7;
	setFlag(F(), CF, carry);
	adjustXY<Z80>(F(), operand = (operand << 1) | (carry >> 7));
}

void EightBit::Z80::rrc(uint8_t& operand) {
	clearFlag(F(), NF | HC);
	const auto carry = operand & Bit0;
	setFlag(F(), CF, carry);
	adjustXY<Z80>(F(), operand = (operand >> 1) | (carry << 7));
}

void EightBit::Z80::rl(uint8_t& operand) {
	clearFlag(F(), NF | HC);
	const auto carry = F() & CF;
	setFlag(F(), CF, operand & Bit7);
	adjustXY<Z80>(F(), operand = (operand << 1) | carry);
}

void EightBit::Z80::rr(uint8_t& operand) {
	clearFlag(F(), NF | HC);
	const auto carry = F() & CF;
	setFlag(F(), CF, operand & Bit0);
	adjustXY<Z80>(F(), operand = (operand >> 1) | (carry << 7));
}

//

void EightBit::Z80::sla(uint8_t& operand) {
	clearFlag(F(), NF | HC);
	setFlag(F(), CF, operand & Bit7);
	adjustXY<Z80>(F(), operand <<= 1);
}

void EightBit::Z80::sra(uint8_t& operand) {
	clearFlag(F(), NF | HC);
	setFlag(F(), CF, operand & Bit0);
	adjustXY<Z80>(F(), operand = (operand >> 1) | (operand & Bit7));
}

void EightBit::Z80::sll(uint8_t& operand) {
	clearFlag(F(), NF | HC);
	setFlag(F(), CF, operand & Bit7);
	adjustXY<Z80>(F(), operand = (operand << 1) | Bit0);
}

void EightBit::Z80::srl(uint8_t& operand) {
	clearFlag(F(), NF | HC);
	setFlag(F(), CF, operand & Bit0);
	operand = (operand >> 1) & ~Bit7;
	adjustXY<Z80>(F(), operand);
	setFlag(F(), ZF, operand);
}

uint8_t EightBit::Z80::bit(const int n, const uint8_t operand) {
	ASSUME(n >= 0);
	ASSUME(n <= 7);
	setFlag(F(), HC);
	clearFlag(F(), NF);
	const auto discarded = operand & (1 << n);
	adjustSZXY<Z80>(F(), discarded);
	clearFlag(F(), PF, discarded);
	return operand;
}

uint8_t EightBit::Z80::res(const int n, const uint8_t operand) {
	ASSUME(n >= 0);
	ASSUME(n <= 7);
	return operand & ~(1 << n);
}

uint8_t EightBit::Z80::set(const int n, const uint8_t operand) {
	ASSUME(n >= 0);
	ASSUME(n <= 7);
	return operand | (1 << n);
}

void EightBit::Z80::neg() {

	setFlag(F(), PF, A() == Bit7);
	setFlag(F(), CF, A());
	setFlag(F(), NF);

	const auto original = A();

	A() = (~A() + 1);	// two's complement

	adjustHalfCarrySub(F(), 0U, original, A());
	adjustOverflowSub(F(), 0U, original, A());

	adjustSZXY<Z80>(F(), A());
}

void EightBit::Z80::daa() {

	auto updated = A();

	const auto lowAdjust = (F() & HC) || (lowNibble(A()) > 9);
	const auto highAdjust = (F() & CF) || (A() > 0x99);

	if (F() & NF) {
		if (lowAdjust)
			updated -= 6;
		if (highAdjust)
			updated -= 0x60;
	} else {
		if (lowAdjust)
			updated += 6;
		if (highAdjust)
			updated += 0x60;
	}

	F() = (F() & (CF | NF)) | (A() > 0x99 ? CF : 0) | ((A() ^ updated) & HC);

	adjustSZPXY<Z80>(F(), A() = updated);
}

void EightBit::Z80::cpl() {
	setFlag(F(), HC | NF);
	adjustXY<Z80>(F(), A() = ~A());
}

void EightBit::Z80::scf() {
	setFlag(F(), CF);
	clearFlag(F(), HC | NF);
	adjustXY<Z80>(F(), A());
}

void EightBit::Z80::ccf() {
	clearFlag(F(), NF);
	const auto carry = F() & CF;
	setFlag(F(), HC, carry);
	clearFlag(F(), CF, carry);
	adjustXY<Z80>(F(), A());
}

void EightBit::Z80::xhtl() {
	MEMPTR().low = BUS().read(SP());
	BUS().write(HL2().low);
	HL2().low = MEMPTR().low;
	++BUS().ADDRESS();
	MEMPTR().high = BUS().read();
	BUS().write(HL2().high);
	HL2().high = MEMPTR().high;
}

void EightBit::Z80::blockCompare(const register16_t source, register16_t& counter) {

	const auto value = BUS().read(source);
	uint8_t result = A() - value;

	setFlag(F(), PF, --counter.word);

	adjustSZ<Z80>(F(), result);
	adjustHalfCarrySub(F(), A(), value, result);
	setFlag(F(), NF);

	result -= ((F() & HC) >> 4);

	setFlag(F(), YF, result & Bit1);
	setFlag(F(), XF, result & Bit3);
}

void EightBit::Z80::cpi() {
	blockCompare(HL()++, BC());
	++MEMPTR();
}

void EightBit::Z80::cpd() {
	blockCompare(HL()--, BC());
	--MEMPTR();
}

bool EightBit::Z80::cpir() {
	cpi();
	return (F() & PF) && !(F() & ZF);	// See CPI
}

bool EightBit::Z80::cpdr() {
	cpd();
	return (F() & PF) && !(F() & ZF);	// See CPD
}

void EightBit::Z80::blockLoad(const register16_t source, const register16_t destination, register16_t& counter) {
	const auto value = BUS().read(source);
	BUS().write(destination, value);
	const auto xy = A() + value;
	setFlag(F(), XF, xy & 8);
	setFlag(F(), YF, xy & 2);
	clearFlag(F(), NF | HC);
	setFlag(F(), PF, --counter.word);
}

void EightBit::Z80::ldd() {
	blockLoad(HL()--, DE()--, BC());
}

void EightBit::Z80::ldi() {
	blockLoad(HL()++, DE()++, BC());
}

bool EightBit::Z80::ldir() {
	ldi();
	return !!(F() & PF);		// See LDI
}

bool EightBit::Z80::lddr() {
	ldd();
	return !!(F() & PF);		// See LDD
}

void EightBit::Z80::blockIn(register16_t& source, const register16_t destination) {
	MEMPTR() = BUS().ADDRESS() = source;
	const auto value = readPort();
	BUS().write(destination, value);
	decrement(source.high);
	setFlag(F(), NF);
}

void EightBit::Z80::ini() {
	blockIn(BC(), HL()++);
	++MEMPTR();
}

void EightBit::Z80::ind() {
	blockIn(BC(), HL()--);
	--MEMPTR();
}

bool EightBit::Z80::inir() {
	ini();
	return !(F() & ZF);	// See INI
}

bool EightBit::Z80::indr() {
	ind();
	return !(F() & ZF);	// See IND
}

void EightBit::Z80::blockOut(const register16_t source, register16_t& destination) {
	const auto value = BUS().read(source);
	BUS().ADDRESS() = destination;
	writePort();
	decrement(destination.high);
	MEMPTR() = destination;
	setFlag(F(), NF, value & Bit7);
	setFlag(F(), HC | CF, (L() + value) > 0xff);
	adjustParity<Z80>(F(), ((value + L()) & 7) ^ B());
}

void EightBit::Z80::outi() {
	blockOut(HL()++, BC());
	++MEMPTR();
}

void EightBit::Z80::outd() {
	blockOut(HL()--, BC());
	--MEMPTR();
}

bool EightBit::Z80::otir() {
	outi();
	return !(F() & ZF);	// See OUTI
}

bool EightBit::Z80::otdr() {
	outd();
	return !(F() & ZF);	// See OUTD
}

void EightBit::Z80::rrd() {
	(MEMPTR() = BUS().ADDRESS() = HL())++;
	const auto memory = BUS().read();
	BUS().write(promoteNibble(A()) | highNibble(memory));
	A() = higherNibble(A()) | lowerNibble(memory);
	adjustSZPXY<Z80>(F(), A());
	clearFlag(F(), NF | HC);
}

void EightBit::Z80::rld() {
	(MEMPTR() = BUS().ADDRESS() = HL())++;
	const auto memory = BUS().read();
	BUS().write(promoteNibble(memory) | lowNibble(A()));
	A() = higherNibble(A()) | highNibble(memory);
	adjustSZPXY<Z80>(F(), A());
	clearFlag(F(), NF | HC);
}

void EightBit::Z80::writePort(const uint8_t port) {
	MEMPTR() = BUS().ADDRESS() = { port, A() };
	BUS().DATA() = A();
	writePort();
	++MEMPTR().low;
}

void EightBit::Z80::writePort() {
	m_ports.write(BUS().ADDRESS().low, BUS().DATA());
}

uint8_t EightBit::Z80::readPort(const uint8_t port) {
	MEMPTR() = BUS().ADDRESS() = { port, A() };
	++MEMPTR().low;
	return readPort();
}

uint8_t EightBit::Z80::readPort() {
	return BUS().DATA() = m_ports.read(BUS().ADDRESS().low);
}

int EightBit::Z80::step() {
	ExecutingInstruction.fire(*this);
	m_displaced = m_prefixCB = m_prefixDD = m_prefixED = m_prefixFD = false;
	resetCycles();
	if (LIKELY(powered())) {
		lower(M1());
		if (UNLIKELY(lowered(RESET()))) {
			handleRESET();
		} else if (UNLIKELY(lowered(NMI()))) {
			handleNMI();
		} else if (UNLIKELY(lowered(INT()))) {
			handleINT();
		} else if (UNLIKELY(lowered(HALT()))) {
			execute(0);	// NOP
		} else {
			execute(fetchByte());
		}
	}
	ExecutedInstruction.fire(*this);
	return cycles();
}

int EightBit::Z80::execute(const uint8_t opcode) {

	ASSUME(lowered(M1()));

	if (LIKELY(!(m_prefixCB && m_displaced))) {
		++REFRESH();
		raise(M1());
	}

	const auto& decoded = getDecodedOpcode(opcode);

	const auto x = decoded.x;
	const auto y = decoded.y;
	const auto z = decoded.z;

	const auto p = decoded.p;
	const auto q = decoded.q;

	const auto prefixed = m_prefixCB || m_prefixED;
	if (LIKELY(!prefixed)) {
		executeOther(x, y, z, p, q);
	} else {
		if (m_prefixCB)
			executeCB(x, y, z);
		else if (m_prefixED)
			executeED(x, y, z, p, q);
		else
			UNREACHABLE;
	}

	ASSUME(cycles() > 0);
	return cycles();
}

void EightBit::Z80::executeCB(const int x, const int y, const int z) {
	ASSUME(x >= 0);
	ASSUME(x <= 3);
	ASSUME(y >= 0);
	ASSUME(y <= 7);
	ASSUME(z >= 0);
	ASSUME(z <= 7);
	switch (x) {
	case 0:	{ // rot[y] r[z]
		auto operand = LIKELY(!m_displaced) ? R(z) : BUS().read(displacedAddress());
		switch (y) {
		case 0:
			rlc(operand);
			break;
		case 1:
			rrc(operand);
			break;
		case 2:
			rl(operand);
			break;
		case 3:
			rr(operand);
			break;
		case 4:
			sla(operand);
			break;
		case 5:
			sra(operand);
			break;
		case 6:
			sll(operand);
			break;
		case 7:
			srl(operand);
			break;
		default:
			UNREACHABLE;
		}
		adjustSZP<Z80>(F(), operand);
		if (UNLIKELY(m_displaced)) {
			if (LIKELY(z != 6))
				R2(z, operand);
			BUS().write(operand);
			addCycles(15);
		} else {
			R(z, operand);
			if (UNLIKELY(z == 6))
				addCycles(7);
		}
		addCycles(8);
		break;
	} case 1:	// BIT y, r[z]
		addCycles(8);
		if (UNLIKELY(m_displaced)) {
			bit(y, BUS().read(displacedAddress()));
			adjustXY<Z80>(F(), MEMPTR().high);
			addCycles(12);
		} else {
			const auto operand = bit(y, R(z));
			if (UNLIKELY(z == 6)) {
				adjustXY<Z80>(F(), MEMPTR().high);
				addCycles(4);
			} else {
				adjustXY<Z80>(F(), operand);
			}
		}
		break;
	case 2:	// RES y, r[z]
		addCycles(8);
		if (UNLIKELY(m_displaced)) {
			auto operand = BUS().read(displacedAddress());
			operand = res(y, operand);
			BUS().write(operand);
			R2(z, operand);
			addCycles(15);
		} else {
			R(z, res(y, R(z)));
			if (UNLIKELY(z == 6))
				addCycles(7);
		}
		break;
	case 3:	// SET y, r[z]
		addCycles(8);
		if (UNLIKELY(m_displaced)) {
			auto operand = BUS().read(displacedAddress());
			operand = set(y, operand);
			BUS().write(operand);
			R2(z, operand);
			addCycles(15);
		} else {
			R(z, set(y, R(z)));
			if (UNLIKELY(z == 6))
				addCycles(7);
		}
		break;
	default:
		UNREACHABLE;
	}
}

void EightBit::Z80::executeED(const int x, const int y, const int z, const int p, const int q) {
	ASSUME(x >= 0);
	ASSUME(x <= 3);
	ASSUME(y >= 0);
	ASSUME(y <= 7);
	ASSUME(z >= 0);
	ASSUME(z <= 7);
	ASSUME(p >= 0);
	ASSUME(p <= 3);
	ASSUME(q >= 0);
	ASSUME(q <= 1);
	switch (x) {
	case 0:
	case 3:	// Invalid instruction, equivalent to NONI followed by NOP
		addCycles(8);
		break;
	case 1:
		switch (z) {
		case 0: // Input from port with 16-bit address
			(MEMPTR() = BUS().ADDRESS() = BC())++;
			readPort();
			if (LIKELY(y != 6))	// IN r[y],(C)
				R(y, BUS().DATA());
			adjustSZPXY<Z80>(F(), BUS().DATA());
			clearFlag(F(), NF | HC);
			addCycles(12);
			break;
		case 1:	// Output to port with 16-bit address
			(MEMPTR() = BUS().ADDRESS() = BC())++;
			if (UNLIKELY(y == 6))	// OUT (C),0
				BUS().DATA() = 0;
			else		// OUT (C),r[y]
				BUS().DATA() = R(y);
			writePort();
			addCycles(12);
			break;
		case 2:	// 16-bit add/subtract with carry
			switch (q) {
			case 0:	// SBC HL, rp[p]
				sbc(RP(p));
				break;
			case 1:	// ADC HL, rp[p]
				adc(RP(p));
				break;
			default:
				UNREACHABLE;
			}
			addCycles(15);
			break;
		case 3:	// Retrieve/store register pair from/to immediate address
			BUS().ADDRESS() = fetchWord();
			switch (q) {
			case 0: // LD (nn), rp[p]
				setWord(RP(p));
				break;
			case 1:	// LD rp[p], (nn)
				RP(p) = getWord();
				break;
			default:
				UNREACHABLE;
			}
			addCycles(20);
			break;
		case 4:	// Negate accumulator
			neg();
			addCycles(8);
			break;
		case 5:	// Return from interrupt
			switch (y) {
			case 1:
				reti();	// RETI
				break;
			default:
				retn();	// RETN
				break;
			}
			addCycles(14);
			break;
		case 6:	// Set interrupt mode
			switch (y) {
			case 0:
			case 4:
				IM() = 0;
				break;
			case 2:
			case 6:
				IM() = 1;
				break;
			case 3:
			case 7:
				IM() = 2;
				break;
			case 1:
			case 5:
				IM() = 0;
				break;
			default:
				UNREACHABLE;
			}
			addCycles(8);
			break;
		case 7:	// Assorted ops
			switch (y) {
			case 0:	// LD I,A
				IV() = A();
				addCycles(9);
				break;
			case 1:	// LD R,A
				REFRESH() = A();
				addCycles(9);
				break;
			case 2:	// LD A,I
				adjustSZXY<Z80>(F(), A() = IV());
				clearFlag(F(), NF | HC);
				setFlag(F(), PF, IFF2());
				addCycles(9);
				break;
			case 3:	// LD A,R
				adjustSZXY<Z80>(F(), A() = REFRESH());
				clearFlag(F(), NF | HC);
				setFlag(F(), PF, IFF2());
				addCycles(9);
				break;
			case 4:	// RRD
				rrd();
				addCycles(18);
				break;
			case 5:	// RLD
				rld();
				addCycles(18);
				break;
			case 6:	// NOP
			case 7:	// NOP
				addCycles(4);
				break;
			default:
				UNREACHABLE;
			}
			break;
		default:
			UNREACHABLE;
		}
		break;
	case 2:
		switch (z) {
		case 0:	// LD
			switch (y) {
			case 4:	// LDI
				ldi();
				break;
			case 5:	// LDD
				ldd();
				break;
			case 6:	// LDIR
				if (LIKELY(ldir())) {
					MEMPTR() = --PC();
					--PC();
					addCycles(5);
				}
				break;
			case 7:	// LDDR
				if (LIKELY(lddr())) {
					MEMPTR() = --PC();
					--PC();
					addCycles(5);
				}
				break;
			}
			break;
		case 1:	// CP
			switch (y) {
			case 4:	// CPI
				cpi();
				break;
			case 5:	// CPD
				cpd();
				break;
			case 6:	// CPIR
				if (LIKELY(cpir())) {
					MEMPTR() = --PC();
					--PC();
					addCycles(5);
				}
				break;
			case 7:	// CPDR
				if (LIKELY(cpdr())) {
					MEMPTR() = --PC();
					--PC();
					addCycles(5);
				} else {
					MEMPTR() = PC() - 2;
				}
				break;
			}
			break;
		case 2:	// IN
			switch (y) {
			case 4:	// INI
				ini();
				break;
			case 5:	// IND
				ind();
				break;
			case 6:	// INIR
				if (LIKELY(inir())) {
					PC() -= 2;
					addCycles(5);
				}
				break;
			case 7:	// INDR
				if (LIKELY(indr())) {
					PC() -= 2;
					addCycles(5);
				}
				break;
			}
			break;
		case 3:	// OUT
			switch (y) {
			case 4:	// OUTI
				outi();
				break;
			case 5:	// OUTD
				outd();
				break;
			case 6:	// OTIR
				if (LIKELY(otir())) {
					PC() -= 2;
					addCycles(5);
				}
				break;
			case 7:	// OTDR
				if (LIKELY(otdr())) {
					PC() -= 2;
					addCycles(5);
				}
				break;
			}
			break;
		}
		addCycles(16);
		break;
	}
}

void EightBit::Z80::executeOther(const int x, const int y, const int z, const int p, const int q) {
	ASSUME(x >= 0);
	ASSUME(x <= 3);
	ASSUME(y >= 0);
	ASSUME(y <= 7);
	ASSUME(z >= 0);
	ASSUME(z <= 7);
	ASSUME(p >= 0);
	ASSUME(p <= 3);
	ASSUME(q >= 0);
	ASSUME(q <= 1);
	switch (x) {
	case 0:
		switch (z) {
		case 0:	// Relative jumps and assorted ops
			switch (y) {
			case 0:	// NOP
				addCycles(4);
				break;
			case 1:	// EX AF AF'
				exxAF();
				addCycles(4);
				break;
			case 2:	// DJNZ d
				if (LIKELY(jrConditional(--B())))
					addCycles(5);
				addCycles(8);
				break;
			case 3:	// JR d
				jr(fetchByte());
				addCycles(12);
				break;
			case 4: // JR cc,d
			case 5:
			case 6:
			case 7:
				if (UNLIKELY(jrConditionalFlag(y - 4)))
					addCycles(5);
				addCycles(5);
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0: // LD rp,nn
				RP(p) = fetchWord();
				addCycles(10);
				break;
			case 1:	// ADD HL,rp
				add(RP(p));
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
					(MEMPTR() = BUS().ADDRESS() = BC())++;
					MEMPTR().high = BUS().DATA() = A();
					BUS().write();
					addCycles(7);
					break;
				case 1:	// LD (DE),A
					(MEMPTR() = BUS().ADDRESS() = DE())++;
					MEMPTR().high = BUS().DATA() = A();
					BUS().write();
					addCycles(7);
					break;
				case 2:	// LD (nn),HL
					BUS().ADDRESS() = fetchWord();
					setWord(HL2());
					addCycles(16);
					break;
				case 3: // LD (nn),A
					(MEMPTR() = BUS().ADDRESS() = fetchWord())++;
					MEMPTR().high = BUS().DATA() = A();
					BUS().write();
					addCycles(13);
					break;
				default:
					UNREACHABLE;
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					(MEMPTR() = BUS().ADDRESS() = BC())++;
					A() = BUS().read();
					addCycles(7);
					break;
				case 1:	// LD A,(DE)
					(MEMPTR() = BUS().ADDRESS() = DE())++;
					A() = BUS().read();
					addCycles(7);
					break;
				case 2:	// LD HL,(nn)
					BUS().ADDRESS() = fetchWord();
					HL2() = getWord();
					addCycles(16);
					break;
				case 3:	// LD A,(nn)
					(MEMPTR() = BUS().ADDRESS() = fetchWord())++;
					A() = BUS().read();
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
				++RP(p);
				break;
			case 1:	// DEC rp
				--RP(p);
				break;
			default:
				UNREACHABLE;
			}
			addCycles(6);
			break;
		case 4: { // 8-bit INC
			if (UNLIKELY(m_displaced && (y == 6)))
				fetchDisplacement();
			auto operand = R(y);
			increment(operand);
			R(y, operand);
			addCycles(4);
			break;
		} case 5: {	// 8-bit DEC
			if (UNLIKELY(y == 6)) {
				addCycles(7);
				if (UNLIKELY(m_displaced))
					fetchDisplacement();
			}
			auto operand = R(y);
			decrement(operand);
			R(y, operand);
			addCycles(4);
			break;
		} case 6:	// 8-bit load immediate
			if (UNLIKELY(y == 6)) {
				addCycles(3);
				if (UNLIKELY(m_displaced))
					fetchDisplacement();
			}
			R(y, fetchByte());	// LD r,n
			addCycles(7);
			break;
		case 7:	// Assorted operations on accumulator/flags
			switch (y) {
			case 0:
				rlc(A());
				break;
			case 1:
				rrc(A());
				break;
			case 2:
				rl(A());
				break;
			case 3:
				rr(A());
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
			bool normal = true;
			if (UNLIKELY(m_displaced)) {
				if (UNLIKELY(z == 6)) {
					fetchDisplacement();
					switch (y) {
					case 4:
						H() = R(z);
						normal = false;
						break;
					case 5:
						L() = R(z);
						normal = false;
						break;
					}
				}
				if (UNLIKELY(y == 6)) {
					fetchDisplacement();
					switch (z) {
					case 4:
						R(y, H());
						normal = false;
						break;
					case 5:
						R(y, L());
						normal = false;
						break;
					}
				}
			}
			if (LIKELY(normal))
				R(y, R(z));
			if (UNLIKELY((y == 6) || (z == 6)))	// M operations
				addCycles(3);
		}
		addCycles(4);
		break;
	case 2: { // Operate on accumulator and register/memory location
		if (UNLIKELY(z == 6)) {
			addCycles(3);
			if (UNLIKELY(m_displaced))
				fetchDisplacement();
		}
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
			sbc(value);
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
		addCycles(4);
		break;
	}
	case 3:
		switch (z) {
		case 0:	// Conditional return
			if (UNLIKELY(returnConditionalFlag(y)))
				addCycles(6);
			addCycles(5);
			break;
		case 1:	// POP & various ops
			switch (q) {
			case 0:	// POP rp2[p]
				RP2(p) = popWord();
				addCycles(10);
				break;
			case 1:
				switch (p) {
				case 0:	// RET
					ret();
					addCycles(10);
					break;
				case 1:	// EXX
					exx();
					addCycles(4);
					break;
				case 2:	// JP HL
					jump(HL2());
					addCycles(4);
					break;
				case 3:	// LD SP,HL
					SP() = HL2();
					addCycles(4);
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
			jumpConditionalFlag(y);
			addCycles(10);
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0:	// JP nn
				jump(MEMPTR() = fetchWord());
				addCycles(10);
				break;
			case 1:	// CB prefix
				m_prefixCB = true;
				if (UNLIKELY(m_displaced))
					fetchDisplacement();
				lower(M1());
				execute(fetchByte());
				break;
			case 2:	// OUT (n),A
				writePort(fetchByte());
				addCycles(11);
				break;
			case 3:	// IN A,(n)
				A() = readPort(fetchByte());
				addCycles(11);
				break;
			case 4:	// EX (SP),HL
				xhtl();
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
			default:
				UNREACHABLE;
			}
			break;
		case 4:	// Conditional call: CALL cc[y], nn
			if (UNLIKELY(callConditionalFlag(y)))
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
					call(MEMPTR() = fetchWord());
					addCycles(17);
					break;
				case 1:	// DD prefix
					m_displaced = m_prefixDD = true;
					lower(M1());
					execute(fetchByte());
					break;
				case 2:	// ED prefix
					m_prefixED = true;
					lower(M1());
					execute(fetchByte());
					break;
				case 3:	// FD prefix
					m_displaced = m_prefixFD = true;
					lower(M1());
					execute(fetchByte());
					break;
				default:
					UNREACHABLE;
				}
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 6: { // Operate on accumulator and immediate operand: alu[y] n
			const auto operand = fetchByte();
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
				sbc(operand);
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
			addCycles(7);
			break;
		}
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

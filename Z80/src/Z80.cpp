#include "stdafx.h"
#include "Z80.h"

// based on http://www.z80.info/decoding.htm

EightBit::Z80::Z80(Bus& bus, InputOutput& ports)
: IntelProcessor(bus),
  m_ports(ports) {
	RaisedPOWER.connect([this](EventArgs) {

		raiseM1();
		raiseIORQ();
		raiseRD();
		raiseWR();

		di();
		IM() = 0;

		REFRESH() = 0;
		IV() = Mask8;

		exxAF();
		exx();

		AF() = IX() = IY() = BC() = DE() = HL() = Mask16;

		m_prefixCB = m_prefixDD = m_prefixED = m_prefixFD = false;
	});

	LoweredM1.connect([this](EventArgs) {
		++REFRESH();
	});
}

DEFINE_PIN_LEVEL_CHANGERS(NMI, Z80);
DEFINE_PIN_LEVEL_CHANGERS(M1, Z80);
DEFINE_PIN_LEVEL_CHANGERS(MREQ, Z80);
DEFINE_PIN_LEVEL_CHANGERS(IORQ, Z80);
DEFINE_PIN_LEVEL_CHANGERS(RD, Z80);
DEFINE_PIN_LEVEL_CHANGERS(WR, Z80);

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

void EightBit::Z80::busWrite() {
	lowerMREQ();
	lowerWR();
	IntelProcessor::busWrite();
	raiseWR();
	raiseMREQ();
}

uint8_t EightBit::Z80::busRead() {
	lowerMREQ();
	lowerRD();
	const auto returned = IntelProcessor::busRead();
	raiseRD();
	raiseMREQ();
	return returned;
}

void EightBit::Z80::handleRESET() {
	IntelProcessor::handleRESET();
	di();
	tick(3);
}

void EightBit::Z80::handleNMI() {
	raiseNMI();
	raiseHALT();
	IFF1() = false;
	lowerM1();
	const auto discarded = BUS().DATA();
	raiseM1();
	restart(0x66);
	tick(13);
}

void EightBit::Z80::handleINT() {
	IntelProcessor::handleINT();
	lowerM1();
	lowerIORQ();
	const auto data = BUS().DATA();
	raiseIORQ();
	raiseM1();
	di();
	switch (IM()) {
	case 0:		// i8080 equivalent
		execute(data);
		break;
	case 1:
		restart(7 << 3);
		tick(13);
		break;
	case 2:
		call(MEMPTR() = register16_t(data, IV()));
		tick(19);
		break;
	default:
		UNREACHABLE;
	}
}

void EightBit::Z80::di() {
	IFF1() = IFF2() = false;
}

void EightBit::Z80::ei() {
	IFF1() = IFF2() = true;
}

uint8_t EightBit::Z80::increment(const uint8_t operand) {
	clearFlag(F(), NF);
	const uint8_t result = operand + 1;
	adjustSZXY<Z80>(F(), result);
	setFlag(F(), VF, result == Bit7);
	clearFlag(F(), HC, lowNibble(result));
	return result;
}

uint8_t EightBit::Z80::decrement(const uint8_t operand) {
	setFlag(F(), NF);
	clearFlag(F(), HC, lowNibble(operand));
	const uint8_t result = operand - 1;
	adjustSZXY<Z80>(F(), result);
	setFlag(F(), VF, result == Mask7);
	return result;
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

EightBit::register16_t EightBit::Z80::sbc(const register16_t operand, const register16_t value) {

	const auto subtraction = operand.word - value.word - (F() & CF);

	const register16_t result = subtraction;

	setFlag(F(), NF);
	clearFlag(F(), ZF, result.word);
	setFlag(F(), CF, subtraction & Bit16);
	adjustHalfCarrySub(F(), operand.high, value.high, result.high);
	adjustXY<Z80>(F(), result.high);

	const auto beforeNegative = operand.high & SF;
	const auto valueNegative = value.high & SF;
	const auto afterNegative = result.high & SF;

	setFlag(F(), SF, afterNegative);
	adjustOverflowSub(F(), beforeNegative, valueNegative, afterNegative);

	MEMPTR() = operand + 1;

	return result;
}

EightBit::register16_t EightBit::Z80::adc(const register16_t operand, const register16_t value) {

	const auto result = add(operand, value, F() & CF);
	clearFlag(F(), ZF, result.word);

	const auto beforeNegative = operand.high & SF;
	const auto valueNegative = value.high & SF;
	const auto afterNegative = result.high & SF;

	setFlag(F(), SF, afterNegative);
	adjustOverflowAdd(F(), beforeNegative, valueNegative, afterNegative);

	return result;
}

EightBit::register16_t EightBit::Z80::add(const register16_t operand, const register16_t value, int carry) {

	const int addition = operand.word + value.word + carry;
	const register16_t result = addition;

	clearFlag(F(), NF);
	setFlag(F(), CF, addition & Bit16);
	adjustHalfCarryAdd(F(), operand.high, value.high, result.high);
	adjustXY<Z80>(F(), result.high);

	MEMPTR() = operand + 1;

	return result;
}

uint8_t EightBit::Z80::add(const uint8_t operand, const uint8_t value, const int carry) {

	const register16_t addition = operand + value + carry;
	const auto result = addition.low;

	adjustHalfCarryAdd(F(), operand, value, result);
	adjustOverflowAdd(F(), operand, value, result);

	clearFlag(F(), NF);
	setFlag(F(), CF, addition.high & CF);
	adjustSZXY<Z80>(F(), result);

	return result;
}

uint8_t EightBit::Z80::adc(const uint8_t operand, const uint8_t value) {
	return add(operand, value, F() & CF);
}

uint8_t EightBit::Z80::subtract(const uint8_t operand, const uint8_t value, const int carry) {

	const register16_t subtraction = operand - value - carry;
	const auto result = subtraction.low;

	adjustHalfCarrySub(F(), operand, value, result);
	adjustOverflowSub(F(), operand, value, result);

	setFlag(F(), NF);
	setFlag(F(), CF, subtraction.high & CF);
	adjustSZ<Z80>(F(), result);

	return result;
}

uint8_t EightBit::Z80::sub(const uint8_t operand, const uint8_t value, const int carry) {
	const auto subtraction = subtract(operand, value, carry);
	adjustXY<Z80>(F(), subtraction);
	return subtraction;
}

uint8_t EightBit::Z80::sbc(const uint8_t operand, const uint8_t value) {
	return sub(operand, value, F() & CF);
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
	subtract(A(), value);
	adjustXY<Z80>(F(), value);
}

uint8_t EightBit::Z80::rlc(const uint8_t operand) {
	clearFlag(F(), NF | HC);
	const auto carry = operand & Bit7;
	setFlag(F(), CF, carry);
	const uint8_t result = (operand << 1) | (carry >> 7);
	adjustXY<Z80>(F(), result);
	return result;
}

uint8_t EightBit::Z80::rrc(const uint8_t operand) {
	clearFlag(F(), NF | HC);
	const auto carry = operand & Bit0;
	setFlag(F(), CF, carry);
	const uint8_t result = (operand >> 1) | (carry << 7);
	adjustXY<Z80>(F(), result);
	return result;
}

uint8_t EightBit::Z80::rl(const uint8_t operand) {
	clearFlag(F(), NF | HC);
	const auto carry = F() & CF;
	setFlag(F(), CF, operand & Bit7);
	const uint8_t result = (operand << 1) | carry;
	adjustXY<Z80>(F(), result);
	return result;
}

uint8_t EightBit::Z80::rr(const uint8_t operand) {
	clearFlag(F(), NF | HC);
	const auto carry = F() & CF;
	setFlag(F(), CF, operand & Bit0);
	const uint8_t result = (operand >> 1) | (carry << 7);
	adjustXY<Z80>(F(), result);
	return result;
}

//

uint8_t EightBit::Z80::sla(const uint8_t operand) {
	clearFlag(F(), NF | HC);
	setFlag(F(), CF, operand & Bit7);
	const uint8_t result = operand << 1;
	adjustXY<Z80>(F(), result);
	return result;
}

uint8_t EightBit::Z80::sra(const uint8_t operand) {
	clearFlag(F(), NF | HC);
	setFlag(F(), CF, operand & Bit0);
	const uint8_t result = (operand >> 1) | (operand & Bit7);
	adjustXY<Z80>(F(), result);
	return result;
}

uint8_t EightBit::Z80::sll(const uint8_t operand) {
	clearFlag(F(), NF | HC);
	setFlag(F(), CF, operand & Bit7);
	const uint8_t result = (operand << 1) | Bit0;
	adjustXY<Z80>(F(), result);
	return result;
}

uint8_t EightBit::Z80::srl(const uint8_t operand) {
	clearFlag(F(), NF | HC);
	setFlag(F(), CF, operand & Bit0);
	const uint8_t result = (operand >> 1) & ~Bit7;
	adjustXY<Z80>(F(), result);
	setFlag(F(), ZF, result);
	return result;
}

void EightBit::Z80::bit(const int n, const uint8_t operand) {
	ASSUME(n >= 0);
	ASSUME(n <= 7);
	setFlag(F(), HC);
	clearFlag(F(), NF);
	const auto discarded = operand & (1 << n);
	adjustSZ<Z80>(F(), discarded);
	clearFlag(F(), PF, discarded);
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

void EightBit::Z80::xhtl(register16_t& exchange) {
	MEMPTR().low = IntelProcessor::busRead(SP());
	++BUS().ADDRESS();
	MEMPTR().high = IntelProcessor::busRead();
	IntelProcessor::busWrite(exchange.high);
	exchange.high = MEMPTR().high;
	--BUS().ADDRESS();
	IntelProcessor::busWrite(exchange.low);
	exchange.low = MEMPTR().low;
}

void EightBit::Z80::blockCompare(const register16_t source, register16_t& counter) {

	const auto value = IntelProcessor::busRead(source);
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
	const auto value = IntelProcessor::busRead(source);
	IntelProcessor::busWrite(destination, value);
	const auto xy = A() + value;
	setFlag(F(), XF, xy & Bit3);
	setFlag(F(), YF, xy & Bit1);
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
	IntelProcessor::busWrite(destination, value);
	source.high = decrement(source.high);
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
	const auto value = IntelProcessor::busRead(source);
	destination.high = decrement(destination.high);
	BUS().ADDRESS() = destination;
	writePort();
	MEMPTR() = destination;
	setFlag(F(), NF, value & Bit7);
	setFlag(F(), HC | CF, (L() + value) > 0xff);
	adjustParity<Z80>(F(), ((value + L()) & Mask3) ^ B());
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
	const auto memory = busRead();
	IntelProcessor::busWrite(promoteNibble(A()) | highNibble(memory));
	A() = higherNibble(A()) | lowerNibble(memory);
	adjustSZPXY<Z80>(F(), A());
	clearFlag(F(), NF | HC);
}

void EightBit::Z80::rld() {
	(MEMPTR() = BUS().ADDRESS() = HL())++;
	const auto memory = busRead();
	IntelProcessor::busWrite(promoteNibble(memory) | lowNibble(A()));
	A() = higherNibble(A()) | highNibble(memory);
	adjustSZPXY<Z80>(F(), A());
	clearFlag(F(), NF | HC);
}

void EightBit::Z80::writePort(const uint8_t port) {
	MEMPTR() = BUS().ADDRESS() = register16_t(port, A());
	BUS().DATA() = A();
	writePort();
	++MEMPTR().low;
}

void EightBit::Z80::writePort() {
	lowerIORQ();
	lowerWR();
	m_ports.write(BUS().ADDRESS().low, BUS().DATA());
	raiseWR();
	raiseIORQ();
}

uint8_t EightBit::Z80::readPort(const uint8_t port) {
	MEMPTR() = BUS().ADDRESS() = register16_t(port, A());
	++MEMPTR().low;
	return readPort();
}

uint8_t EightBit::Z80::readPort() {
	lowerIORQ();
	lowerRD();
	const auto returned = BUS().DATA() = m_ports.read(BUS().ADDRESS().low);
	raiseRD();
	raiseIORQ();
	return returned;
}

int EightBit::Z80::step() {
	resetCycles();
	ExecutingInstruction.fire(*this);
	if (LIKELY(powered())) {
		m_displaced = m_prefixCB = m_prefixDD = m_prefixED = m_prefixFD = false;
		bool handled = false;
		if (lowered(RESET())) {
			handleRESET();
			handled = true;
		} else if (lowered(NMI())) {
			handleNMI();
			handled = true;
		} else if (lowered(INT())) {
			raiseHALT();
			if (IFF1()) {
				handleINT();
				handled = true;
			}
		} else if (lowered(HALT())) {
			execute(0);	// NOP
			handled = true;
		}
		if (!handled) {
			lowerM1();
			execute(fetchByte());
		}
	}
	ExecutedInstruction.fire(*this);
	return cycles();
}

int EightBit::Z80::execute() {

	raiseM1();

	const auto& decoded = getDecodedOpcode(opcode());

	const auto x = decoded.x;
	const auto y = decoded.y;
	const auto z = decoded.z;

	const auto p = decoded.p;
	const auto q = decoded.q;

	if (m_prefixCB)
		executeCB(x, y, z);
	else if (m_prefixED)
		executeED(x, y, z, p, q);
	else
		executeOther(x, y, z, p, q);

	ASSUME(cycles() > 0);
	return cycles();
}

void EightBit::Z80::executeCB(const int x, const int y, const int z) {
	const bool memoryY = y == 6;
	const bool memoryZ = z == 6;
	const bool indirect = (!m_displaced && memoryZ) || m_displaced;
	auto operand = m_displaced ? IntelProcessor::busRead(displacedAddress()) : R(z);
	const bool update = x != 1; // BIT does not update
	switch (x) {
	case 0:	{ // rot[y] r[z]
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
		case 6:
			operand = sll(operand);
			break;
		case 7:
			operand = srl(operand);
			break;
		default:
			UNREACHABLE;
		}
		adjustSZP<Z80>(F(), operand);
		tick(8);
		break;
	} case 1:	// BIT y, r[z]
		tick(8);
		bit(y, operand);
		if (indirect) {
			adjustXY<Z80>(F(), MEMPTR().high);
			tick(4);
		} else {
			adjustXY<Z80>(F(), operand);
		}
		break;
	case 2:	// RES y, r[z]
		tick(8);
		operand = res(y, operand);
		break;
	case 3:	// SET y, r[z]
		tick(8);
		operand = set(y, operand);
		break;
	default:
		UNREACHABLE;
	}
	if (update) {
		if (m_displaced) {
			IntelProcessor::busWrite(operand);
			if (!memoryZ)
				R2(z, operand);
			tick(15);
		} else {
			R(z, operand);
			if (memoryZ)
				tick(7);
		}
	}
}

void EightBit::Z80::executeED(const int x, const int y, const int z, const int p, const int q) {
	const bool memoryY = y == 6;
	const bool memoryZ = z == 6;
	switch (x) {
	case 0:
	case 3:	// Invalid instruction, equivalent to NONI followed by NOP
		tick(8);
		break;
	case 1:
		switch (z) {
		case 0: // Input from port with 16-bit address
			(MEMPTR() = BUS().ADDRESS() = BC())++;
			readPort();
			if (y != 6)	// IN r[y],(C)
				R(y, BUS().DATA());
			adjustSZPXY<Z80>(F(), BUS().DATA());
			clearFlag(F(), NF | HC);
			tick(12);
			break;
		case 1:	// Output to port with 16-bit address
			(MEMPTR() = BUS().ADDRESS() = BC())++;
			if (y == 6)			// OUT (C),0
				BUS().DATA() = 0;
			else				// OUT (C),r[y]
				BUS().DATA() = R(y);
			writePort();
			tick(12);
			break;
		case 2:	// 16-bit add/subtract with carry
			switch (q) {
			case 0:	// SBC HL, rp[p]
				HL2() = sbc(HL2(), RP(p));
				break;
			case 1:	// ADC HL, rp[p]
				HL2() = adc(HL2(), RP(p));
				break;
			default:
				UNREACHABLE;
			}
			tick(15);
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
			tick(20);
			break;
		case 4:	// Negate accumulator
			neg();
			tick(8);
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
			tick(14);
			break;
		case 6:	// Set interrupt mode
			switch (y) {
			case 0:
			case 1:
			case 4:
			case 5:
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
			default:
				UNREACHABLE;
			}
			tick(8);
			break;
		case 7:	// Assorted ops
			switch (y) {
			case 0:	// LD I,A
				IV() = A();
				tick(9);
				break;
			case 1:	// LD R,A
				REFRESH() = A();
				tick(9);
				break;
			case 2:	// LD A,I
				adjustSZXY<Z80>(F(), A() = IV());
				clearFlag(F(), NF | HC);
				setFlag(F(), PF, IFF2());
				tick(9);
				break;
			case 3:	// LD A,R
				adjustSZXY<Z80>(F(), A() = REFRESH());
				clearFlag(F(), NF | HC);
				setFlag(F(), PF, IFF2());
				tick(9);
				break;
			case 4:	// RRD
				rrd();
				tick(18);
				break;
			case 5:	// RLD
				rld();
				tick(18);
				break;
			case 6:	// NOP
			case 7:	// NOP
				tick(4);
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
				if (ldir()) {
					MEMPTR() = --PC();
					--PC();
					tick(5);
				}
				break;
			case 7:	// LDDR
				if (lddr()) {
					MEMPTR() = --PC();
					--PC();
					tick(5);
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
				if (cpir()) {
					MEMPTR() = --PC();
					--PC();
					tick(5);
				}
				break;
			case 7:	// CPDR
				if (cpdr()) {
					MEMPTR() = --PC();
					--PC();
					tick(5);
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
				if (inir()) {
					PC() -= 2;
					tick(5);
				}
				break;
			case 7:	// INDR
				if (indr()) {
					PC() -= 2;
					tick(5);
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
				if (otir()) {
					PC() -= 2;
					tick(5);
				}
				break;
			case 7:	// OTDR
				if (otdr()) {
					PC() -= 2;
					tick(5);
				}
				break;
			}
			break;
		}
		tick(16);
		break;
	}
}

void EightBit::Z80::executeOther(const int x, const int y, const int z, const int p, const int q) {
	const bool memoryY = y == 6;
	const bool memoryZ = z == 6;
	switch (x) {
	case 0:
		switch (z) {
		case 0:	// Relative jumps and assorted ops
			switch (y) {
			case 0:	// NOP
				if (m_prefixDD)
					tick(4);
				tick(4);
				break;
			case 1:	// EX AF AF'
				exxAF();
				tick(4);
				break;
			case 2:	// DJNZ d
				if (jrConditional(--B()))
					tick(5);
				tick(8);
				break;
			case 3:	// JR d
				jr(fetchByte());
				tick(12);
				break;
			case 4: // JR cc,d
			case 5:
			case 6:
			case 7:
				if (jrConditionalFlag(y - 4))
					tick(5);
				tick(5);
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0: // LD rp,nn
				RP(p) = fetchWord();
				tick(10);
				break;
			case 1:	// ADD HL,rp
				HL2() = add(HL2(), RP(p));
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
					(MEMPTR() = BUS().ADDRESS() = BC())++;
					MEMPTR().high = BUS().DATA() = A();
					busWrite();
					tick(7);
					break;
				case 1:	// LD (DE),A
					(MEMPTR() = BUS().ADDRESS() = DE())++;
					MEMPTR().high = BUS().DATA() = A();
					busWrite();
					tick(7);
					break;
				case 2:	// LD (nn),HL
					BUS().ADDRESS() = fetchWord();
					setWord(HL2());
					tick(16);
					break;
				case 3: // LD (nn),A
					(MEMPTR() = BUS().ADDRESS() = fetchWord())++;
					MEMPTR().high = BUS().DATA() = A();
					busWrite();
					tick(13);
					break;
				default:
					UNREACHABLE;
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					(MEMPTR() = BUS().ADDRESS() = BC())++;
					A() = busRead();
					tick(7);
					break;
				case 1:	// LD A,(DE)
					(MEMPTR() = BUS().ADDRESS() = DE())++;
					A() = busRead();
					tick(7);
					break;
				case 2:	// LD HL,(nn)
					BUS().ADDRESS() = fetchWord();
					HL2() = getWord();
					tick(16);
					break;
				case 3:	// LD A,(nn)
					(MEMPTR() = BUS().ADDRESS() = fetchWord())++;
					A() = busRead();
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
		case 4:	// 8-bit INC
			if (m_displaced && memoryY)
				fetchDisplacement();
			R(y, increment(R(y)));
			tick(4);
			break;
		case 5:	// 8-bit DEC
			if (memoryY) {
				tick(7);
				if (m_displaced)
					fetchDisplacement();
			}
			R(y, decrement(R(y)));
			tick(4);
			break;
		case 6:	// 8-bit load immediate
			if (memoryY) {
				tick(3);
				if (m_displaced)
					fetchDisplacement();
			}
			R(y, fetchByte());	// LD r,n
			tick(7);
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
			tick(4);
			break;
		default:
			UNREACHABLE;
		}
		break;
	case 1:	// 8-bit loading
		if ((memoryZ && memoryY)) { 	// Exception (replaces LD (HL), (HL))
			lowerHALT();
		} else {
			bool normal = true;
			if (m_displaced) {
				if (memoryZ || memoryY)
					fetchDisplacement();
				if (memoryZ) {
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
				if (memoryY) {
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
			if (normal)
				R(y, R(z));
			if (memoryY || memoryZ)	// M operations
				tick(3);
		}
		tick(4);
		break;
	case 2: { // Operate on accumulator and register/memory location
		if (memoryZ) {
			tick(3);
			if (m_displaced)
				fetchDisplacement();
		}
		const auto value = R(z);
		switch (y) {
		case 0:	// ADD A,r
			A() = add(A(), value);
			break;
		case 1:	// ADC A,r
			A() = adc(A(), value);
			break;
		case 2:	// SUB r
			A() = sub(A(), value);
			break;
		case 3:	// SBC A,r
			A() = sbc(A(), value);
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
		break;
	}
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
				case 1:	// EXX
					exx();
					tick(4);
					break;
				case 2:	// JP (HL)
					jump(HL2());
					tick(4);
					break;
				case 3:	// LD SP,HL
					SP() = HL2();
					tick(4);
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
			tick(10);
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0:	// JP nn
				jump(MEMPTR() = fetchWord());
				tick(10);
				break;
			case 1:	// CB prefix
				m_prefixCB = true;
				if (m_displaced)
					fetchDisplacement();
				else
					lowerM1();
				execute(fetchByte());
				break;
			case 2:	// OUT (n),A
				writePort(fetchByte());
				tick(11);
				break;
			case 3:	// IN A,(n)
				A() = readPort(fetchByte());
				tick(11);
				break;
			case 4:	// EX (SP),HL
				xhtl(HL2());
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
			default:
				UNREACHABLE;
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
					call(MEMPTR() = fetchWord());
					tick(17);
					break;
				case 1:	// DD prefix
					m_displaced = m_prefixDD = true;
					lowerM1();
					execute(fetchByte());
					break;
				case 2:	// ED prefix
					m_prefixED = true;
					lowerM1();
					execute(fetchByte());
					break;
				case 3:	// FD prefix
					m_displaced = m_prefixFD = true;
					lowerM1();
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
				A() = add(A(), operand);
				break;
			case 1:	// ADC A,n
				A() = adc(A(), operand);
				break;
			case 2:	// SUB n
				A() = sub(A(), operand);
				break;
			case 3:	// SBC A,n
				A() = sbc(A(), operand);
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

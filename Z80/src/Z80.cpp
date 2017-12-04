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

void EightBit::Z80::reset() {

	IntelProcessor::reset();

	INT() = NMI() = false;
	di();
	IM() = 0;

	REFRESH() = 0;
	IV() = 0xff;

	exxAF();
	exx();

	AF().word = 0xffff;

	BC().word = 0xffff;
	DE().word = 0xffff;
	HL().word = 0xffff;

	IX().word = 0xffff;
	IY().word = 0xffff;

	m_prefixCB = m_prefixDD = m_prefixED = m_prefixFD = false;
}

void EightBit::Z80::di() {
	IFF1() = IFF2() = false;
}

void EightBit::Z80::ei() {
	IFF1() = IFF2() = true;
}

void EightBit::Z80::increment(uint8_t& f, uint8_t& operand) {
	clearFlag(f, NF);
	adjustSZXY<Z80>(f, ++operand);
	setFlag(f, VF, operand == Bit7);
	clearFlag(f, HC, lowNibble(operand));
}

void EightBit::Z80::decrement(uint8_t& f, uint8_t& operand) {
	setFlag(f, NF);
	clearFlag(f, HC, lowNibble(operand));
	adjustSZXY<Z80>(f, --operand);
	setFlag(f, VF, operand == Mask7);
}

bool EightBit::Z80::jrConditionalFlag(uint8_t& f, const int flag) {
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
		UNREACHABLE;
	}
	throw std::logic_error("Unhandled JR conditional");
}

bool EightBit::Z80::jumpConditionalFlag(uint8_t& f, const int flag) {
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

void EightBit::Z80::retn() {
	ret();
	IFF1() = IFF2();
}

void EightBit::Z80::reti() {
	retn();
}

bool EightBit::Z80::returnConditionalFlag(uint8_t& f, const int flag) {
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

bool EightBit::Z80::callConditionalFlag(uint8_t& f, const int flag) {
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

void EightBit::Z80::sbc(uint8_t& f, register16_t& operand, const register16_t value) {

	MEMPTR() = operand;

	const auto beforeNegative = MEMPTR().high & SF;
	const auto valueNegative = value.high & SF;

	const auto result = MEMPTR().word - value.word - (f & CF);
	operand.word = result;

	const auto afterNegative = operand.high & SF;

	setFlag(f, SF, afterNegative);
	clearFlag(f, ZF, operand.word);
	adjustHalfCarrySub(f, MEMPTR().high, value.high, operand.high);
	adjustOverflowSub(f, beforeNegative, valueNegative, afterNegative);
	setFlag(f, NF);
	setFlag(f, CF, result & Bit16);
	adjustXY<Z80>(f, operand.high);

	MEMPTR().word++;
}

void EightBit::Z80::adc(uint8_t& f, register16_t& operand, const register16_t value) {

	MEMPTR() = operand;

	const auto beforeNegative = MEMPTR().high & SF;
	const auto valueNegative = value.high & SF;

	const auto result = MEMPTR().word + value.word + (f & CF);
	operand.word = result;

	const auto afterNegative = operand.high & SF;

	setFlag(f, SF, afterNegative);
	clearFlag(f, ZF, operand.word);
	adjustHalfCarryAdd(f, MEMPTR().high, value.high, operand.high);
	adjustOverflowAdd(f, beforeNegative, valueNegative, afterNegative);
	clearFlag(f, NF);
	setFlag(f, CF, result & Bit16);
	adjustXY<Z80>(f, operand.high);

	MEMPTR().word++;
}

void EightBit::Z80::add(uint8_t& f, register16_t& operand, const register16_t value) {

	MEMPTR() = operand;

	const auto result = MEMPTR().word + value.word;

	operand.word = result;

	clearFlag(f, NF);
	setFlag(f, CF, result & Bit16);
	adjustHalfCarryAdd(f, MEMPTR().high, value.high, operand.high);
	adjustXY<Z80>(f, operand.high);

	MEMPTR().word++;
}

void EightBit::Z80::add(uint8_t& f, uint8_t& operand, const uint8_t value, const int carry) {

	register16_t result;
	result.word = operand + value + carry;

	adjustHalfCarryAdd(f, operand, value, result.low);
	adjustOverflowAdd(f, operand, value, result.low);

	operand = result.low;

	clearFlag(f, NF);
	setFlag(f, CF, result.word & Bit8);
	adjustSZXY<Z80>(f, operand);
}

void EightBit::Z80::adc(uint8_t& f, uint8_t& operand, const uint8_t value) {
	add(f, operand, value, f & CF);
}

void EightBit::Z80::subtract(uint8_t& f, uint8_t& operand, const uint8_t value, const int carry) {

	register16_t result;
	result.word = operand - value - carry;

	adjustHalfCarrySub(f, operand, value, result.low);
	adjustOverflowSub(f, operand, value, result.low);

	operand = result.low;

	setFlag(f, NF);
	setFlag(f, CF, result.word & Bit8);
	adjustSZ<Z80>(f, operand);
}

void EightBit::Z80::sub(uint8_t& f, uint8_t& operand, const uint8_t value, const int carry) {
	subtract(f, operand, value, carry);
	adjustXY<Z80>(f, operand);
}

void EightBit::Z80::sbc(uint8_t& f, uint8_t& operand, const uint8_t value) {
	sub(f, operand, value, f & CF);
}

void EightBit::Z80::andr(uint8_t& f, uint8_t& operand, const uint8_t value) {
	setFlag(f, HC);
	clearFlag(f, CF | NF);
	adjustSZPXY<Z80>(f, operand &= value);
}

void EightBit::Z80::xorr(uint8_t& f, uint8_t& operand, const uint8_t value) {
	clearFlag(f, HC | CF | NF);
	adjustSZPXY<Z80>(f, operand ^= value);
}

void EightBit::Z80::orr(uint8_t& f, uint8_t& operand, const uint8_t value) {
	clearFlag(f, HC | CF | NF);
	adjustSZPXY<Z80>(f, operand |= value);
}

void EightBit::Z80::compare(uint8_t& f, uint8_t check, const uint8_t value) {
	subtract(f, check, value);
	adjustXY<Z80>(f, value);
}

uint8_t EightBit::Z80::rlc(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	const auto carry = operand & Bit7;
	operand = (operand << 1) | (carry >> 7);
	setFlag(f, CF, carry);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t EightBit::Z80::rrc(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	const auto carry = operand & Bit0;
	operand = (operand >> 1) | (carry << 7);
	setFlag(f, CF, carry);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t EightBit::Z80::rl(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	const auto carry = f & CF;
	setFlag(f, CF, operand & Bit7);
	operand = (operand << 1) | carry;
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t EightBit::Z80::rr(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	const auto carry = f & CF;
	setFlag(f, CF, operand & Bit0);
	operand = (operand >> 1) | (carry << 7);
	adjustXY<Z80>(f, operand);
	return operand;
}

//

uint8_t EightBit::Z80::sla(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	setFlag(f, CF, operand & Bit7);
	operand <<= 1;
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t EightBit::Z80::sra(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	setFlag(f, CF, operand & Bit0);
	operand = (operand >> 1) | (operand & Bit7);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t EightBit::Z80::sll(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	setFlag(f, CF, operand & Bit7);
	operand = (operand << 1) | Bit0;
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t EightBit::Z80::srl(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC);
	setFlag(f, CF, operand & Bit0);
	operand = (operand >> 1) & ~Bit7;
	adjustXY<Z80>(f, operand);
	setFlag(f, ZF, operand);
	return operand;
}

uint8_t EightBit::Z80::bit(uint8_t& f, int n, uint8_t operand) {
	setFlag(f, HC);
	clearFlag(f, NF);
	const auto discarded = operand & (1 << n);
	adjustSZXY<Z80>(f, discarded);
	clearFlag(f, PF, discarded);
	return operand;
}

uint8_t EightBit::Z80::res(int n, const uint8_t operand) {
	return operand & ~(1 << n);
}

uint8_t EightBit::Z80::set(int n, const uint8_t operand) {
	return operand | (1 << n);
}

void EightBit::Z80::neg(uint8_t& a, uint8_t& f) {

	setFlag(f, PF, a == Bit7);
	setFlag(f, CF, a);
	setFlag(f, NF);

	const auto original = a;

	a = (~a + 1);	// two's complement

	adjustHalfCarrySub(f, 0U, original, a);
	adjustOverflowSub(f, 0U, original, a);

	adjustSZXY<Z80>(f, a);
}

void EightBit::Z80::daa(uint8_t& a, uint8_t& f) {

	auto updated = a;

	const auto lowAdjust = (f & HC) || (lowNibble(a) > 9);
	const auto highAdjust = (f & CF) || (a > 0x99);

	if (f & NF) {
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

	f = (f & (CF | NF)) | (a > 0x99 ? CF : 0) | ((a ^ updated) & HC);
	a = updated;

	adjustSZPXY<Z80>(f, a);
}

void EightBit::Z80::cpl(uint8_t& a, uint8_t& f) {
	setFlag(f, HC | NF);
	adjustXY<Z80>(f, a = ~a);
}

void EightBit::Z80::scf(const uint8_t a, uint8_t& f) {
	setFlag(f, CF);
	clearFlag(f, HC | NF);
	adjustXY<Z80>(f, a);
}

void EightBit::Z80::ccf(const uint8_t a, uint8_t& f) {
	clearFlag(f, NF);
	const auto carry = f & CF;
	setFlag(f, HC, carry);
	clearFlag(f, CF, carry);
	adjustXY<Z80>(f, a);
}

void EightBit::Z80::xhtl(register16_t& operand) {
	MEMPTR().low = getByte(SP());
	setByte(operand.low);
	operand.low = MEMPTR().low;
	BUS().ADDRESS().word++;
	MEMPTR().high = getByte();
	setByte(operand.high);
	operand.high = MEMPTR().high;
}

void EightBit::Z80::blockCompare(const uint8_t a, uint8_t& f) {

	const auto value = getByte(HL());
	uint8_t result = a - value;

	setFlag(f, PF, --BC().word);

	adjustSZ<Z80>(f, result);
	adjustHalfCarrySub(f, a, value, result);
	setFlag(f, NF);

	result -= ((f & HC) >> 4);

	setFlag(f, YF, result & Bit1);
	setFlag(f, XF, result & Bit3);
}

void EightBit::Z80::cpi(const uint8_t a, uint8_t& f) {
	blockCompare(a, f);
	HL().word++;
	MEMPTR().word++;
}

void EightBit::Z80::cpd(const uint8_t a, uint8_t& f) {
	blockCompare(a, f);
	HL().word--;
	MEMPTR().word--;
}

bool EightBit::Z80::cpir(const uint8_t a, uint8_t& f) {
	cpi(a, f);
	MEMPTR() = PC();
	const auto again = (f & PF) && !(f & ZF);	// See CPI
	if (LIKELY(again))
		MEMPTR().word--;
	return again;
}

bool EightBit::Z80::cpdr(const uint8_t a, uint8_t& f) {
	cpd(a, f);
	MEMPTR().word = PC().word - 1;
	const auto again = (f & PF) && !(f & ZF);	// See CPD
	if (UNLIKELY(!again))
		MEMPTR().word--;
	return again;
}

void EightBit::Z80::blockLoad(const uint8_t a, uint8_t& f, const register16_t source, const register16_t destination) {
	const auto value = getByte(source);
	setByte(destination, value);
	const auto xy = a + value;
	setFlag(f, XF, xy & 8);
	setFlag(f, YF, xy & 2);
	clearFlag(f, NF | HC);
	setFlag(f, PF, --BC().word);
}

void EightBit::Z80::ldd(const uint8_t a, uint8_t& f) {
	blockLoad(a, f, HL(), DE());
	HL().word--;
	DE().word--;
}

void EightBit::Z80::ldi(const uint8_t a, uint8_t& f) {
	blockLoad(a, f, HL(), DE());
	HL().word++;
	DE().word++;
}

bool EightBit::Z80::ldir(const uint8_t a, uint8_t& f) {
	ldi(a, f);
	const auto again = (f & PF) != 0;
	if (LIKELY(again))		// See LDI
		MEMPTR().word = PC().word - 1;
	return again;
}

bool EightBit::Z80::lddr(const uint8_t a, uint8_t& f) {
	ldd(a, f);
	const auto again = (f & PF) != 0;
	if (LIKELY(again))		// See LDR
		MEMPTR().word = PC().word - 1;
	return again;
}

void EightBit::Z80::ini(uint8_t& f) {
	MEMPTR() = BUS().ADDRESS() = BC();
	MEMPTR().word++;
	readPort();
	auto value = BUS().DATA();
	setByte(HL().word++, value);
	decrement(f, B());
	setFlag(f, NF);
}

void EightBit::Z80::ind(uint8_t& f) {
	MEMPTR() = BUS().ADDRESS() = BC();
	MEMPTR().word--;
	readPort();
	auto value = BUS().DATA();
	setByte(HL().word--, value);
	decrement(f, B());
	setFlag(f, NF);
}

bool EightBit::Z80::inir(uint8_t& f) {
	ini(f);
	return !(f & ZF);	// See INI
}

bool EightBit::Z80::indr(uint8_t& f) {
	ind(f);
	return !(f & ZF);	// See IND
}

void EightBit::Z80::blockOut(uint8_t& f) {
	const auto value = getByte();
	BUS().ADDRESS() = BC();
	writePort();
	decrement(f, B());
	setFlag(f, NF, value & Bit7);
	setFlag(f, HC | CF, (L() + value) > 0xff);
	adjustParity<Z80>(f, ((value + L()) & 7) ^ B());
}

void EightBit::Z80::outi(uint8_t& f) {
	BUS().ADDRESS().word = HL().word++;
	blockOut(f);
	MEMPTR().word = BC().word + 1;
}

void EightBit::Z80::outd(uint8_t& f) {
	BUS().ADDRESS().word = HL().word--;
	blockOut(f);
	MEMPTR().word = BC().word - 1;
}

bool EightBit::Z80::otir(uint8_t& f) {
	outi(f);
	return !(f & ZF);	// See OUTI
}

bool EightBit::Z80::otdr(uint8_t& f) {
	outd(f);
	return !(f & ZF);	// See OUTD
}

void EightBit::Z80::rrd(uint8_t& a, uint8_t& f) {
	MEMPTR() = HL();
	memptrReference();
	const auto memory = getByte();
	setByte(promoteNibble(a) | highNibble(memory));
	a = (a & 0xf0) | lowNibble(memory);
	adjustSZPXY<Z80>(f, a);
	clearFlag(f, NF | HC);
}

void EightBit::Z80::rld(uint8_t& a, uint8_t& f) {
	MEMPTR() = HL();
	memptrReference();
	const auto memory = getByte();
	setByte(promoteNibble(memory) | lowNibble(a));
	a = (a & 0xf0) | highNibble(memory);
	adjustSZPXY<Z80>(f, a);
	clearFlag(f, NF | HC);
}

void EightBit::Z80::writePort(const uint8_t port, const uint8_t data) {
	BUS().ADDRESS().low = port;
	BUS().ADDRESS().high = data;
	MEMPTR() = BUS().ADDRESS();
	BUS().placeDATA(data);
	writePort();
	MEMPTR().low++;
}

void EightBit::Z80::writePort() {
	m_ports.write(BUS().ADDRESS().low, BUS().DATA());
}

void EightBit::Z80::readPort(const uint8_t port, uint8_t& a) {
	BUS().ADDRESS().low = port;
	BUS().ADDRESS().high = a;
	MEMPTR() = BUS().ADDRESS();
	readPort();
	a = BUS().DATA();
	MEMPTR().low++;
}

void EightBit::Z80::readPort() {
	BUS().placeDATA(m_ports.read(BUS().ADDRESS().low));
}

int EightBit::Z80::step() {
	ExecutingInstruction.fire(*this);
	m_displaced = m_prefixCB = m_prefixDD = m_prefixED = m_prefixFD = false;
	resetCycles();
	if (LIKELY(powered())) {
		M1() = true;
		if (UNLIKELY(NMI())) {
			NMI() = IFF1() = false;
			restart(0x66);
			addCycles(13);
			return cycles();
		}
		if (UNLIKELY(INT())) {
			INT() = false;
			if (IFF1()) {
				di();
				switch (IM()) {
				case 0:		// i8080 equivalent
					return execute(BUS().DATA());
				case 1:
					restart(7 << 3);
					addCycles(13);
					return cycles();
				case 2:
					pushWord(PC());
					PC().low = BUS().DATA();
					PC().high = IV();
					addCycles(19);
					return cycles();
				default:
					UNREACHABLE;
				}
			}
		}
		return execute(fetchByte());
	}
	return cycles();
}

int EightBit::Z80::execute(const uint8_t opcode) {

	if (UNLIKELY(!M1()))
		throw std::logic_error("M1 cannot be high");

	if (LIKELY(!(m_prefixCB && m_displaced))) {
		++REFRESH();
		M1() = false;
	}

	const auto& decoded = getDecodedOpcode(opcode);

	const auto x = decoded.x;
	const auto y = decoded.y;
	const auto z = decoded.z;

	const auto p = decoded.p;
	const auto q = decoded.q;

	auto prefixed = m_prefixCB || m_prefixED;
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

	if (UNLIKELY(cycles() == 0))
		throw std::logic_error("Unhandled opcode");

	return cycles();
}

void EightBit::Z80::executeCB(const int x, const int y, const int z) {
	auto& a = A();
	auto& f = F();
	switch (x) {
	case 0:	{ // rot[y] r[z]
		auto operand = LIKELY(!m_displaced) ? R(z, a) : getByte(displacedAddress());
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
		case 6:
			operand = sll(f, operand);
			break;
		case 7:
			operand = srl(f, operand);
			break;
		default:
			UNREACHABLE;
		}
		adjustSZP<Z80>(f, operand);
		if (LIKELY(!m_displaced)) {
			R(z, a, operand);
			if (UNLIKELY(z == 6))
				addCycles(7);
		} else {
			if (LIKELY(z != 6))
				R2(z, a, operand);
			setByte(operand);
			addCycles(15);
		}
		addCycles(8);
		break;
	} case 1:	// BIT y, r[z]
		addCycles(8);
		if (LIKELY(!m_displaced)) {
			const auto operand = bit(f, y, R(z, a));
			if (UNLIKELY(z == 6)) {
				adjustXY<Z80>(f, MEMPTR().high);
				addCycles(4);
			} else {
				adjustXY<Z80>(f, operand);
			}
		} else {
			bit(f, y, getByte(displacedAddress()));
			adjustXY<Z80>(f, MEMPTR().high);
			addCycles(12);
		}
		break;
	case 2:	// RES y, r[z]
		addCycles(8);
		if (LIKELY(!m_displaced)) {
			R(z, a, res(y, R(z, a)));
			if (UNLIKELY(z == 6))
				addCycles(7);
		} else {
			auto operand = getByte(displacedAddress());
			operand = res(y, operand);
			setByte(operand);
			R2(z, a, operand);
			addCycles(15);
		}
		break;
	case 3:	// SET y, r[z]
		addCycles(8);
		if (LIKELY(!m_displaced)) {
			R(z, a, set(y, R(z, a)));
			if (UNLIKELY(z == 6))
				addCycles(7);
		} else {
			auto operand = getByte(displacedAddress());
			operand = set(y, operand);
			setByte(operand);
			R2(z, a, operand);
			addCycles(15);
		}
		break;
	default:
		UNREACHABLE;
	}
}

void EightBit::Z80::executeED(const int x, const int y, const int z, const int p, const int q) {
	auto& a = A();
	auto& f = F();
	switch (x) {
	case 0:
	case 3:	// Invalid instruction, equivalent to NONI followed by NOP
		addCycles(8);
		break;
	case 1:
		switch (z) {
		case 0: // Input from port with 16-bit address
			MEMPTR() = BUS().ADDRESS() = BC();
			MEMPTR().word++;
			readPort();
			if (LIKELY(y != 6))	// IN r[y],(C)
				R(y, a, BUS().DATA());
			adjustSZPXY<Z80>(f, BUS().DATA());
			clearFlag(f, NF | HC);
			addCycles(12);
			break;
		case 1:	// Output to port with 16-bit address
			MEMPTR() = BUS().ADDRESS() = BC();
			MEMPTR().word++;
			if (UNLIKELY(y == 6))	// OUT (C),0
				BUS().placeDATA(0);
			else		// OUT (C),r[y]
				BUS().placeDATA(R(y, a));
			writePort();
			addCycles(12);
			break;
		case 2:	// 16-bit add/subtract with carry
			switch (q) {
			case 0:	// SBC HL, rp[p]
				sbc(f, HL2(), RP(p));
				break;
			case 1:	// ADC HL, rp[p]
				adc(f, HL2(), RP(p));
				break;
			default:
				UNREACHABLE;
			}
			addCycles(15);
			break;
		case 3:	// Retrieve/store register pair from/to immediate address
			switch (q) {
			case 0:	// LD (nn), rp[p]
				fetchWord();
				setWordViaMemptr(RP(p));
				break;
			case 1:	// LD rp[p], (nn)
				fetchWord();
				getWordViaMemptr(RP(p));
				break;
			default:
				UNREACHABLE;
			}
			addCycles(20);
			break;
		case 4:	// Negate accumulator
			neg(a, f);
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
				IV() = a;
				addCycles(9);
				break;
			case 1:	// LD R,A
				REFRESH() = a;
				addCycles(9);
				break;
			case 2:	// LD A,I
				a = IV();
				adjustSZXY<Z80>(f, a);
				clearFlag(f, NF | HC);
				setFlag(f, PF, IFF2());
				addCycles(9);
				break;
			case 3:	// LD A,R
				a = REFRESH();
				adjustSZXY<Z80>(f, a);
				clearFlag(f, NF | HC);
				setFlag(f, PF, IFF2());
				addCycles(9);
				break;
			case 4:	// RRD
				rrd(a, f);
				addCycles(18);
				break;
			case 5:	// RLD
				rld(a, f);
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
				ldi(a, f);
				break;
			case 5:	// LDD
				ldd(a, f);
				break;
			case 6:	// LDIR
				if (LIKELY(ldir(a, f))) {
					PC().word -= 2;
					addCycles(5);
				}
				break;
			case 7:	// LDDR
				if (LIKELY(lddr(a, f))) {
					PC().word -= 2;
					addCycles(5);
				}
				break;
			}
			break;
		case 1:	// CP
			switch (y) {
			case 4:	// CPI
				cpi(a, f);
				break;
			case 5:	// CPD
				cpd(a, f);
				break;
			case 6:	// CPIR
				if (LIKELY(cpir(a, f))) {
					PC().word -= 2;
					addCycles(5);
				}
				break;
			case 7:	// CPDR
				if (LIKELY(cpdr(a, f))) {
					PC().word -= 2;
					addCycles(5);
				}
				break;
			}
			break;
		case 2:	// IN
			switch (y) {
			case 4:	// INI
				ini(f);
				break;
			case 5:	// IND
				ind(f);
				break;
			case 6:	// INIR
				if (LIKELY(inir(f))) {
					PC().word -= 2;
					addCycles(5);
				}
				break;
			case 7:	// INDR
				if (LIKELY(indr(f))) {
					PC().word -= 2;
					addCycles(5);
				}
				break;
			}
			break;
		case 3:	// OUT
			switch (y) {
			case 4:	// OUTI
				outi(f);
				break;
			case 5:	// OUTD
				outd(f);
				break;
			case 6:	// OTIR
				if (LIKELY(otir(f))) {
					PC().word -= 2;
					addCycles(5);
				}
				break;
			case 7:	// OTDR
				if (LIKELY(otdr(f))) {
					PC().word -= 2;
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
	auto& a = A();
	auto& f = F();
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
				if (jrConditionalFlag(f, y - 4))
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
				fetchWord(RP(p));
				addCycles(10);
				break;
			case 1:	// ADD HL,rp
				add(f, HL2(), RP(p));
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
					memptrReference();
					setByte(MEMPTR().high = a);
					addCycles(7);
					break;
				case 1:	// LD (DE),A
					MEMPTR() = DE();
					memptrReference();
					setByte(MEMPTR().high = a);
					addCycles(7);
					break;
				case 2:	// LD (nn),HL
					fetchWord();
					setWordViaMemptr(HL2());
					addCycles(16);
					break;
				case 3: // LD (nn),A
					fetchWord();
					memptrReference();
					setByte(MEMPTR().high = a);
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
					memptrReference();
					a = getByte();
					addCycles(7);
					break;
				case 1:	// LD A,(DE)
					MEMPTR() = DE();
					memptrReference();
					a = getByte();
					addCycles(7);
					break;
				case 2:	// LD HL,(nn)
					fetchWord();
					getWordViaMemptr(HL2());
					addCycles(16);
					break;
				case 3:	// LD A,(nn)
					fetchWord();
					memptrReference();
					a = getByte();
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
			if (UNLIKELY(m_displaced && (y == 6)))
				fetchDisplacement();
			auto operand = R(y, a);
			increment(f, operand);
			R(y, a, operand);
			addCycles(4);
			break;
		} case 5: {	// 8-bit DEC
			if (UNLIKELY(m_displaced && (y == 6)))
				fetchDisplacement();
			auto operand = R(y, a);
			decrement(f, operand);
			R(y, a, operand);
			addCycles(4);
			if (UNLIKELY(y == 6))
				addCycles(7);
			break;
		} case 6:	// 8-bit load immediate
			if (UNLIKELY(m_displaced && (y == 6)))
				fetchDisplacement();
			R(y, a, fetchByte());	// LD r,n
			addCycles(7);
			if (UNLIKELY(y == 6))
				addCycles(3);
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
						H() = R(z, a);
						normal = false;
						break;
					case 5:
						L() = R(z, a);
						normal = false;
						break;
					}
				}
				if (UNLIKELY(y == 6)) {
					fetchDisplacement();
					switch (z) {
					case 4:
						R(y, a, H());
						normal = false;
						break;
					case 5:
						R(y, a, L());
						normal = false;
						break;
					}
				}
			}
			if (LIKELY(normal))
				R(y, a, R(z, a));
			if (UNLIKELY((y == 6) || (z == 6)))	// M operations
				addCycles(3);
		}
		addCycles(4);
		break;
	case 2:	// Operate on accumulator and register/memory location
		if (UNLIKELY(m_displaced && (z == 6)))
			fetchDisplacement();
		switch (y) {
		case 0:	// ADD A,r
			add(f, a, R(z, a));
			break;
		case 1:	// ADC A,r
			adc(f, a, R(z, a));
			break;
		case 2:	// SUB r
			sub(f, a, R(z, a));
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
				case 1:	// EXX
					exx();
					addCycles(4);
					break;
				case 2:	// JP HL
					PC() = HL2();
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
			case 1:	// CB prefix
				m_prefixCB = true;
				if (UNLIKELY(m_displaced))
					fetchDisplacement();
				M1() = true;
				execute(fetchByte());
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
				xhtl(HL2());
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
				case 1:	// DD prefix
					m_displaced = m_prefixDD = true;
					M1() = true;
					execute(fetchByte());
					break;
				case 2:	// ED prefix
					m_prefixED = true;
					M1() = true;
					execute(fetchByte());
					break;
				case 3:	// FD prefix
					m_displaced = m_prefixFD = true;
					M1() = true;
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
		case 6:	// Operate on accumulator and immediate operand: alu[y] n
			switch (y) {
			case 0:	// ADD A,n
				add(f, a, fetchByte());
				break;
			case 1:	// ADC A,n
				adc(f, a, fetchByte());
				break;
			case 2:	// SUB n
				sub(f, a, fetchByte());
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

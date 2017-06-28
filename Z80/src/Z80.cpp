#include "stdafx.h"
#include "Z80.h"

// based on http://www.z80.info/decoding.htm
// Half carry flag help from https://github.com/oubiwann/z80

EightBit::Z80::Z80(Memory& memory, InputOutput& ports)
: IntelProcessor(memory),
  m_ports(ports),
  m_registerSet(0),
  m_accumulatorFlagsSet(0),
  m_refresh(0x7f),
  iv(0xff),
  m_interruptMode(0),
  m_iff1(false),
  m_iff2(false),
  m1(false),
  m_prefixCB(false),
  m_prefixDD(false),
  m_prefixED(false),
  m_prefixFD(false),
  m_displacement(0),
  m_displaced(false) {
	IX().word = 0xffff;
	IY().word = 0xffff;
}

void EightBit::Z80::reset() {
	IntelProcessor::reset();
	di();
}

void EightBit::Z80::initialise() {

	IntelProcessor::initialise();

	for (int i = 0; i < 0x100; ++i) {
		m_decodedOpcodes[i] = i;
	}

	IM() = 0;

	AF().word = 0xffff;

	BC().word = 0xffff;
	DE().word = 0xffff;
	HL().word = 0xffff;

	exxAF();
	exx();

	AF().word = 0xffff;

	BC().word = 0xffff;
	DE().word = 0xffff;
	HL().word = 0xffff;

	IX().word = 0xffff;
	IY().word = 0xffff;

	REFRESH() = 0x7f;
	IV() = 0xff;

	m_prefixCB = false;
	m_prefixDD = false;
	m_prefixED = false;
	m_prefixFD = false;
}

#pragma region Interrupt routines

void EightBit::Z80::di() {
	IFF1() = IFF2() = false;
}

void EightBit::Z80::ei() {
	IFF1() = IFF2() = true;
}

int EightBit::Z80::interrupt(bool maskable, uint8_t value) {
	cycles = 0;
	if (!maskable || (maskable && IFF1())) {
		if (maskable) {
			di();
			switch (IM()) {
			case 0:
				M1() = true;
				cycles += execute(value);
				break;
			case 1:
				restart(7 << 3);
				cycles += 13;
				break;
			case 2:
				pushWord(PC());
				PC().low = value;
				PC().high = IV();
				cycles += 19;
				break;
			}
		} else {
			IFF1() = false;
			restart(0x66);
			cycles += 13;
		}
	}
	// Could be zero for a masked interrupt...
	return cycles;
}

#pragma endregion Interrupt routines

#pragma region Flag manipulation helpers

void EightBit::Z80::postIncrement(uint8_t& f, uint8_t value) {
	adjustSZXY<Z80>(f, value);
	clearFlag(f, NF);
	setFlag(f, VF, value == Bit7);
	clearFlag(f, HC, lowNibble(value));
}

void EightBit::Z80::postDecrement(uint8_t& f, uint8_t value) {
	adjustSZXY<Z80>(f, value);
	setFlag(f, NF);
	setFlag(f, VF, value == Mask7);
	clearFlag(f, HC, lowNibble(value + 1));
}

#pragma endregion Flag manipulation helpers

#pragma region PC manipulation: call/ret/jp/jr

bool EightBit::Z80::jrConditionalFlag(uint8_t& f, int flag) {
	switch (flag) {
	case 0:	// NZ
		return jrConditional(!(f & ZF));
	case 1:	// Z
		return jrConditional(f & ZF);
	case 2:	// NC
		return jrConditional(!(f & CF));
	case 3:	// C
		return jrConditional(f & CF);
	case 4:	// PO
		return jrConditional(!(f & PF));
	case 5:	// PE
		return jrConditional(f & PF);
	case 6:	// P
		return jrConditional(!(f & SF));
	case 7:	// M
		return jrConditional(f & SF);
	}
	throw std::logic_error("Unhandled JR conditional");
}

bool EightBit::Z80::jumpConditionalFlag(uint8_t& f, int flag) {
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

bool EightBit::Z80::returnConditionalFlag(uint8_t& f, int flag) {
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
	}
	throw std::logic_error("Unhandled RET conditional");
}

bool EightBit::Z80::callConditionalFlag(uint8_t& f, int flag) {
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
	}
	throw std::logic_error("Unhandled CALL conditional");
}

#pragma endregion PC manipulation: call/ret/jp/jr

#pragma region 16-bit arithmetic

void EightBit::Z80::sbc(uint8_t& f, register16_t& operand, register16_t value) {

	const auto before = operand;

	const auto beforeNegative = before.high & SF;
	const auto valueNegative = value.high & SF;

	const auto result = before.word - value.word - (f & CF);
	operand.word = result;

	const auto afterNegative = operand.high & SF;

	setFlag(f, SF, afterNegative);
	clearFlag(f, ZF, operand.word);
	adjustHalfCarrySub(f, before.high, value.high, operand.high);
	adjustOverflowSub(f, beforeNegative, valueNegative, afterNegative);
	setFlag(f, NF);
	setFlag(f, CF, result & Bit16);
	adjustXY<Z80>(f, operand.high);
}

void EightBit::Z80::adc(uint8_t& f, register16_t& operand, register16_t value) {

	const auto before = operand;

	const auto beforeNegative = before.high & SF;
	const auto valueNegative = value.high & SF;

	const auto result = before.word + value.word + (f & CF);
	operand.word = result;

	auto afterNegative = operand.high & SF;

	setFlag(f, SF, afterNegative);
	clearFlag(f, ZF, operand.word);
	adjustHalfCarryAdd(f, before.high, value.high, operand.high);
	adjustOverflowAdd(f, beforeNegative, valueNegative, afterNegative);
	clearFlag(f, NF);
	setFlag(f, CF, result & Bit16);
	adjustXY<Z80>(f, operand.high);
}

void EightBit::Z80::add(uint8_t& f, register16_t& operand, register16_t value) {

	const auto before = operand;

	const auto result = before.word + value.word;

	operand.word = result;

	clearFlag(f, NF);
	setFlag(f, CF, result & Bit16);
	adjustHalfCarryAdd(f, before.high, value.high, operand.high);
	adjustXY<Z80>(f, operand.high);
}

#pragma endregion 16-bit arithmetic

#pragma region ALU

void EightBit::Z80::add(uint8_t& f, uint8_t& operand, uint8_t value, int carry) {

	register16_t result;
	result.word = operand + value + carry;

	adjustHalfCarryAdd(f, operand, value, result.low);
	adjustOverflowAdd(f, operand, value, result.low);

	operand = result.low;

	clearFlag(f, NF);
	setFlag(f, CF, result.word & Bit8);
	adjustSZXY<Z80>(f, operand);
}

void EightBit::Z80::adc(uint8_t& f, uint8_t& operand, uint8_t value) {
	add(f, operand, value, f & CF);
}

void EightBit::Z80::subtract(uint8_t& f, uint8_t& operand, uint8_t value, int carry) {

	register16_t result;
	result.word = operand - value - carry;

	adjustHalfCarrySub(f, operand, value, result.low);
	adjustOverflowSub(f, operand, value, result.low);

	operand = result.low;

	setFlag(f, NF);
	setFlag(f, CF, result.word & Bit8);
	adjustSZ<Z80>(f, operand);
}

void EightBit::Z80::sub(uint8_t& f, uint8_t& operand, uint8_t value, int carry) {
	subtract(f, operand, value, carry);
	adjustXY<Z80>(f, operand);
}

void EightBit::Z80::sbc(uint8_t& f, uint8_t& operand, uint8_t value) {
	sub(f, operand, value, f & CF);
}

void EightBit::Z80::andr(uint8_t& f, uint8_t& operand, uint8_t value) {
	operand &= value;
	setFlag(f, HC);
	clearFlag(f, CF | NF);
	adjustSZPXY<Z80>(f, operand);
}

void EightBit::Z80::xorr(uint8_t& f, uint8_t& operand, uint8_t value) {
	operand ^= value;
	clearFlag(f, HC | CF | NF);
	adjustSZPXY<Z80>(f, operand);
}

void EightBit::Z80::orr(uint8_t& f, uint8_t& operand, uint8_t value) {
	operand |= value;
	clearFlag(f, HC | CF | NF);
	adjustSZPXY<Z80>(f, operand);
}

void EightBit::Z80::compare(uint8_t& f, uint8_t check, uint8_t value) {
	subtract(f, check, value);
	adjustXY<Z80>(f, value);
}

#pragma endregion ALU

#pragma region Shift and rotate

uint8_t& EightBit::Z80::rlc(uint8_t& f, uint8_t& operand) {
	const auto carry = operand & Bit7;
	operand = _rotl8(operand, 1);
	setFlag(f, CF, carry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t& EightBit::Z80::rrc(uint8_t& f, uint8_t& operand) {
	const auto carry = operand & Bit0;
	operand = _rotr8(operand, 1);
	setFlag(f, CF, carry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t& EightBit::Z80::rl(uint8_t& f, uint8_t& operand) {
	const auto oldCarry = f & CF;
	const auto newCarry = operand & Bit7;
	operand <<= 1;
	oldCarry ? operand |= Bit0 : operand &= ~Bit0;
	setFlag(f, CF, newCarry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t& EightBit::Z80::rr(uint8_t& f, uint8_t& operand) {
	const auto oldCarry = f & CF;
	const auto newCarry = operand & Bit0;
	operand >>= 1;
	operand |= oldCarry << 7;
	setFlag(f, CF, newCarry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	return operand;
}

//

uint8_t& EightBit::Z80::sla(uint8_t& f, uint8_t& operand) {
	const auto newCarry = operand & Bit7;
	operand <<= 1;
	setFlag(f, CF, newCarry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t& EightBit::Z80::sra(uint8_t& f, uint8_t& operand) {
	const auto new7 = operand & Bit7;
	const auto newCarry = operand & Bit0;
	operand >>= 1;
	operand |= new7;
	setFlag(f, CF, newCarry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t& EightBit::Z80::sll(uint8_t& f, uint8_t& operand) {
	const auto newCarry = operand & Bit7;
	operand <<= 1;
	operand |= 1;
	setFlag(f, CF, newCarry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t& EightBit::Z80::srl(uint8_t& f, uint8_t& operand) {
	const auto newCarry = operand & Bit0;
	operand >>= 1;
	operand &= ~Bit7;	// clear bit 7
	setFlag(f, CF, newCarry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	setFlag(f, ZF, operand);
	return operand;
}

#pragma endregion Shift and rotate

#pragma region BIT/SET/RES

uint8_t& EightBit::Z80::bit(uint8_t& f, int n, uint8_t& operand) {
	const uint8_t discarded = operand & (1 << n);
	setFlag(f, HC);
	clearFlag(f, NF);
	adjustSZXY<Z80>(f, discarded);
	clearFlag(f, PF, discarded);
	return operand;
}

uint8_t& EightBit::Z80::res(int n, uint8_t& operand) {
	operand &= ~(1 << n);
	return operand;
}

uint8_t& EightBit::Z80::set(int n, uint8_t& operand) {
	operand |= (1 << n);
	return operand;
}

#pragma endregion BIT/SET/RES

#pragma region Miscellaneous instructions

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

	auto lowAdjust = (f & HC) | (lowNibble(a) > 9);
	auto highAdjust = (f & CF) | (a > 0x99);

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

	f = (f & (CF | NF)) | (a > 0x99) | ((a ^ updated) & HC);
	a = updated;

	adjustSZPXY<Z80>(f, a);
}

void EightBit::Z80::cpl(uint8_t& a, uint8_t& f) {
	a = ~a;
	adjustXY<Z80>(f, a);
	setFlag(f, HC | NF);
}

void EightBit::Z80::scf(uint8_t a, uint8_t& f) {
	setFlag(f, CF);
	adjustXY<Z80>(f, a);
	clearFlag(f, HC | NF);
}

void EightBit::Z80::ccf(uint8_t a, uint8_t& f) {
	const auto carry = f & CF;
	setFlag(f, HC, carry);
	clearFlag(f, CF, carry);
	clearFlag(f, NF);
	adjustXY<Z80>(f, a);
}

void EightBit::Z80::xhtl(register16_t& operand) {
	m_memory.ADDRESS() = SP();
	MEMPTR().low = m_memory.reference();
	m_memory.reference() = operand.low;
	operand.low = MEMPTR().low;
	m_memory.ADDRESS().word++;
	MEMPTR().high = m_memory.reference();
	m_memory.reference() = operand.high;
	operand.high = MEMPTR().high;
}

#pragma endregion Miscellaneous instructions

#pragma region Block instructions

#pragma region Block compare instructions

void EightBit::Z80::blockCompare(uint8_t a, uint8_t& f) {

	m_memory.ADDRESS() = HL();

	const auto value = m_memory.reference();
	uint8_t result = a - value;

	setFlag(f, PF, --BC().word);

	adjustSZ<Z80>(f, result);
	adjustHalfCarrySub(f, a, value, result);
	setFlag(f, NF);

	if (f & HC)
		result -= 1;

	setFlag(f, YF, result & Bit1);
	setFlag(f, XF, result & Bit3);
}

void EightBit::Z80::cpi(uint8_t a, uint8_t& f) {
	blockCompare(a, f);
	HL().word++;
	MEMPTR().word++;
}

void EightBit::Z80::cpd(uint8_t a, uint8_t& f) {
	blockCompare(a, f);
	HL().word--;
	MEMPTR().word--;
}

bool EightBit::Z80::cpir(uint8_t a, uint8_t& f) {
	cpi(a, f);
	MEMPTR() = PC();
	auto again = (f & PF) && !(f & ZF);	// See CPI
	if (again)
		MEMPTR().word--;
	return again;
}

bool EightBit::Z80::cpdr(uint8_t a, uint8_t& f) {
	cpd(a, f);
	MEMPTR().word = PC().word - 1;
	auto again = (f & PF) && !(f & ZF);	// See CPD
	if (!again)
		MEMPTR().word--;
	return again;
}

#pragma endregion Block compare instructions

#pragma region Block load instructions

void EightBit::Z80::blockLoad(uint8_t a, uint8_t& f, register16_t source, register16_t destination) {
	m_memory.ADDRESS() = source;
	auto value = m_memory.reference();
	m_memory.ADDRESS() = destination;
	m_memory.reference() = value;
	auto xy = a + value;
	setFlag(f, XF, xy & 8);
	setFlag(f, YF, xy & 2);
	clearFlag(f, NF | HC);
	setFlag(f, PF, --BC().word);
}

void EightBit::Z80::ldd(uint8_t a, uint8_t& f) {
	blockLoad(a, f, HL(), DE());
	HL().word--;
	DE().word--;
}

void EightBit::Z80::ldi(uint8_t a, uint8_t& f) {
	blockLoad(a, f, HL(), DE());
	HL().word++;
	DE().word++;
}

bool EightBit::Z80::ldir(uint8_t a, uint8_t& f) {
	ldi(a, f);
	auto again = (f & PF) != 0;
	if (again)		// See LDI
		MEMPTR().word = PC().word - 1;
	return again;
}

bool EightBit::Z80::lddr(uint8_t a, uint8_t& f) {
	ldd(a, f);
	auto again = (f & PF) != 0;
	if (again)		// See LDR
		MEMPTR().word = PC().word - 1;
	return again;
}

#pragma endregion Block load instructions

#pragma region Block input instructions

void EightBit::Z80::ini(uint8_t& f) {
	MEMPTR() = m_memory.ADDRESS() = BC();
	MEMPTR().word++;
	readPort();
	auto value = m_memory.DATA();
	m_memory.ADDRESS().word = HL().word++;
	m_memory.reference() = value;
	postDecrement(f, --B());
	setFlag(f, NF);
}

void EightBit::Z80::ind(uint8_t& f) {
	MEMPTR() = m_memory.ADDRESS() = BC();
	MEMPTR().word--;
	readPort();
	auto value = m_memory.DATA();
	m_memory.ADDRESS().word = HL().word--;
	m_memory.reference() = value;
	postDecrement(f, --B());
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

#pragma endregion Block input instructions

#pragma region Block output instructions

void EightBit::Z80::blockOut(uint8_t& f) {
	auto value = m_memory.reference();
	m_memory.ADDRESS().word = BC().word;
	writePort();
	postDecrement(f, --B());
	setFlag(f, NF, value & Bit7);
	setFlag(f, HC | CF, (L() + value) > 0xff);
	adjustParity<Z80>(f, ((value + L()) & 7) ^ B());
}

void EightBit::Z80::outi(uint8_t& f) {
	m_memory.ADDRESS().word = HL().word++;
	blockOut(f);
	MEMPTR().word = BC().word + 1;
}

void EightBit::Z80::outd(uint8_t& f) {
	m_memory.ADDRESS().word = HL().word--;
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

#pragma endregion Block output instructions

#pragma endregion Block instructions

#pragma region Nibble rotation

void EightBit::Z80::rrd(uint8_t& a, uint8_t& f) {
	MEMPTR() = HL();
	auto memory = memptrReference();
	m_memory.reference() = promoteNibble(a) | highNibble(memory);
	a = (a & 0xf0) | lowNibble(memory);
	adjustSZPXY<Z80>(f, a);
	clearFlag(f, NF | HC);
}

void EightBit::Z80::rld(uint8_t& a, uint8_t& f) {
	MEMPTR() = HL();
	auto memory = memptrReference();
	m_memory.reference() = promoteNibble(memory) | lowNibble(a);
	a = (a & 0xf0) | highNibble(memory);
	adjustSZPXY<Z80>(f, a);
	clearFlag(f, NF | HC);
}

#pragma endregion Nibble rotation

#pragma region I/O instructions

void EightBit::Z80::writePort(uint8_t port, uint8_t data) {
	m_memory.ADDRESS().low = port;
	m_memory.ADDRESS().high = data;
	MEMPTR() = m_memory.ADDRESS();
	m_memory.placeDATA(data);
	writePort();
	MEMPTR().low++;
}

void EightBit::Z80::writePort() {
	m_ports.write(m_memory.ADDRESS().low, m_memory.DATA());
}

void EightBit::Z80::readPort(uint8_t port, uint8_t& a) {
	m_memory.ADDRESS().low = port;
	m_memory.ADDRESS().high = a;
	MEMPTR() = m_memory.ADDRESS();
	readPort();
	a = m_memory.DATA();
	MEMPTR().low++;
}

void EightBit::Z80::readPort() {
	m_memory.placeDATA(m_ports.read(m_memory.ADDRESS().low));
}

#pragma endregion I/O instructions

int EightBit::Z80::step() {
	ExecutingInstruction.fire(*this);
	m_displaced = m_prefixCB = m_prefixDD = m_prefixED = m_prefixFD = false;
	cycles = 0;
	return fetchExecute();
}

int EightBit::Z80::execute(uint8_t opcode) {

	if (!M1())
		throw std::logic_error("M1 cannot be high");

	const auto& decoded = m_decodedOpcodes[opcode];

	auto x = decoded.x;
	auto y = decoded.y;
	auto z = decoded.z;

	auto p = decoded.p;
	auto q = decoded.q;

	if (!(m_prefixCB && m_displaced)) {
		++REFRESH();
		M1() = false;
	}

	if (m_prefixCB)
		executeCB(x, y, z);
	else if (m_prefixED)
		executeED(x, y, z, p, q);
	else
		executeOther(x, y, z, p, q);

	if (cycles == 0)
		throw std::logic_error("Unhandled opcode");

	return cycles;
}

void EightBit::Z80::executeCB(int x, int y, int z) {
	auto& a = A();
	auto& f = F();
	switch (x) {
	case 0:	// rot[y] r[z]
		switch (y) {
		case 0:
			adjustSZP<Z80>(f, m_displaced ? R2(z, a) = rlc(f, DISPLACED()) : rlc(f, R(z, a)));
			break;
		case 1:
			adjustSZP<Z80>(f, m_displaced ? R2(z, a) = rrc(f, DISPLACED()) : rrc(f, R(z, a)));
			break;
		case 2:
			adjustSZP<Z80>(f, m_displaced ? R2(z, a) = rl(f, DISPLACED()) : rl(f, R(z, a)));
			break;
		case 3:
			adjustSZP<Z80>(f, m_displaced ? R2(z, a) = rr(f, DISPLACED()) : rr(f, R(z, a)));
			break;
		case 4:
			adjustSZP<Z80>(f, m_displaced ? R2(z, a) = sla(f, DISPLACED()) : sla(f, R(z, a)));
			break;
		case 5:
			adjustSZP<Z80>(f, m_displaced ? R2(z, a) = sra(f, DISPLACED()) : sra(f, R(z, a)));
			break;
		case 6:
			adjustSZP<Z80>(f, m_displaced ? R2(z, a) = sll(f, DISPLACED()) : sll(f, R(z, a)));
			break;
		case 7:
			adjustSZP<Z80>(f, m_displaced ? R2(z, a) = srl(f, DISPLACED()) : srl(f, R(z, a)));
			break;
		}
		if (m_displaced) {
			cycles += 23;
		} else {
			cycles += 8;
			if (z == 6)
				cycles += 7;
		}
		break;
	case 1:	// BIT y, r[z]
		if (m_displaced) {
			bit(f, y, DISPLACED());
			adjustXY<Z80>(f, MEMPTR().high);
			cycles += 20;
		} else {
			auto operand = bit(f, y, R(z, a));
			cycles += 8;
			if (z == 6) {
				adjustXY<Z80>(f, MEMPTR().high);
				cycles += 4;
			} else {
				adjustXY<Z80>(f, operand);
			}
		}
		break;
	case 2:	// RES y, r[z]
		if (m_displaced) {
			R2(z, a) = res(y, DISPLACED());
			cycles += 23;
		} else {
			res(y, R(z, a));
			cycles += 8;
			if (z == 6)
				cycles += 7;
		}
		break;
	case 3:	// SET y, r[z]
		if (m_displaced) {
			R2(z, a) = set(y, DISPLACED());
			cycles += 23;
		} else {
			set(y, R(z, a));
			cycles += 8;
			if (z == 6)
				cycles += 7;
		}
		break;
	}
}

void EightBit::Z80::executeED(int x, int y, int z, int p, int q) {
	auto& a = A();
	auto& f = F();
	switch (x) {
	case 0:
	case 3:	// Invalid instruction, equivalent to NONI followed by NOP
		cycles += 8;
		break;
	case 1:
		switch (z) {
		case 0: // Input from port with 16-bit address
			MEMPTR() = m_memory.ADDRESS() = BC();
			MEMPTR().word++;
			readPort();
			if (y != 6)	// IN r[y],(C)
				R(y, a) = m_memory.DATA();
			adjustSZPXY<Z80>(f, m_memory.DATA());
			clearFlag(f, NF | HC);
			cycles += 12;
			break;
		case 1:	// Output to port with 16-bit address
			MEMPTR() = m_memory.ADDRESS() = BC();
			MEMPTR().word++;
			if (y == 6)	// OUT (C),0
				m_memory.placeDATA(0);
			else		// OUT (C),r[y]
				m_memory.placeDATA(R(y, a));
			writePort();
			cycles += 12;
			break;
		case 2:	// 16-bit add/subtract with carry
			switch (q) {
			case 0:	// SBC HL, rp[p]
				sbcViaMemptr(f, HL2(), RP(p));
				break;
			case 1:	// ADC HL, rp[p]
				adcViaMemptr(f, HL2(), RP(p));
				break;
			default:
				__assume(0);
			}
			cycles += 15;
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
				__assume(0);
			}
			cycles += 20;
			break;
		case 4:	// Negate accumulator
			neg(a, f);
			cycles += 8;
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
			cycles += 14;
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
			default:
				__assume(0);
			}
			cycles += 8;
			break;
		case 7:	// Assorted ops
			switch (y) {
			case 0:	// LD I,A
				IV() = a;
				cycles += 9;
				break;
			case 1:	// LD R,A
				REFRESH() = a;
				cycles += 9;
				break;
			case 2:	// LD A,I
				a = IV();
				adjustSZXY<Z80>(f, a);
				clearFlag(f, NF | HC);
				setFlag(f, PF, IFF2());
				cycles += 9;
				break;
			case 3:	// LD A,R
				a = REFRESH();
				adjustSZXY<Z80>(f, a);
				clearFlag(f, NF | HC);
				setFlag(f, PF, IFF2());
				cycles += 9;
				break;
			case 4:	// RRD
				rrd(a, f);
				cycles += 18;
				break;
			case 5:	// RLD
				rld(a, f);
				cycles += 18;
				break;
			case 6:	// NOP
			case 7:	// NOP
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
				if (ldir(a, f)) {
					PC().word -= 2;
					cycles += 5;
				}
				break;
			case 7:	// LDDR
				if (lddr(a, f)) {
					PC().word -= 2;
					cycles += 5;
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
				if (cpir(a, f)) {
					PC().word -= 2;
					cycles += 5;
				}
				break;
			case 7:	// CPDR
				if (cpdr(a, f)) {
					PC().word -= 2;
					cycles += 5;
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
				if (inir(f)) {
					PC().word -= 2;
					cycles += 5;
				}
				break;
			case 7:	// INDR
				if (indr(f)) {
					PC().word -= 2;
					cycles += 5;
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
				if (otir(f)) {
					PC().word -= 2;
					cycles += 5;
				}
				break;
			case 7:	// OTDR
				if (otdr(f)) {
					PC().word -= 2;
					cycles += 5;
				}
				break;
			}
			break;
		}
		cycles += 16;
		break;
	}
}

void EightBit::Z80::executeOther(int x, int y, int z, int p, int q) {
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
				exxAF();
				cycles += 4;
				break;
			case 2:	// DJNZ d
				if (jrConditional(--B()))
					cycles += 5;
				cycles += 8;
				break;
			case 3:	// JR d
				jr(fetchByte());
				cycles += 12;
				break;
			default:	// JR cc,d
				if (jrConditionalFlag(f, y - 4))
					cycles += 5;
				cycles += 5;
				break;
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0: // LD rp,nn
				Processor::fetchWord(RP(p));
				cycles += 10;
				break;
			case 1:	// ADD HL,rp
				addViaMemptr(f, HL2(), RP(p));
				cycles += 11;
				break;
			default:
				__assume(0);
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
					setWordViaMemptr(HL2());
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
					getWordViaMemptr(HL2());
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
			postIncrement(f, ++R(y, a));	// INC r
			cycles += 4;
			break;
		case 5:	// 8-bit DEC
			postDecrement(f, --R(y, a));	// DEC r
			cycles += 4;
			if (y == 6)
				cycles += 7;
			break;
		case 6: { // 8-bit load immediate
			auto& r = R(y, a);	// LD r,n
			r = fetchByte();
			cycles += 7;
			if (y == 6)
				cycles += 3;
			break;
		} case 7:	// Assorted operations on accumulator/flags
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
				cpl(a, f);
				break;
			case 6:
				scf(a, f);
				break;
			case 7:
				ccf(a, f);
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
			bool normal = true;
			if (m_displaced) {
				if (z == 6) {
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
				if (y == 6) {
					switch (z) {
					case 4:
						R(y, a) = H();
						normal = false;
						break;
					case 5:
						R(y, a) = L();
						normal = false;
						break;
					}
				}
			}
			if (normal)
				R(y, a) = R(z, a);
			if ((y == 6) || (z == 6))	// M operations
				cycles += 3;
		}
		cycles += 4;
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
					exx();
					cycles += 4;
					break;
				case 2:	// JP HL
					PC() = HL2();
					cycles += 4;
					break;
				case 3:	// LD SP,HL
					SP() = HL2();
					cycles += 4;
					break;
				default:
					__assume(0);
				}
			////default:
			////	__assume(0);
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
				m_prefixCB = true;
				if (m_displaced)
					m_displacement = fetchByte();
				fetchExecute();
				break;
			case 2:	// OUT (n),A
				writePort(fetchByte(), a);
				cycles += 11;
				break;
			case 3:	// IN A,(n)
				readPort(fetchByte(), a);
				cycles += 11;
				break;
			case 4:	// EX (SP),HL
				xhtl(HL2());
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
					m_displaced = m_prefixDD = true;
					fetchExecute();
					break;
				case 2:	// ED prefix
					m_prefixED = true;
					fetchExecute();
					break;
				case 3:	// FD prefix
					m_displaced = m_prefixFD = true;
					fetchExecute();
					break;
				default:
					__assume(0);
				}
			////default:
			////	__assume(0);
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
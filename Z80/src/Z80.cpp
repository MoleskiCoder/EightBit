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
  m_prefixFD(false) {
	IX().word = 0xffff;
	IY().word = 0xffff;
}

void EightBit::Z80::reset() {
	IntelProcessor::reset();
	di();
}

void EightBit::Z80::initialise() {

	IntelProcessor::initialise();

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

bool EightBit::Z80::jrConditionalFlag(int flag) {
	switch (flag) {
	case 0:	// NZ
		return jrConditional(!(F() & ZF));
	case 1:	// Z
		return jrConditional(F() & ZF);
	case 2:	// NC
		return jrConditional(!(F() & CF));
	case 3:	// C
		return jrConditional(F() & CF);
	case 4:	// PO
		return jrConditional(!(F() & PF));
	case 5:	// PE
		return jrConditional(F() & PF);
	case 6:	// P
		return jrConditional(!(F() & SF));
	case 7:	// M
		return jrConditional(F() & SF);
	}
	throw std::logic_error("Unhandled JR conditional");
}

bool EightBit::Z80::jumpConditionalFlag(int flag) {
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

bool EightBit::Z80::returnConditionalFlag(int flag) {
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
	}
	throw std::logic_error("Unhandled RET conditional");
}

bool EightBit::Z80::callConditionalFlag(int flag) {
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
	}
	throw std::logic_error("Unhandled CALL conditional");
}

#pragma endregion PC manipulation: call/ret/jp/jr

#pragma region 16-bit arithmetic

void EightBit::Z80::sbc(register16_t& operand, register16_t value) {

	auto& f = F();

	auto before = operand;

	auto beforeNegative = before.high & SF;
	auto valueNegative = value.high & SF;

	auto result = before.word - value.word - (f & CF);
	operand.word = result;

	auto afterNegative = operand.high & SF;

	setFlag(f, SF, afterNegative);
	clearFlag(f, ZF, operand.word);
	adjustHalfCarrySub(f, before.high, value.high, operand.high);
	adjustOverflowSub(f, beforeNegative, valueNegative, afterNegative);
	setFlag(f, NF);
	setFlag(f, CF, result & Bit16);
	adjustXY<Z80>(f, operand.high);
}

void EightBit::Z80::adc(register16_t& operand, register16_t value) {

	auto& f = F();

	auto before = operand;

	auto beforeNegative = before.high & SF;
	auto valueNegative = value.high & SF;

	auto result = before.word + value.word + (f & CF);
	operand.word = result;

	auto afterNegative = operand.high & SF;

	setFlag(f, SF, afterNegative);
	clearFlag(f, ZF, result);
	adjustHalfCarryAdd(f, before.high, value.high, operand.high);
	adjustOverflowAdd(f, beforeNegative, valueNegative, afterNegative);
	clearFlag(f, NF);
	setFlag(f, CF, result & Bit16);
	adjustXY<Z80>(f, operand.high);
}

void EightBit::Z80::add(register16_t& operand, register16_t value) {

	auto& f = F();

	auto before = operand;

	auto result = before.word + value.word;

	operand.word = result;

	clearFlag(f, NF);
	setFlag(f, CF, result & Bit16);
	adjustHalfCarryAdd(f, before.high, value.high, operand.high);
	adjustXY<Z80>(f, operand.high);
}

#pragma endregion 16-bit arithmetic

#pragma region ALU

void EightBit::Z80::add(uint8_t& operand, uint8_t value, int carry) {

	auto& f = F();

	register16_t result;
	result.word = operand + value + carry;

	adjustHalfCarryAdd(f, operand, value, result.low);
	adjustOverflowAdd(f, operand, value, result.low);

	operand = result.low;

	clearFlag(f, NF);
	setFlag(f, CF, result.word & Bit8);
	adjustSZXY<Z80>(f, operand);
}

void EightBit::Z80::adc(uint8_t& operand, uint8_t value) {
	add(operand, value, F() & CF);
}

void EightBit::Z80::sub(uint8_t& operand, uint8_t value, int carry) {

	auto& f = F();

	register16_t result;
	result.word = operand - value - carry;

	adjustHalfCarrySub(f, operand, value, result.low);
	adjustOverflowSub(f, operand, value, result.low);

	operand = result.low;

	setFlag(f, NF);
	setFlag(f, CF, result.word & Bit8);
	adjustSZXY<Z80>(f, operand);
}

void EightBit::Z80::sbc(uint8_t& operand, uint8_t value) {
	sub(operand, value, F() & CF);
}

void EightBit::Z80::andr(uint8_t& operand, uint8_t value) {
	auto& f = F();
	operand &= value;
	setFlag(f, HC);
	clearFlag(f, CF | NF);
	adjustSZPXY<Z80>(f, operand);
}

void EightBit::Z80::xorr(uint8_t& operand, uint8_t value) {
	auto& f = F();
	operand ^= value;
	clearFlag(f, HC | CF | NF);
	adjustSZPXY<Z80>(f, operand);
}

void EightBit::Z80::orr(uint8_t& operand, uint8_t value) {
	auto& f = F();
	operand |= value;
	clearFlag(f, HC | CF | NF);
	adjustSZPXY<Z80>(f, operand);
}

void EightBit::Z80::compare(uint8_t value) {
	auto check = A();
	sub(check, value);
	adjustXY<Z80>(F(), value);
}

#pragma endregion ALU

#pragma region Shift and rotate

uint8_t& EightBit::Z80::rlc(uint8_t& operand) {
	auto& f = F();
	auto carry = operand & Bit7;
	operand <<= 1;
	carry ? operand |= Bit0 : operand &= ~Bit0;
	setFlag(f, CF, carry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t& EightBit::Z80::rrc(uint8_t& operand) {
	auto& f = F();
	auto carry = operand & Bit0;
	operand >>= 1;
	carry ? operand |= Bit7 : operand &= ~Bit7;
	setFlag(f, CF, carry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t& EightBit::Z80::rl(uint8_t& operand) {
	auto& f = F();
	auto oldCarry = f & CF;
	auto newCarry = operand & Bit7;
	operand <<= 1;
	oldCarry ? operand |= Bit0 : operand &= ~Bit0;
	setFlag(f, CF, newCarry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t& EightBit::Z80::rr(uint8_t& operand) {
	auto& f = F();
	auto oldCarry = f & CF;
	auto newCarry = operand & Bit0;
	operand >>= 1;
	operand |= oldCarry << 7;
	setFlag(f, CF, newCarry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	return operand;
}

//

uint8_t& EightBit::Z80::sla(uint8_t& operand) {
	auto& f = F();
	auto newCarry = operand & Bit7;
	operand <<= 1;
	setFlag(f, CF, newCarry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t& EightBit::Z80::sra(uint8_t& operand) {
	auto& f = F();
	auto new7 = operand & Bit7;
	auto newCarry = operand & Bit0;
	operand >>= 1;
	operand |= new7;
	setFlag(f, CF, newCarry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t& EightBit::Z80::sll(uint8_t& operand) {
	auto& f = F();
	auto newCarry = operand & Bit7;
	operand <<= 1;
	operand |= 1;
	setFlag(f, CF, newCarry);
	clearFlag(f, NF | HC);
	adjustXY<Z80>(f, operand);
	return operand;
}

uint8_t& EightBit::Z80::srl(uint8_t& operand) {
	auto& f = F();
	auto newCarry = operand & Bit0;
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

uint8_t& EightBit::Z80::bit(int n, uint8_t& operand) {
	auto& f = F();
	auto carry = f & CF;
	uint8_t discarded = operand;
	andr(discarded, 1 << n);
	clearFlag(f, PF, discarded);
	setFlag(f, CF, carry);
	return operand;
}

uint8_t& EightBit::Z80::res(int n, uint8_t& operand) {
	auto bit = 1 << n;
	operand &= ~bit;
	return operand;
}

uint8_t& EightBit::Z80::set(int n, uint8_t& operand) {
	auto bit = 1 << n;
	operand |= bit;
	return operand;
}

#pragma endregion BIT/SET/RES

#pragma region Miscellaneous instructions

void EightBit::Z80::neg() {
	auto& a = A();
	auto& f = F();
	auto original = a;
	a = 0;
	sub(a, original);
	setFlag(f, PF, original == Bit7);
	setFlag(f, CF, original);
}

void EightBit::Z80::daa() {

	auto& acc = A();
	auto& f = F();

	auto a = acc;

	auto lowAdjust = (f & HC) | (lowNibble(acc) > 9);
	auto highAdjust = (f & CF) | (acc > 0x99);

	if (f & NF) {
		if (lowAdjust)
			a -= 6;
		if (highAdjust)
			a -= 0x60;
	} else {
		if (lowAdjust)
			a += 6;
		if (highAdjust)
			a += 0x60;
	}

	f = (f & (CF | NF)) | (acc > 0x99) | ((acc ^ a) & HC);

	adjustSZPXY<Z80>(f, a);

	acc = a;
}

void EightBit::Z80::cpl() {
	auto& a = A();
	auto& f = F();
	a = ~a;
	adjustXY<Z80>(f, A());
	setFlag(f, HC | NF);
}

void EightBit::Z80::scf() {
	auto& f = F();
	setFlag(f, CF);
	adjustXY<Z80>(f, A());
	clearFlag(f, HC | NF);
}

void EightBit::Z80::ccf() {
	auto& f = F();
	auto carry = f & CF;
	setFlag(f, HC, carry);
	clearFlag(f, CF, carry);
	clearFlag(f, NF);
	adjustXY<Z80>(f, A());
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

void EightBit::Z80::xhtl() {
	xhtl(HL2());
}

#pragma endregion Miscellaneous instructions

#pragma region Block instructions

#pragma region Block compare instructions

void EightBit::Z80::blockCompare() {

	const auto& a = A();
	auto& f = F();

	m_memory.ADDRESS() = HL();

	auto value = m_memory.reference();
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

void EightBit::Z80::cpi() {
	blockCompare();
	HL().word++;
	MEMPTR().word++;
}

void EightBit::Z80::cpd() {
	blockCompare();
	HL().word--;
	MEMPTR().word--;
}

bool EightBit::Z80::cpir() {
	cpi();
	MEMPTR() = PC();
	auto again = (F() & PF) && !(F() & ZF);	// See CPI
	if (again)
		MEMPTR().word--;
	return again;
}

bool EightBit::Z80::cpdr() {
	cpd();
	MEMPTR().word = PC().word - 1;
	auto again = (F() & PF) && !(F() & ZF);	// See CPD
	if (!again)
		MEMPTR().word--;
	return again;
}

#pragma endregion Block compare instructions

#pragma region Block load instructions

void EightBit::Z80::blockLoad(register16_t source, register16_t destination) {
	auto& f = F();
	m_memory.ADDRESS() = source;
	auto value = m_memory.reference();
	m_memory.ADDRESS() = destination;
	m_memory.reference() = value;
	auto xy = A() + value;
	setFlag(f, XF, xy & 8);
	setFlag(f, YF, xy & 2);
	clearFlag(f, NF | HC);
	setFlag(f, PF, --BC().word);
}

void EightBit::Z80::ldd() {
	blockLoad(HL(), DE());
	HL().word--;
	DE().word--;
}

void EightBit::Z80::ldi() {
	blockLoad(HL(), DE());
	HL().word++;
	DE().word++;
}

bool EightBit::Z80::ldir() {
	ldi();
	auto again = (F() & PF) != 0;
	if (again)		// See LDI
		MEMPTR().word = PC().word - 1;
	return again;
}

bool EightBit::Z80::lddr() {
	ldd();
	auto again = (F() & PF) != 0;
	if (again)		// See LDR
		MEMPTR().word = PC().word - 1;
	return again;
}

#pragma endregion Block load instructions

#pragma region Block input instructions

void EightBit::Z80::ini() {
	auto& f = F();
	MEMPTR() = m_memory.ADDRESS() = BC();
	MEMPTR().word++;
	readPort();
	auto value = m_memory.DATA();
	m_memory.ADDRESS().word = HL().word++;
	m_memory.reference() = value;
	postDecrement(f, --B());
	setFlag(f, NF);
}

void EightBit::Z80::ind() {
	auto& f = F();
	MEMPTR() = m_memory.ADDRESS() = BC();
	MEMPTR().word--;
	readPort();
	auto value = m_memory.DATA();
	m_memory.ADDRESS().word = HL().word--;
	m_memory.reference() = value;
	postDecrement(f, --B());
	setFlag(f, NF);
}

bool EightBit::Z80::inir() {
	ini();
	return !(F() & ZF);	// See INI
}

bool EightBit::Z80::indr() {
	ind();
	return !(F() & ZF);	// See IND
}

#pragma endregion Block input instructions

#pragma region Block output instructions

void EightBit::Z80::blockOut() {
	auto& f = F();
	auto value = m_memory.reference();
	m_memory.ADDRESS().word = BC().word;
	writePort();
	postDecrement(f, --B());
	setFlag(f, NF, value & Bit7);
	setFlag(f, HC | CF, (L() + value) > 0xff);
	adjustParity<Z80>(f, ((value + L()) & 7) ^ B());
}

void EightBit::Z80::outi() {
	m_memory.ADDRESS().word = HL().word++;
	blockOut();
	MEMPTR().word = BC().word + 1;
}

void EightBit::Z80::outd() {
	m_memory.ADDRESS().word = HL().word--;
	blockOut();
	MEMPTR().word = BC().word - 1;
}

bool EightBit::Z80::otir() {
	outi();
	return !(F() & ZF);	// See OUTI
}

bool EightBit::Z80::otdr() {
	outd();
	return !(F() & ZF);	// See OUTD
}

#pragma endregion Block output instructions

#pragma endregion Block instructions

#pragma region Nibble rotation

void EightBit::Z80::rrd() {
	auto& a = A();
	auto& f = F();
	MEMPTR() = HL();
	auto memory = memptrReference();
	m_memory.reference() = promoteNibble(a) | highNibble(memory);
	a = (a & 0xf0) | lowNibble(memory);
	adjustSZPXY<Z80>(f, a);
	clearFlag(f, NF | HC);
}

void EightBit::Z80::rld() {
	auto& a = A();
	auto& f = F();
	MEMPTR() = HL();
	auto memory = memptrReference();
	m_memory.reference() = promoteNibble(memory) | lowNibble(a);
	a = (a & 0xf0) | highNibble(memory);
	adjustSZPXY<Z80>(f, a);
	clearFlag(f, NF | HC);
}

#pragma endregion Nibble rotation

int EightBit::Z80::step() {
	ExecutingInstruction.fire(*this);
	m_displaced = m_prefixCB = m_prefixDD = m_prefixED = m_prefixFD = false;
	cycles = 0;
	return fetchExecute();
}

int EightBit::Z80::execute(uint8_t opcode) {

	if (!M1())
		throw std::logic_error("M1 cannot be high");

	auto x = (opcode & 0b11000000) >> 6;
	auto y = (opcode & 0b111000) >> 3;
	auto z = (opcode & 0b111);

	auto p = (y & 0b110) >> 1;
	auto q = (y & 1);

	if (!(m_prefixCB && m_displaced)) {
		incrementRefresh();
		M1() = false;
	}

	if (m_prefixCB)
		executeCB(x, y, z, p, q);
	else if (m_prefixED)
		executeED(x, y, z, p, q);
	else
		executeOther(x, y, z, p, q);

	if (cycles == 0)
		throw std::logic_error("Unhandled opcode");

	return cycles;
}

void EightBit::Z80::executeCB(int x, int y, int z, int p, int q) {
	auto& f = F();
	switch (x) {
	case 0:	// rot[y] r[z]
		switch (y) {
		case 0:
			adjustSZP<Z80>(f, m_displaced ? R2(z) = rlc(DISPLACED()) : rlc(R(z)));
			break;
		case 1:
			adjustSZP<Z80>(f, m_displaced ? R2(z) = rrc(DISPLACED()) : rrc(R(z)));
			break;
		case 2:
			adjustSZP<Z80>(f, m_displaced ? R2(z) = rl(DISPLACED()) : rl(R(z)));
			break;
		case 3:
			adjustSZP<Z80>(f, m_displaced ? R2(z) = rr(DISPLACED()) : rr(R(z)));
			break;
		case 4:
			adjustSZP<Z80>(f, m_displaced ? R2(z) = sla(DISPLACED()) : sla(R(z)));
			break;
		case 5:
			adjustSZP<Z80>(f, m_displaced ? R2(z) = sra(DISPLACED()) : sra(R(z)));
			break;
		case 6:
			adjustSZP<Z80>(f, m_displaced ? R2(z) = sll(DISPLACED()) : sll(R(z)));
			break;
		case 7:
			adjustSZP<Z80>(f, m_displaced ? R2(z) = srl(DISPLACED()) : srl(R(z)));
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
			bit(y, DISPLACED());
			adjustXY<Z80>(f, MEMPTR().high);
			cycles += 20;
		} else {
			auto operand = bit(y, R(z));
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
			R2(z) = res(y, DISPLACED());
			cycles += 23;
		} else {
			res(y, R(z));
			cycles += 8;
			if (z == 6)
				cycles += 7;
		}
		break;
	case 3:	// SET y, r[z]
		if (m_displaced) {
			R2(z) = set(y, DISPLACED());
			cycles += 23;
		} else {
			set(y, R(z));
			cycles += 8;
			if (z == 6)
				cycles += 7;
		}
		break;
	}
}

void EightBit::Z80::executeED(int x, int y, int z, int p, int q) {
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
				R(y) = m_memory.DATA();
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
				m_memory.placeDATA(R(y));
			writePort();
			cycles += 12;
			break;
		case 2:	// 16-bit add/subtract with carry
			switch (q) {
			case 0:	// SBC HL, rp[p]
				sbcViaMemptr(HL2(), RP(p));
				break;
			case 1:	// ADC HL, rp[p]
				adcViaMemptr(HL2(), RP(p));
				break;
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
			}
			cycles += 20;
			break;
		case 4:	// Negate accumulator
			neg();
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
			}
			cycles += 8;
			break;
		case 7:	// Assorted ops
			switch (y) {
			case 0:	// LD I,A
				IV() = A();
				cycles += 9;
				break;
			case 1:	// LD R,A
				REFRESH() = A();
				cycles += 9;
				break;
			case 2:	// LD A,I
				A() = IV();
				adjustSZXY<Z80>(f, A());
				setFlag(f, PF, IFF2());
				clearFlag(f, NF | HC);
				cycles += 9;
				break;
			case 3:	// LD A,R
				A() = REFRESH();
				adjustSZXY<Z80>(f, A());
				clearFlag(f, NF | HC);
				setFlag(f, PF, IFF2());
				cycles += 9;
				break;
			case 4:	// RRD
				rrd();
				cycles += 18;
				break;
			case 5:	// RLD
				rld();
				cycles += 18;
				break;
			case 6:	// NOP
			case 7:	// NOP
				cycles += 4;
				break;
			}
			break;
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
					PC().word -= 2;
					cycles += 5;
				}
				break;
			case 7:	// LDDR
				if (lddr()) {
					PC().word -= 2;
					cycles += 5;
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
					PC().word -= 2;
					cycles += 5;
				}
				break;
			case 7:	// CPDR
				if (cpdr()) {
					PC().word -= 2;
					cycles += 5;
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
					PC().word -= 2;
					cycles += 5;
				}
				break;
			case 7:	// INDR
				if (indr()) {
					PC().word -= 2;
					cycles += 5;
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
					PC().word -= 2;
					cycles += 5;
				}
				break;
			case 7:	// OTDR
				if (otdr()) {
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
				if (jrConditionalFlag(y - 4))
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
				addViaMemptr(HL2(), RP(p));
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
					MEMPTR().high = memptrReference() = A();
					cycles += 7;
					break;
				case 1:	// LD (DE),A
					MEMPTR() = DE();
					MEMPTR().high = memptrReference() = A();
					cycles += 7;
					break;
				case 2:	// LD (nn),HL
					fetchWord();
					setWordViaMemptr(HL2());
					cycles += 16;
					break;
				case 3: // LD (nn),A
					fetchWord();
					MEMPTR().high = memptrReference() = A();
					cycles += 13;
					break;
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					MEMPTR() = BC();
					A() = memptrReference();
					cycles += 7;
					break;
				case 1:	// LD A,(DE)
					MEMPTR() = DE();
					A() = memptrReference();
					cycles += 7;
					break;
				case 2:	// LD HL,(nn)
					fetchWord();
					getWordViaMemptr(HL2());
					cycles += 16;
					break;
				case 3:	// LD A,(nn)
					fetchWord();
					A() = memptrReference();
					cycles += 13;
					break;
				}
				break;
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
			}
			cycles += 6;
			break;
		case 4:	// 8-bit INC
			postIncrement(f, ++R(y));	// INC r
			cycles += 4;
			break;
		case 5:	// 8-bit DEC
			postDecrement(f, --R(y));	// DEC r
			cycles += 4;
			if (y == 6)
				cycles += 7;
			break;
		case 6:	// 8-bit load immediate
			R(y) = fetchByte();	// LD r,n
			cycles += 7;
			if (y == 6)
				cycles += 3;
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
			}
			cycles += 4;
			break;
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
						H() = R(z);
						normal = false;
						break;
					case 5:
						L() = R(z);
						normal = false;
						break;
					}
				}
				if (y == 6) {
					switch (z) {
					case 4:
						R(y) = H();
						normal = false;
						break;
					case 5:
						R(y) = L();
						normal = false;
						break;
					}
				}
			}
			if (normal)
				R(y) = R(z);
			if ((y == 6) || (z == 6))	// M operations
				cycles += 3;
		}
		cycles += 4;
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
			sub(A(), R(z));
			break;
		case 3:	// SBC A,r
			sbc(A(), R(z));
			break;
		case 4:	// AND r
			andr(A(), R(z));
			break;
		case 5:	// XOR r
			xorr(A(), R(z));
			break;
		case 6:	// OR r
			orr(A(), R(z));
			break;
		case 7:	// CP r
			compare(R(z));
			break;
		}
		cycles += 4;
		if (z == 6)
			cycles += 3;
		break;
	case 3:
		switch (z) {
		case 0:	// Conditional return
			if (returnConditionalFlag(y))
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
				}
			}
			break;
		case 2:	// Conditional jump
			jumpConditionalFlag(y);
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
				m_memory.ADDRESS().low = fetchByte();
				m_memory.ADDRESS().high = A();
				MEMPTR() = m_memory.ADDRESS();
				m_memory.placeDATA(A());
				writePort();
				MEMPTR().low++;
				cycles += 11;
				break;
			case 3:	// IN A,(n)
				m_memory.ADDRESS().low = fetchByte();
				m_memory.ADDRESS().high = A();
				MEMPTR() = m_memory.ADDRESS();
				readPort();
				A() = m_memory.DATA();
				MEMPTR().low++;
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
			}
			break;
		case 4:	// Conditional call: CALL cc[y], nn
			if (callConditionalFlag(y))
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
				}
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
				sub(A(), fetchByte());
				break;
			case 3:	// SBC A,n
				sbc(A(), fetchByte());
				break;
			case 4:	// AND n
				andr(A(), fetchByte());
				break;
			case 5:	// XOR n
				xorr(A(), fetchByte());
				break;
			case 6:	// OR n
				orr(A(), fetchByte());
				break;
			case 7:	// CP n
				compare(fetchByte());
				break;
			}
			cycles += 7;
			break;
		case 7:	// Restart: RST y * 8
			restart(y << 3);
			cycles += 11;
			break;
		}
		break;
	}
}
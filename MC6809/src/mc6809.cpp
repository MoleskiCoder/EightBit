#include "stdafx.h"
#include "mc6809.h"

EightBit::mc6809::mc6809(Bus& bus)
: BigEndianProcessor(bus) {}

void EightBit::mc6809::powerOn() {
	Processor::powerOn();
}

int EightBit::mc6809::step() {
	resetCycles();
	auto returned = 0;
	if (LIKELY(powered())) {
		ExecutingInstruction.fire(*this);
		returned = execute(fetchByte());
		ExecutedInstruction.fire(*this);
	}
	return returned;
}

void EightBit::mc6809::reset() {
	Processor::reset();
	DP() = 0;	// Reestablish zero page
	CC() |= (IF & FF);	// Disable interrupts
	m_prefix10 = m_prefix11 = false;
	jump(getWordPaged(0xff, RESETvector));
}

int EightBit::mc6809::execute(uint8_t opcode) {
	if (m_prefix10)
		return execute10(opcode);
	else if (m_prefix11)
		return execute11(opcode);
	return executeUnprefixed(opcode);
}

int EightBit::mc6809::executeUnprefixed(uint8_t opcode) {

	ASSUME(!m_prefix10);
	ASSUME(!m_prefix11);
	ASSUME(cycles() == 0);

	switch (opcode) {

	case 0x10:	m_prefix10 = true;	break;
	case 0x11:	m_prefix11 = true;	break;

	// ABX
	case 0x3a:	addCycles(3);	abx();									break;		// ABX (inherent)

	// ADC
	case 0x89:	addCycles(2);	A() = adc(A(), AM_immediate_byte());	break;		// ADC (ADCA, immediate)
	case 0x99:	addCycles(4);	A() = adc(A(), AM_direct_byte());		break;		// ADC (ADCA, direct)
	case 0xa9:	addCycles(4);	A() = adc(A(), AM_indexed_byte());		break;		// ADC (ADCA, indexed)
	case 0xb9:	addCycles(4);	A() = adc(A(), AM_extended_byte());		break;		// ADC (ADCA, extended)

	case 0xc9:	addCycles(2);	B() = adc(B(), AM_immediate_byte());	break;		// ADC (ADCB, immediate)
	case 0xd9:	addCycles(4);	B() = adc(B(), AM_direct_byte());		break;		// ADC (ADCB, direct)
	case 0xe9:	addCycles(4);	B() = adc(B(), AM_indexed_byte());		break;		// ADC (ADCB, indexed)
	case 0xf9:	addCycles(4);	B() = adc(B(), AM_extended_byte());		break;		// ADC (ADCB, extended)

	// ADD
	case 0x8b: addCycles(2);	A() = add(A(), AM_immediate_byte());	break;		// ADD (ADDA, immediate)
	case 0x9b: addCycles(4);	A() = add(A(), AM_direct_byte());		break;		// ADD (ADDA, direct)
	case 0xab: addCycles(4);	A() = add(A(), AM_indexed_byte());		break;		// ADD (ADDA, indexed)
	case 0xbb: addCycles(5);	A() = add(A(), AM_extended_byte());		break;		// ADD (ADDA, extended)

	case 0xcb: addCycles(2);	B() = add(B(), AM_immediate_byte());	break;		// ADD (ADDB, immediate)
	case 0xdb: addCycles(4);	B() = add(B(), AM_direct_byte());		break;		// ADD (ADDB, direct)
	case 0xeb: addCycles(4);	B() = add(B(), AM_indexed_byte());		break;		// ADD (ADDB, indexed)
	case 0xfb: addCycles(5);	B() = add(B(), AM_extended_byte());		break;		// ADD (ADDB, extended)

	case 0xc3: addCycles(4);	D() = add(D(), AM_immediate_word());	break;		// ADD (ADDD, immediate)
	case 0xd3: addCycles(6);	D() = add(D(), AM_direct_word());		break;		// ADD (ADDD, direct)
	case 0xe3: addCycles(6);	D() = add(D(), AM_indexed_word());		break;		// ADD (ADDD, indexed)
	case 0xf3: addCycles(7);	D() = add(D(), AM_extended_word());		break;		// ADD (ADDD, extended)

	// AND
	case 0x84:	addCycles(2);	A() = andr(A(), AM_immediate_byte());	break;		// AND (ANDA, immediate)
	case 0x94:	addCycles(4);	A() = andr(A(), AM_direct_byte());		break;		// AND (ANDA, direct)
	case 0xa4:	addCycles(4);	A() = andr(A(), AM_indexed_byte());		break;		// AND (ANDA, indexed)
	case 0xb4:	addCycles(5);	A() = andr(A(), AM_extended_byte());	break;		// AND (ANDA, extended)

	case 0xc4:	addCycles(2);	B() = andr(B(), AM_immediate_byte());	break;		// AND (ANDB, immediate)
	case 0xd4:	addCycles(4);	B() = andr(B(), AM_direct_byte());		break;		// AND (ANDB, direct)
	case 0xe4:	addCycles(4);	B() = andr(B(), AM_indexed_byte());		break;		// AND (ANDB, indexed)
	case 0xf4:	addCycles(5);	B() = andr(B(), AM_extended_byte());	break;		// AND (ANDB, extended)

	case 0x1c:	addCycles(3);	CC() = andr(CC(), AM_immediate_byte());	break;		// AND (ANDCC, immediate)

	// ASL
	case 0x08:	addCycles(6);	BUS().write(asl(AM_direct_byte()));		break;		// ASL (ASL, direct)
	case 0x48:	addCycles(2);	A() = asl(A());							break;		// ASL (ASLA, inherent)
	case 0x58:	addCycles(2);	B() = asl(B());							break;		// ASL (ASLB, inherent)
	case 0x68:	addCycles(6);	BUS().write(asl(AM_indexed_byte()));	break;		// ASL (ASL, indexed)
	case 0x78:	addCycles(7);	BUS().write(asl(AM_extended_byte()));	break;		// ASL (ASL, extended)

	// ASR
	case 0x07:	addCycles(6);	BUS().write(asr(AM_direct_byte()));		break;		// ASR (ASR, direct)
	case 0x47:	addCycles(2);	A() = asr(A());							break;		// ASR (ASRA, inherent)
	case 0x57:	addCycles(2);	B() = asr(B());							break;		// ASR (ASRB, inherent)
	case 0x67:	addCycles(6);	BUS().write(asr(AM_indexed_byte()));	break;		// ASR (ASR, indexed)
	case 0x77:	addCycles(7);	BUS().write(asr(AM_extended_byte()));	break;		// ASR (ASR, extended)

	// BIT
	case 0x85:	addCycles(2);	andr(A(), AM_immediate_byte());			break;		// BIT (BITA, immediate)
	case 0x95:	addCycles(4);	andr(A(), AM_direct_byte());			break;		// BIT (BITA, direct)
	case 0xa5:	addCycles(4);	andr(A(), AM_indexed_byte());			break;		// BIT (BITA, indexed)
	case 0xb5:	addCycles(5);	andr(A(), AM_extended_byte());			break;		// BIT (BITA, extended)

	case 0xc5:	addCycles(2);	andr(B(), AM_immediate_byte());			break;		// BIT (BITB, immediate)
	case 0xd5:	addCycles(4);	andr(B(), AM_direct_byte());			break;		// BIT (BITB, direct)
	case 0xe5:	addCycles(4);	andr(B(), AM_indexed_byte());			break;		// BIT (BITB, indexed)
	case 0xf5:	addCycles(5);	andr(B(), AM_extended_byte());			break;		// BIT (BITB, extended)

	// CLR
	case 0x0f:	addCycles(6);	Address_direct(); BUS().write(clr());	break;		// CLR (CLR, direct)
	case 0x4f:	addCycles(2);	A() = clr();							break;		// CLR (CLRA, implied)
	case 0x5f:	addCycles(2);	B() = clr();							break;		// CLR (CLRB, implied)
	case 0x6f:	addCycles(6);	Address_indexed(); BUS().write(clr());	break;		// CLR (CLR, indexed)
	case 0x7f:	addCycles(7);	Address_extended(); BUS().write(clr());	break;		// CLR (CLR, extended)

	// CMP

	// CMPA
	case 0x81:	addCycles(2);	cmp(A(), AM_immediate_byte());			break;		// CMP (CMPA, immediate)
	case 0x91:	addCycles(4);	cmp(A(), AM_direct_byte());				break;		// CMP (CMPA, direct)
	case 0xa1:	addCycles(4);	cmp(A(), AM_indexed_byte());			break;		// CMP (CMPA, indexed)
	case 0xb1:	addCycles(5);	cmp(A(), AM_extended_byte());			break;		// CMP (CMPA, extended)

	// CMPB
	case 0xc1:	addCycles(2);	cmp(B(), AM_immediate_byte());			break;		// CMP (CMPB, immediate)
	case 0xd1:	addCycles(4);	cmp(B(), AM_direct_byte());				break;		// CMP (CMPB, direct)
	case 0xe1:	addCycles(4);	cmp(B(), AM_indexed_byte());			break;		// CMP (CMPB, indexed)
	case 0xf1:	addCycles(5);	cmp(B(), AM_extended_byte());			break;		// CMP (CMPB, extended)

	// CMPX
	case 0x8c:	addCycles(4);	cmp(X(), AM_immediate_word());			break;		// CMP (CMPX, immediate)
	case 0x9c:	addCycles(6);	cmp(X(), AM_direct_word());				break;		// CMP (CMPX, direct)
	case 0xac:	addCycles(6);	cmp(X(), AM_indexed_word());			break;		// CMP (CMPX, indexed)
	case 0xbc:	addCycles(7);	cmp(X(), AM_extended_word());			break;		// CMP (CMPX, extended)

	// NEG
	case 0x00:	addCycles(6);	BUS().write(neg(AM_direct_byte()));		break;		// NEG (direct)
	case 0x40:	addCycles(2);	A() = neg(A());							break;		// NEG (NEGA, inherent)
	case 0x50:	addCycles(2);	B() = neg(B());							break;		// NEG (NEGB, inherent)
	case 0x60:	addCycles(6);	BUS().write(neg(AM_indexed_byte()));	break;		// NEG (indexed)
	case 0x70:	addCycles(7);	BUS().write(neg(AM_extended_byte()));	break;		// NEG (extended)

	// COM
	case 0x03:	addCycles(6);	BUS().write(com(AM_direct_byte()));		break;		// COM (COM direct)
	case 0x43:	addCycles(2);	A() = com(A());							break;		// COM (COMA inherent)
	case 0x53:	addCycles(2);	B() = com(B());							break;		// COM (COMB inherent)
	case 0x63:	addCycles(6);	BUS().write(com(AM_indexed_byte()));	break;		// COM (COM indexed)
	case 0x73:	addCycles(7);	BUS().write(com(AM_extended_byte()));	break;		// COM (COM extended)

	// CWAI
	case 0x3c:	addCycles(20);	cwai(AM_direct_byte());					break;		// CWAI (CWAI direct)

	default:
		UNREACHABLE;
	}

	if (m_prefix10 || m_prefix11)
		ASSUME(cycles() == 0);
	else
		ASSUME(cycles() > 0);

	return cycles();
}

int EightBit::mc6809::execute10(uint8_t opcode) {

	ASSUME(m_prefix10);
	ASSUME(!m_prefix11);
	ASSUME(cycles() == 0);

	switch (opcode) {

	// CMP

	// CMPD
	case 0x83:	addCycles(5);	cmp(D(), AM_immediate_word());			break;		// CMP (CMPD, immediate)
	case 0x93:	addCycles(7);	cmp(D(), AM_direct_word());				break;		// CMP (CMPD, direct)
	case 0xa3:	addCycles(7);	cmp(D(), AM_indexed_word());			break;		// CMP (CMPD, indexed)
	case 0xb3:	addCycles(8);	cmp(D(), AM_extended_word());			break;		// CMP (CMPD, extended)

	// CMPY
	case 0x8c:	addCycles(5);	cmp(Y(), AM_immediate_word());			break;		// CMP (CMPY, immediate)
	case 0x9c:	addCycles(7);	cmp(Y(), AM_direct_word());				break;		// CMP (CMPY, direct)
	case 0xac:	addCycles(7);	cmp(Y(), AM_indexed_word());			break;		// CMP (CMPY, indexed)
	case 0xbc:	addCycles(8);	cmp(Y(), AM_extended_word());			break;		// CMP (CMPY, extended)

	default:
		UNREACHABLE;
	}

	m_prefix10 = false;

	ASSUME(cycles() > 0);
	return cycles();
}

int EightBit::mc6809::execute11(uint8_t opcode) {

	ASSUME(m_prefix10);
	ASSUME(!m_prefix11);
	ASSUME(cycles() == 0);

	switch (opcode) {

	// CMP

	// CMPU
	case 0x83:	addCycles(5);	cmp(U(), AM_immediate_word());			break;		// CMP (CMPU, immediate)
	case 0x93:	addCycles(7);	cmp(U(), AM_direct_word());				break;		// CMP (CMPU, direct)
	case 0xa3:	addCycles(7);	cmp(U(), AM_indexed_word());			break;		// CMP (CMPU, indexed)
	case 0xb3:	addCycles(8);	cmp(U(), AM_extended_word());			break;		// CMP (CMPU, extended)

	// CMPS
	case 0x8c:	addCycles(5);	cmp(S(), AM_immediate_word());			break;		// CMP (CMPS, immediate)
	case 0x9c:	addCycles(7);	cmp(S(), AM_direct_word());				break;		// CMP (CMPS, direct)
	case 0xac:	addCycles(7);	cmp(S(), AM_indexed_word());			break;		// CMP (CMPS, indexed)
	case 0xbc:	addCycles(8);	cmp(S(), AM_extended_word());			break;		// CMP (CMPS, extended)

	default:
		UNREACHABLE;
	}

	m_prefix11 = false;

	ASSUME(cycles() > 0);
	return cycles();
}

//

EightBit::register16_t& EightBit::mc6809::RR(int which) {
	ASSUME(which >= 0);
	ASSUME(which <= 3);
	switch (which) {
	case 0b00:
		return X();
	case 0b01:
		return Y();
	case 0b10:
		return U();
	case 0b11:
		return S();
	default:
		UNREACHABLE;
	}
}

void EightBit::mc6809::Address_direct() {
	BUS().ADDRESS() = register16_t(fetchByte(), DP());
}

void EightBit::mc6809::Address_indexed() {
	const auto type = fetchByte();
	auto& r = RR((type & (Bit6 | Bit5)) >> 5);

	if (type & Bit7) {
		const auto indirect = type & Bit4;
		switch (type & Mask4) {
		case 0b0000:	// ,R+
			ASSUME(!indirect);
			addCycles(2);
			BUS().ADDRESS() = r++;
			break;
		case 0b0001:	// ,R++
			addCycles(3);
			BUS().ADDRESS() = r;
			r += 2;
			break;
		case 0b0010:	// ,-R
			ASSUME(!indirect);
			addCycles(2);
			BUS().ADDRESS() = --r;
			break;
		case 0b0011:	// ,--R
			addCycles(3);
			r -= 2;
			BUS().ADDRESS() = r;
			break;
		case 0b0100:	// ,R
			BUS().ADDRESS() = r;
			break;
		case 0b0101:	// B,R
			addCycles(1);
			BUS().ADDRESS() = r + (int8_t)B();
			break;
		case 0b0110:	// A,R
			addCycles(1);
			BUS().ADDRESS() = r + (int8_t)A();
			break;
		case 0b1000:	// n,R (eight-bit)
			addCycles(1);
			BUS().ADDRESS() = r + (int8_t)fetchByte();
			break;
		case 0b1001:	// n,R (sixteen-bit)
			addCycles(4);
			BUS().ADDRESS() = r + (int16_t)fetchWord().word;
			break;
		case 0b1011:	// D,R
			addCycles(4);
			BUS().ADDRESS() = r + D();
			break;
		case 0b1100:	// n,PCR (eight-bit)
			addCycles(1);
			BUS().ADDRESS() = PC() + (int8_t)fetchByte();
			break;
		case 0b1101:	// n,PCR (sixteen-bit)
			addCycles(1);
			BUS().ADDRESS() = PC() + (int16_t)fetchWord().word;
			break;
		default:
			UNREACHABLE;
		}
		if (indirect) {
			addCycles(3);
			BUS().ADDRESS() = fetchWord();
		}
	} else {
		// EA = ,R + 5-bit offset
		addCycle();
		BUS().ADDRESS() = r + (type & Mask5);
	}
}

void EightBit::mc6809::Address_extended() {
	BUS().ADDRESS() = fetchWord();
}

//

uint8_t EightBit::mc6809::AM_immediate_byte() {
	return fetchByte();
}

uint8_t EightBit::mc6809::AM_direct_byte() {
	Address_direct();
	return BUS().read();
}

uint8_t EightBit::mc6809::AM_indexed_byte() {
	Address_indexed();
	return BUS().read();
}

uint8_t EightBit::mc6809::AM_extended_byte() {
	Address_extended();
	return BUS().read();
}

//

EightBit::register16_t EightBit::mc6809::AM_immediate_word() {
	return fetchWord();
}

EightBit::register16_t EightBit::mc6809::AM_direct_word() {
	Address_direct();
	return getWord();
}

EightBit::register16_t EightBit::mc6809::AM_indexed_word() {
	Address_indexed();
	return getWord();
}

EightBit::register16_t EightBit::mc6809::AM_extended_word() {
	Address_extended();
	return getWord();
}

//

void EightBit::mc6809::abx() {
	X() += B();
}

uint8_t EightBit::mc6809::neg(uint8_t operand) {
	setFlag(CC(), VF, operand == Bit7);
	const register16_t result = 0 - operand;
	operand = result.low;
	adjustNZ(operand);
	adjustCarry(result.word);
	return operand;
}

uint8_t EightBit::mc6809::adc(uint8_t operand, uint8_t data) {
	return add(operand, data, CC() & CF);
}

uint8_t EightBit::mc6809::add(uint8_t operand, uint8_t data, int carry) {
	const register16_t addition = operand + data + carry;
	adjustAddition(operand, data, addition);
	return addition.low;
}

EightBit::register16_t EightBit::mc6809::add(register16_t operand, register16_t data) {
	const uint32_t addition = operand.word + data.word;
	adjustAddition(operand.word, data.word, addition);
	return addition & Mask16;
}

uint8_t EightBit::mc6809::andr(uint8_t operand, uint8_t data) {
	const uint8_t result = operand & data;
	clearFlag(CC(), VF);
	adjustNZ(result);
	return result;
}

uint8_t EightBit::mc6809::asl(uint8_t operand) {
	setFlag(CC(), CF, operand & Bit7);
	operand <<= 1;
	adjustNZ(operand);
	const auto overflow = (CC() & CF) ^ ((CC() & NF) >> 3);
	setFlag(CC(), VF, overflow);
	return operand;
}

uint8_t EightBit::mc6809::asr(uint8_t operand) {
	setFlag(CC(), CF, operand & Bit7);
	operand >>= 1;
	adjustNZ(operand);
	return operand;
}

uint8_t EightBit::mc6809::clr() {
	clearFlag(CC(), HF | ZF | VF | CF);
	setFlag(CC(), ZF);
	return 0;
}

void EightBit::mc6809::cmp(const uint8_t operand, const uint8_t data) {
	const register16_t difference = operand - data;
	adjustSubtraction(operand, data, difference);
}

void EightBit::mc6809::cmp(register16_t operand, register16_t data) {
	const uint32_t difference = operand.word - data.word;
	adjustSubtraction(operand.word, data.word, difference);
}

uint8_t EightBit::mc6809::com(uint8_t operand) {
	const uint8_t result = ~operand;
	adjustNZ(result);
	clearFlag(CC(), VF);
	setFlag(CC(), CF);
	return result;
}

void EightBit::mc6809::cwai(uint8_t data) {
	CC() &= data;
	pushWord(PC());
	pushWord(U());
	pushWord(Y());
	pushWord(X());
	push(DP());
	push(B());
	push(A());
	push(CC());
	halt();
}
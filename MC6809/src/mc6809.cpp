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
	jump(getWordPaged(0xff, RESETvector));
}

int EightBit::mc6809::execute(uint8_t cell) {

	switch (cell) {

	// ABX
	case 0x3a:	addCycles(3);	abx();									break;		// ABX (inherent)

	// ADC
	case 0x89:	addCycles(2);	A() = adc(A(), AM_immediate_byte());	break;		// ADC (ADCA, immediate)
	case 0x99:	addCycles(4);	A() = adc(A(), AM_direct_byte());		break;		// ADC (ADCA, direct)
	case 0xA9:	addCycles(4);	A() = adc(A(), AM_indexed_byte());		break;		// ADC (ADCA, indexed)
	case 0xB9:	addCycles(4);	A() = adc(A(), AM_extended_byte());		break;		// ADC (ADCA, extended)

	case 0xC9:	addCycles(2);	B() = adc(B(), AM_immediate_byte());	break;		// ADC (ADCB, immediate)
	case 0xD9:	addCycles(4);	B() = adc(B(), AM_direct_byte());		break;		// ADC (ADCB, direct)
	case 0xE9:	addCycles(4);	B() = adc(B(), AM_indexed_byte());		break;		// ADC (ADCB, indexed)
	case 0xF9:	addCycles(4);	B() = adc(B(), AM_extended_byte());		break;		// ADC (ADCB, extended)

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
	case 0xA5:	addCycles(4);	andr(A(), AM_indexed_byte());			break;		// BIT (BITA, indexed)
	case 0xB5:	addCycles(5);	andr(A(), AM_extended_byte());			break;		// BIT (BITA, extended)

	case 0xc5:	addCycles(2);	andr(B(), AM_immediate_byte());			break;		// BIT (BITB, immediate)
	case 0xd5:	addCycles(4);	andr(B(), AM_direct_byte());			break;		// BIT (BITB, direct)
	case 0xe5:	addCycles(4);	andr(B(), AM_indexed_byte());			break;		// BIT (BITB, indexed)
	case 0xf5:	addCycles(5);	andr(B(), AM_extended_byte());			break;		// BIT (BITB, extended)

	// NEG
	case 0x00:	addCycles(6);	BUS().write(neg(AM_direct_byte()));		break;		// NEG (direct)
	case 0x40:	addCycles(2);	A() = neg(A());							break;		// NEG (NEGA, inherent)
	case 0x50:	addCycles(2);	B() = neg(B());							break;		// NEG (NEGB, inherent)
	case 0x60:	addCycles(6);	BUS().write(neg(AM_indexed_byte()));	break;		// NEG (indexed)
	case 0x70:	addCycles(7);	BUS().write(neg(AM_extended_byte()));	break;		// NEG (extended)

	default:
		UNREACHABLE;
	}

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
	setFlag(CC(), CF, result.word & Bit8);
	return operand;
}

uint8_t EightBit::mc6809::adc(uint8_t operand, uint8_t data) {
	return add(operand, data, CC() & CF);
}

uint8_t EightBit::mc6809::add(uint8_t operand, uint8_t data, int carry) {
	const register16_t result = operand + data + carry;
	adjustNZ(result.low);
	setFlag(CC(), CF, result.word & Bit8);
	return result.low;
}

EightBit::register16_t EightBit::mc6809::add(register16_t operand, register16_t data) {
	const uint32_t addition = operand.word + data.word;
	const register16_t result = addition;
	setFlag(CC(), NF, result.high & Bit7);
	setFlag(CC(), ZF, result.word == 0);
	setFlag(CC(), CF, addition & Bit16);
	return result;
}

uint8_t EightBit::mc6809::andr(uint8_t operand, uint8_t data) {
	const auto result = operand & data;
	clearFlag(CC(), VF);
	adjustNZ(result);
	return result;
}

uint8_t EightBit::mc6809::asl(uint8_t operand) {
	setFlag(CC(), CF, operand & Bit7);
	operand <<= 1;
	adjustNZ(operand);
	const auto overflow = (CC() & CF) ^ ((CC() & NF) >> 3);
	setFlag(CC(), overflow);
	return operand;
}

uint8_t EightBit::mc6809::asr(uint8_t operand) {
	setFlag(CC(), CF, operand & Bit7);
	operand >>= 1;
	adjustNZ(operand);
	return operand;
}

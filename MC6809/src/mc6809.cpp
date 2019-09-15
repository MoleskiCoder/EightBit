#include "stdafx.h"
#include "mc6809.h"

#include <algorithm>
#include <cassert>

EightBit::mc6809::mc6809(Bus& bus)
: BigEndianProcessor(bus) {
	RaisedPOWER.connect([this](EventArgs) {
		lowerBA();
		lowerBS();
		lowerRW();
	});
}

DEFINE_PIN_LEVEL_CHANGERS(NMI, mc6809);
DEFINE_PIN_LEVEL_CHANGERS(FIRQ, mc6809);
DEFINE_PIN_LEVEL_CHANGERS(HALT, mc6809);
DEFINE_PIN_LEVEL_CHANGERS(BA, mc6809);
DEFINE_PIN_LEVEL_CHANGERS(BS, mc6809);
DEFINE_PIN_LEVEL_CHANGERS(RW, mc6809);

int EightBit::mc6809::step() {
	resetCycles();
	ExecutingInstruction.fire(*this);
	if (LIKELY(powered())) {
		m_prefix10 = m_prefix11 = false;
		if (UNLIKELY(halted()))
			handleHALT();
		else if (UNLIKELY(lowered(RESET())))
			handleRESET();
		else if (UNLIKELY(lowered(NMI())))
			handleNMI();
		else if (UNLIKELY(lowered(FIRQ()) && !fastInterruptMasked()))
			handleFIRQ();
		else if (UNLIKELY(lowered(INT()) && !interruptMasked()))
			handleINT();
		else
			Processor::execute(fetchByte());
	}
	ExecutedInstruction.fire(*this);
	return cycles();
}

// Interrupt (etc.) handlers

void EightBit::mc6809::handleHALT() {
	raiseBA();
	raiseBS();
}

void EightBit::mc6809::handleRESET() {
	BigEndianProcessor::handleRESET();
	raiseNMI();
	lowerBA();
	raiseBS();
	DP() = 0;
	setFlag(CC(), IF);	// Disable IRQ
	setFlag(CC(), FF);	// Disable FIRQ
	jump(getWordPaged(0xff, RESETvector));
	tick(10);
}

void EightBit::mc6809::handleNMI() {
	raiseNMI();
	lowerBA();
	raiseBS();
	saveEntireRegisterState();
	setFlag(CC(), IF);	// Disable IRQ
	setFlag(CC(), FF);	// Disable FIRQ
	jump(getWordPaged(0xff, NMIvector));
	tick(12);
}

void EightBit::mc6809::handleINT() {
	BigEndianProcessor::handleINT();
	lowerBA();
	raiseBS();
	saveEntireRegisterState();
	setFlag(CC(), IF);	// Disable IRQ
	jump(getWordPaged(0xff, IRQvector));
	tick(12);
}

void EightBit::mc6809::handleFIRQ() {
	raiseFIRQ();
	lowerBA();
	raiseBS();
	savePartialRegisterState();
	setFlag(CC(), IF);	// Disable IRQ
	setFlag(CC(), FF);	// Disable FIRQ
	jump(getWordPaged(0xff, FIRQvector));
	tick(12);
}

//

void EightBit::mc6809::busWrite() {
	lowerRW();
	Processor::busWrite();
}

uint8_t EightBit::mc6809::busRead() {
	raiseRW();
	return Processor::busRead();
}

//

int EightBit::mc6809::execute() {
	lowerBA();
	lowerBS();
	const bool prefixed = m_prefix10 || m_prefix11;
	const bool unprefixed = !prefixed;
	if (unprefixed) {
		executeUnprefixed();
	} else {
		if (m_prefix10)
			execute10();
		else
			execute11();
	}
	assert(cycles() > 0);
	return cycles();
}

void EightBit::mc6809::executeUnprefixed() {

	assert(!(m_prefix10 || m_prefix11));
	assert(cycles() == 0);

	switch (opcode()) {

	case 0x10:	m_prefix10 = true;	Processor::execute(fetchByte());	break;
	case 0x11:	m_prefix11 = true;	Processor::execute(fetchByte());	break;

	// ABX
	case 0x3a:	tick(3);	X() += B();											break;		// ABX (inherent)

	// ADC
	case 0x89:	tick(2);	A() = adc(A(), AM_immediate_byte());				break;		// ADC (ADCA immediate)
	case 0x99:	tick(4);	A() = adc(A(), AM_direct_byte());					break;		// ADC (ADCA direct)
	case 0xa9:	tick(4);	A() = adc(A(), AM_indexed_byte());					break;		// ADC (ADCA indexed)
	case 0xb9:	tick(4);	A() = adc(A(), AM_extended_byte());					break;		// ADC (ADCA extended)

	case 0xc9:	tick(2);	B() = adc(B(), AM_immediate_byte());				break;		// ADC (ADCB immediate)
	case 0xd9:	tick(4);	B() = adc(B(), AM_direct_byte());					break;		// ADC (ADCB direct)
	case 0xe9:	tick(4);	B() = adc(B(), AM_indexed_byte());					break;		// ADC (ADCB indexed)
	case 0xf9:	tick(4);	B() = adc(B(), AM_extended_byte());					break;		// ADC (ADCB extended)

	// ADD
	case 0x8b:	tick(2);	A() = add(A(), AM_immediate_byte());				break;		// ADD (ADDA immediate)
	case 0x9b:	tick(4);	A() = add(A(), AM_direct_byte());					break;		// ADD (ADDA direct)
	case 0xab:	tick(4);	A() = add(A(), AM_indexed_byte());					break;		// ADD (ADDA indexed)
	case 0xbb:	tick(5);	A() = add(A(), AM_extended_byte());					break;		// ADD (ADDA extended)

	case 0xcb:	tick(2);	B() = add(B(), AM_immediate_byte());				break;		// ADD (ADDB immediate)
	case 0xdb:	tick(4);	B() = add(B(), AM_direct_byte());					break;		// ADD (ADDB direct)
	case 0xeb:	tick(4);	B() = add(B(), AM_indexed_byte());					break;		// ADD (ADDB indexed)
	case 0xfb:	tick(5);	B() = add(B(), AM_extended_byte());					break;		// ADD (ADDB extended)

	case 0xc3:	tick(4);	D() = add(D(), AM_immediate_word());				break;		// ADD (ADDD immediate)
	case 0xd3:	tick(6);	D() = add(D(), AM_direct_word());					break;		// ADD (ADDD direct)
	case 0xe3:	tick(6);	D() = add(D(), AM_indexed_word());					break;		// ADD (ADDD indexed)
	case 0xf3:	tick(7);	D() = add(D(), AM_extended_word());					break;		// ADD (ADDD extended)

	// AND
	case 0x84:	tick(2);	A() = andr(A(), AM_immediate_byte());				break;		// AND (ANDA immediate)
	case 0x94:	tick(4);	A() = andr(A(), AM_direct_byte());					break;		// AND (ANDA direct)
	case 0xa4:	tick(4);	A() = andr(A(), AM_indexed_byte());					break;		// AND (ANDA indexed)
	case 0xb4:	tick(5);	A() = andr(A(), AM_extended_byte());				break;		// AND (ANDA extended)

	case 0xc4:	tick(2);	B() = andr(B(), AM_immediate_byte());				break;		// AND (ANDB immediate)
	case 0xd4:	tick(4);	B() = andr(B(), AM_direct_byte());					break;		// AND (ANDB direct)
	case 0xe4:	tick(4);	B() = andr(B(), AM_indexed_byte());					break;		// AND (ANDB indexed)
	case 0xf4:	tick(5);	B() = andr(B(), AM_extended_byte());				break;		// AND (ANDB extended)

	case 0x1c:	tick(3);	CC() &= AM_immediate_byte();						break;		// AND (ANDCC immediate)

	// ASL/LSL
	case 0x08:	tick(6);	Processor::busWrite(asl(AM_direct_byte()));			break;		// ASL (direct)
	case 0x48:	tick(2);	A() = asl(A());										break;		// ASL (ASLA inherent)
	case 0x58:	tick(2);	B() = asl(B());										break;		// ASL (ASLB inherent)
	case 0x68:	tick(6);	Processor::busWrite(asl(AM_indexed_byte()));		break;		// ASL (indexed)
	case 0x78:	tick(7);	Processor::busWrite(asl(AM_extended_byte()));		break;		// ASL (extended)

	// ASR
	case 0x07:	tick(6);	Processor::busWrite(asr(AM_direct_byte()));			break;		// ASR (direct)
	case 0x47:	tick(2);	A() = asr(A());										break;		// ASR (ASRA inherent)
	case 0x57:	tick(2);	B() = asr(B());										break;		// ASR (ASRB inherent)
	case 0x67:	tick(6);	Processor::busWrite(asr(AM_indexed_byte()));		break;		// ASR (indexed)
	case 0x77:	tick(7);	Processor::busWrite(asr(AM_extended_byte()));		break;		// ASR (extended)

	// BIT
	case 0x85:	tick(2);	bit(A(), AM_immediate_byte());						break;		// BIT (BITA immediate)
	case 0x95:	tick(4);	bit(A(), AM_direct_byte());							break;		// BIT (BITA direct)
	case 0xa5:	tick(4);	bit(A(), AM_indexed_byte());						break;		// BIT (BITA indexed)
	case 0xb5:	tick(5);	bit(A(), AM_extended_byte());						break;		// BIT (BITA extended)

	case 0xc5:	tick(2);	bit(B(), AM_immediate_byte());						break;		// BIT (BITB immediate)
	case 0xd5:	tick(4);	bit(B(), AM_direct_byte());							break;		// BIT (BITB direct)
	case 0xe5:	tick(4);	bit(B(), AM_indexed_byte());						break;		// BIT (BITB indexed)
	case 0xf5:	tick(5);	bit(B(), AM_extended_byte());						break;		// BIT (BITB extended)

	// CLR
	case 0x0f:	tick(6);	Processor::busWrite(Address_direct(), clr());		break;		// CLR (direct)
	case 0x4f:	tick(2);	A() = clr();										break;		// CLR (CLRA implied)
	case 0x5f:	tick(2);	B() = clr();										break;		// CLR (CLRB implied)
	case 0x6f:	tick(6);	Processor::busWrite(Address_indexed(), clr());		break;		// CLR (indexed)
	case 0x7f:	tick(7);	Processor::busWrite(Address_extended(), clr());		break;		// CLR (extended)

	// CMP

	// CMPA
	case 0x81:	tick(2);	cmp(A(), AM_immediate_byte());						break;		// CMP (CMPA, immediate)
	case 0x91:	tick(4);	cmp(A(), AM_direct_byte());							break;		// CMP (CMPA, direct)
	case 0xa1:	tick(4);	cmp(A(), AM_indexed_byte());						break;		// CMP (CMPA, indexed)
	case 0xb1:	tick(5);	cmp(A(), AM_extended_byte());						break;		// CMP (CMPA, extended)

	// CMPB
	case 0xc1:	tick(2);	cmp(B(), AM_immediate_byte());						break;		// CMP (CMPB, immediate)
	case 0xd1:	tick(4);	cmp(B(), AM_direct_byte());							break;		// CMP (CMPB, direct)
	case 0xe1:	tick(4);	cmp(B(), AM_indexed_byte());						break;		// CMP (CMPB, indexed)
	case 0xf1:	tick(5);	cmp(B(), AM_extended_byte());						break;		// CMP (CMPB, extended)

	// CMPX
	case 0x8c:	tick(4);	cmp(X(), AM_immediate_word());						break;		// CMP (CMPX, immediate)
	case 0x9c:	tick(6);	cmp(X(), AM_direct_word());							break;		// CMP (CMPX, direct)
	case 0xac:	tick(6);	cmp(X(), AM_indexed_word());						break;		// CMP (CMPX, indexed)
	case 0xbc:	tick(7);	cmp(X(), AM_extended_word());						break;		// CMP (CMPX, extended)

	// COM
	case 0x03:	tick(6);	Processor::busWrite(com(AM_direct_byte()));			break;		// COM (direct)
	case 0x43:	tick(2);	A() = com(A());										break;		// COM (COMA inherent)
	case 0x53:	tick(2);	B() = com(B());										break;		// COM (COMB inherent)
	case 0x63:	tick(6);	Processor::busWrite(com(AM_indexed_byte()));		break;		// COM (indexed)
	case 0x73:	tick(7);	Processor::busWrite(com(AM_extended_byte()));		break;		// COM (extended)

	// CWAI
	case 0x3c:	tick(11);	cwai(AM_direct_byte());								break;		// CWAI (direct)

	// DAA
	case 0x19:	tick(2);	A() = da(A());										break;		// DAA (inherent)

	// DEC
	case 0x0a:	tick(6);	Processor::busWrite(dec(AM_direct_byte()));			break;		// DEC (direct)
	case 0x4a:	tick(2);	A() = dec(A());										break;		// DEC (DECA inherent)
	case 0x5a:	tick(2);	B() = dec(B());										break;		// DEC (DECB inherent)
	case 0x6a:	tick(6);	Processor::busWrite(dec(AM_indexed_byte()));		break;		// DEC (indexed)
	case 0x7a:	tick(7);	Processor::busWrite(dec(AM_extended_byte()));		break;		// DEC (extended)

	// EOR

	// EORA
	case 0x88:	tick(2);	A() = eorr(A(), AM_immediate_byte());				break;		// EOR (EORA immediate)
	case 0x98:	tick(4);	A() = eorr(A(), AM_direct_byte());					break;		// EOR (EORA direct)
	case 0xa8:	tick(4);	A() = eorr(A(), AM_indexed_byte());					break;		// EOR (EORA indexed)
	case 0xb8:	tick(5);	A() = eorr(A(), AM_extended_byte());				break;		// EOR (EORA extended)

	// EORB
	case 0xc8:	tick(2);	B() = eorr(B(), AM_immediate_byte());				break;		// EOR (EORB immediate)
	case 0xd8:	tick(4);	B() = eorr(B(), AM_direct_byte());					break;		// EOR (EORB direct)
	case 0xe8:	tick(4);	B() = eorr(B(), AM_indexed_byte());					break;		// EOR (EORB indexed)
	case 0xf8:	tick(5);	B() = eorr(B(), AM_extended_byte());				break;		// EOR (EORB extended)

	// EXG
	case 0x1e:	tick(8);	exg(AM_immediate_byte());							break;		// EXG (R1,R2 immediate)

	// INC
	case 0x0c:	tick(6);	Processor::busWrite(inc(AM_direct_byte()));			break;		// INC (direct)
	case 0x4c:	tick(2);	A() = inc(A());										break;		// INC (INCA inherent)
	case 0x5c:	tick(2);	B() = inc(B());										break;		// INC (INCB inherent)
	case 0x6c:	tick(6);	Processor::busWrite(inc(AM_indexed_byte()));		break;		// INC (indexed)
	case 0x7c:	tick(7);	Processor::busWrite(inc(AM_extended_byte()));		break;		// INC (extended)

	// JMP
	case 0x0e:	tick(6);	jump(Address_direct());								break;		// JMP (direct)
	case 0x6e:	tick(6);	jump(Address_indexed());							break;		// JMP (indexed)
	case 0x7e:	tick(7);	jump(Address_extended());							break;		// JMP (extended)

	// JSR
	case 0x9d:	tick(6);	jsr(Address_direct());								break;		// JSR (direct)
	case 0xad:	tick(6);	jsr(Address_indexed());								break;		// JSR (indexed)
	case 0xbd:	tick(7);	jsr(Address_extended());							break;		// JSR (extended)

	// LD

	// LDA
	case 0x86:	tick(2);	A() = ld(AM_immediate_byte());						break;		// LD (LDA immediate)
	case 0x96:	tick(4);	A() = ld(AM_direct_byte());							break;		// LD (LDA direct)
	case 0xa6:	tick(4);	A() = ld(AM_indexed_byte());						break;		// LD (LDA indexed)
	case 0xb6:	tick(5);	A() = ld(AM_extended_byte());						break;		// LD (LDA extended)

	// LDB
	case 0xc6:	tick(2);	B() = ld(AM_immediate_byte());						break;		// LD (LDB immediate)
	case 0xd6:	tick(4);	B() = ld(AM_direct_byte());							break;		// LD (LDB direct)
	case 0xe6:	tick(4);	B() = ld(AM_indexed_byte());						break;		// LD (LDB indexed)
	case 0xf6:	tick(5);	B() = ld(AM_extended_byte());						break;		// LD (LDB extended)

	// LDD
	case 0xcc:	tick(3);	D() = ld(AM_immediate_word());						break;		// LD (LDD immediate)
	case 0xdc:	tick(5);	D() = ld(AM_direct_word());							break;		// LD (LDD direct)
	case 0xec:	tick(5);	D() = ld(AM_indexed_word());						break;		// LD (LDD indexed)
	case 0xfc:	tick(6);	D() = ld(AM_extended_word());						break;		// LD (LDD extended)

	// LDU
	case 0xce:	tick(3);	U() = ld(AM_immediate_word());						break;		// LD (LDU immediate)
	case 0xde:	tick(5);	U() = ld(AM_direct_word());							break;		// LD (LDU direct)
	case 0xee:	tick(5);	U() = ld(AM_indexed_word());						break;		// LD (LDU indexed)
	case 0xfe:	tick(6);	U() = ld(AM_extended_word());						break;		// LD (LDU extended)

	// LDX
	case 0x8e:	tick(3);	X() = ld(AM_immediate_word());						break;		// LD (LDX immediate)
	case 0x9e:	tick(5);	X() = ld(AM_direct_word());							break;		// LD (LDX direct)
	case 0xae:	tick(5);	X() = ld(AM_indexed_word());						break;		// LD (LDX indexed)
	case 0xbe:	tick(6);	X() = ld(AM_extended_word());						break;		// LD (LDX extended)

	// LEA
	case 0x30:	tick(4);	adjustZero(X() = Address_indexed());				break;		// LEA (LEAX indexed)
	case 0x31:	tick(4);	adjustZero(Y() = Address_indexed());				break;		// LEA (LEAY indexed)
	case 0x32:	tick(4);	S() = Address_indexed();							break;		// LEA (LEAS indexed)
	case 0x33:	tick(4);	U() = Address_indexed();							break;		// LEA (LEAU indexed)

	// LSR
	case 0x04:	tick(6);	Processor::busWrite(lsr(AM_direct_byte()));			break;		// LSR (direct)
	case 0x44:	tick(2);	A() = lsr(A());										break;		// LSR (LSRA inherent)
	case 0x54:	tick(2);	B() = lsr(B());										break;		// LSR (LSRB inherent)
	case 0x64:	tick(6);	Processor::busWrite(lsr(AM_indexed_byte()));		break;		// LSR (indexed)
	case 0x74:	tick(7);	Processor::busWrite(lsr(AM_extended_byte()));		break;		// LSR (extended)

	// MUL
	case 0x3d:	tick(11);	D() = mul(A(), B());								break;		// MUL (inherent)

	// NEG
	case 0x00:	tick(6);	Processor::busWrite(neg(AM_direct_byte()));			break;		// NEG (direct)
	case 0x40:	tick(2);	A() = neg(A());										break;		// NEG (NEGA, inherent)
	case 0x50:	tick(2);	B() = neg(B());										break;		// NEG (NEGB, inherent)
	case 0x60:	tick(6);	Processor::busWrite(neg(AM_indexed_byte()));		break;		// NEG (indexed)
	case 0x70:	tick(7);	Processor::busWrite(neg(AM_extended_byte()));		break;		// NEG (extended)

	// NOP
	case 0x12:	tick(2);														break;		// NOP (inherent)

	// OR

	// ORA
	case 0x8a:	tick(2);	A() = orr(A(), AM_immediate_byte());				break;		// OR (ORA immediate)
	case 0x9a:	tick(4);	A() = orr(A(), AM_direct_byte());					break;		// OR (ORA direct)
	case 0xaa:	tick(4);	A() = orr(A(), AM_indexed_byte());					break;		// OR (ORA indexed)
	case 0xba:	tick(5);	A() = orr(A(), AM_extended_byte());					break;		// OR (ORA extended)

	// ORB
	case 0xca:	tick(2);	B() = orr(B(), AM_immediate_byte());				break;		// OR (ORB immediate)
	case 0xda:	tick(4);	B() = orr(B(), AM_direct_byte());					break;		// OR (ORB direct)
	case 0xea:	tick(4);	B() = orr(B(), AM_indexed_byte());					break;		// OR (ORB indexed)
	case 0xfa:	tick(5);	B() = orr(B(), AM_extended_byte());					break;		// OR (ORB extended)

	// ORCC
	case 0x1a:	tick(3);	CC() |= AM_immediate_byte();						break;		// OR (ORCC immediate)

	// PSH
	case 0x34:	tick(5);	psh(S(), AM_immediate_byte());						break;		// PSH (PSHS immediate)
	case 0x36:	tick(5);	psh(U(), AM_immediate_byte());						break;		// PSH (PSHU immediate)

	// PUL
	case 0x35:	tick(5);	pul(S(), AM_immediate_byte());						break;		// PUL (PULS immediate)
	case 0x37:	tick(5);	pul(U(), AM_immediate_byte());						break;		// PUL (PULU immediate)

	// ROL
	case 0x09:	tick(6);	Processor::busWrite(rol(AM_direct_byte()));			break;		// ROL (direct)
	case 0x49:	tick(2);	A() = rol(A());										break;		// ROL (ROLA inherent)
	case 0x59:	tick(2);	B() = rol(B());										break;		// ROL (ROLB inherent)
	case 0x69:	tick(6);	Processor::busWrite(rol(AM_indexed_byte()));		break;		// ROL (indexed)
	case 0x79:	tick(7);	Processor::busWrite(rol(AM_extended_byte()));		break;		// ROL (extended)

	// ROR
	case 0x06:	tick(6);	Processor::busWrite(ror(AM_direct_byte()));			break;		// ROR (direct)
	case 0x46:	tick(2);	A() = ror(A());										break;		// ROR (RORA inherent)
	case 0x56:	tick(2);	B() = ror(B());										break;		// ROR (RORB inherent)
	case 0x66:	tick(6);	Processor::busWrite(ror(AM_indexed_byte()));		break;		// ROR (indexed)
	case 0x76:	tick(7);	Processor::busWrite(ror(AM_extended_byte()));		break;		// ROR (extended)

	// RTI
	case 0x3B:	tick(6);	rti();												break;		// RTI (inherent)

	// RTS
	case 0x39:	tick(5);	rts();												break;		// RTS (inherent)

	// SBC

	// SBCA
	case 0x82:	tick(4);	A() = sbc(A(), AM_immediate_byte());				break;		// SBC (SBCA immediate)
	case 0x92:	tick(4);	A() = sbc(A(), AM_direct_byte());					break;		// SBC (SBCA direct)
	case 0xa2:	tick(4);	A() = sbc(A(), AM_indexed_byte());					break;		// SBC (SBCA indexed)
	case 0xb2:	tick(5);	A() = sbc(A(), AM_extended_byte());					break;		// SBC (SBCB extended)

	// SBCB
	case 0xc2:	tick(4);	B() = sbc(B(), AM_immediate_byte());				break;		// SBC (SBCB immediate)
	case 0xd2:	tick(4);	B() = sbc(B(), AM_direct_byte());					break;		// SBC (SBCB direct)
	case 0xe2:	tick(4);	B() = sbc(B(), AM_indexed_byte());					break;		// SBC (SBCB indexed)
	case 0xf2:	tick(5);	B() = sbc(B(), AM_extended_byte());					break;		// SBC (SBCB extended)

	// SEX
	case 0x1d:	tick(2);	A() = sex(B());										break;		// SEX (inherent)

	// ST

	// STA
	case 0x97:	tick(4);	Processor::busWrite(Address_direct(), st(A()));		break;		// ST (STA direct)
	case 0xa7:	tick(4);	Processor::busWrite(Address_indexed(), st(A()));	break;		// ST (STA indexed)
	case 0xb7:	tick(5);	Processor::busWrite(Address_extended(), st(A()));	break;		// ST (STA extended)

	// STB
	case 0xd7:	tick(4);	Processor::busWrite(Address_direct(), st(B()));		break;		// ST (STB direct)
	case 0xe7:	tick(4);	Processor::busWrite(Address_indexed(), st(B()));	break;		// ST (STB indexed)
	case 0xf7:	tick(5);	Processor::busWrite(Address_extended(), st(B()));	break;		// ST (STB extended)

	// STD
	case 0xdd:	tick(5);	Processor::setWord(Address_direct(), st(D()));		break;		// ST (STD direct)
	case 0xed:	tick(5);	Processor::setWord(Address_indexed(), st(D()));		break;		// ST (STD indexed)
	case 0xfd:	tick(6);	Processor::setWord(Address_extended(), st(D()));	break;		// ST (STD extended)

	// STU
	case 0xdf:	tick(5);	Processor::setWord(Address_direct(), st(U()));		break;		// ST (STU direct)
	case 0xef:	tick(5);	Processor::setWord(Address_indexed(), st(U()));		break;		// ST (STU indexed)
	case 0xff:	tick(6);	Processor::setWord(Address_extended(), st(U()));	break;		// ST (STU extended)

	// STX
	case 0x9f:	tick(5);	Processor::setWord(Address_direct(), st(X()));		break;		// ST (STX direct)
	case 0xaf:	tick(5);	Processor::setWord(Address_indexed(), st(X()));		break;		// ST (STX indexed)
	case 0xbf:	tick(6);	Processor::setWord(Address_extended(), st(X()));	break;		// ST (STX extended)

	// SUB

	// SUBA
	case 0x80:	tick(2);	A() = sub(A(), AM_immediate_byte());				break;		// SUB (SUBA immediate)
	case 0x90:	tick(4);	A() = sub(A(), AM_direct_byte());					break;		// SUB (SUBA direct)
	case 0xa0:	tick(4);	A() = sub(A(), AM_indexed_byte());					break;		// SUB (SUBA indexed)
	case 0xb0:	tick(5);	A() = sub(A(), AM_extended_byte());					break;		// SUB (SUBA extended)

	// SUBB
	case 0xc0:	tick(2);	B() = sub(B(), AM_immediate_byte());				break;		// SUB (SUBB immediate)
	case 0xd0:	tick(4);	B() = sub(B(), AM_direct_byte());					break;		// SUB (SUBB direct)
	case 0xe0:	tick(4);	B() = sub(B(), AM_indexed_byte());					break;		// SUB (SUBB indexed)
	case 0xf0:	tick(5);	B() = sub(B(), AM_extended_byte());					break;		// SUB (SUBB extended)

	// SUBD
	case 0x83:	tick(4);	D() = sub(D(), AM_immediate_word());				break;		// SUB (SUBD immediate)
	case 0x93:	tick(6);	D() = sub(D(), AM_direct_word());					break;		// SUB (SUBD direct)
	case 0xa3:	tick(6);	D() = sub(D(), AM_indexed_word());					break;		// SUB (SUBD indexed)
	case 0xb3:	tick(7);	D() = sub(D(), AM_extended_word());					break;		// SUB (SUBD extended)

	// SWI
	case 0x3f:	tick(10);	swi();												break;		// SWI (inherent)

	// SYNC
	case 0x13:	tick(4);	halt();												break;		// SYNC (inherent)

	// TFR
	case 0x1f:	tick(6);	tfr(AM_immediate_byte());							break;		// TFR (immediate)

	// TST
	case 0x0d:	tick(6);	tst(AM_direct_byte());								break;		// TST (direct)
	case 0x4d:	tick(2);	tst(A());											break;		// TST (TSTA inherent)
	case 0x5d:	tick(2);	tst(B());											break;		// TST (TSTB inherent)
	case 0x6d:	tick(6);	tst(AM_indexed_byte());								break;		// TST (indexed)
	case 0x7d:	tick(7);	tst(AM_extended_byte());							break;		// TST (extended)

	// Branching

	case 0x16:	tick(5);	jump(Address_relative_word());						break;		// BRA (LBRA relative)
	case 0x17:	tick(9);	jsr(Address_relative_word());						break;		// BSR (LBSR relative)
	case 0x20:	tick(3);	jump(Address_relative_byte());						break;		// BRA (relative)
	case 0x21:	tick(3);	Address_relative_byte();							break;		// BRN (relative)
	case 0x22:	tick(3);	branchShort(HI());									break;		// BHI (relative)
	case 0x23:	tick(3);	branchShort(LS());									break;		// BLS (relative)
	case 0x24:	tick(3);	branchShort(!carry());								break;		// BCC (relative)
	case 0x25:	tick(3);	branchShort(carry());								break;		// BCS (relative)
	case 0x26:	tick(3);	branchShort(!zero());								break;		// BNE (relative)
	case 0x27:	tick(3);	branchShort(zero());								break;		// BEQ (relative)
	case 0x28: 	tick(3);	branchShort(!overflow());							break;		// BVC (relative)
	case 0x29: 	tick(3);	branchShort(overflow());							break;		// BVS (relative)
	case 0x2a: 	tick(3);	branchShort(!negative());							break;		// BPL (relative)
	case 0x2b: 	tick(3);	branchShort(negative());							break;		// BMI (relative)
	case 0x2c:	tick(3);	branchShort(GE());									break;		// BGE (relative)
	case 0x2d:	tick(3);	branchShort(LT());									break;		// BLT (relative)
	case 0x2e:	tick(3);	branchShort(GT());									break;		// BGT (relative)
	case 0x2f:	tick(3);	branchShort(LE());									break;		// BLE (relative)

	case 0x8d:	tick(7);	jsr(Address_relative_byte());						break;		// BSR (relative)

	default:
		UNREACHABLE;
	}
}

void EightBit::mc6809::execute10() {

	assert(m_prefix10 && !m_prefix11);
	assert(cycles() == 0);

	switch (opcode()) {

	// CMP

	// CMPD
	case 0x83:	tick(5);	cmp(D(), AM_immediate_word());						break;		// CMP (CMPD, immediate)
	case 0x93:	tick(7);	cmp(D(), AM_direct_word());							break;		// CMP (CMPD, direct)
	case 0xa3:	tick(7);	cmp(D(), AM_indexed_word());						break;		// CMP (CMPD, indexed)
	case 0xb3:	tick(8);	cmp(D(), AM_extended_word());						break;		// CMP (CMPD, extended)

	// CMPY
	case 0x8c:	tick(5);	cmp(Y(), AM_immediate_word());						break;		// CMP (CMPY, immediate)
	case 0x9c:	tick(7);	cmp(Y(), AM_direct_word());							break;		// CMP (CMPY, direct)
	case 0xac:	tick(7);	cmp(Y(), AM_indexed_word());						break;		// CMP (CMPY, indexed)
	case 0xbc:	tick(8);	cmp(Y(), AM_extended_word());						break;		// CMP (CMPY, extended)

	// LD

	// LDS
	case 0xce:	tick(4);	S() = ld(AM_immediate_word());						break;		// LD (LDS immediate)
	case 0xde:	tick(6);	S() = ld(AM_direct_word());							break;		// LD (LDS direct)
	case 0xee:	tick(6);	S() = ld(AM_indexed_word());						break;		// LD (LDS indexed)
	case 0xfe:	tick(7);	S() = ld(AM_extended_word());						break;		// LD (LDS extended)

	// LDY
	case 0x8e:	tick(4);	Y() = ld(AM_immediate_word());						break;		// LD (LDY immediate)
	case 0x9e:	tick(6);	Y() = ld(AM_direct_word());							break;		// LD (LDY direct)
	case 0xae:	tick(6);	Y() = ld(AM_indexed_word());						break;		// LD (LDY indexed)
	case 0xbe:	tick(7);	Y() = ld(AM_extended_word());						break;		// LD (LDY extended)

	// Branching

	case 0x21:	tick(5);	Address_relative_word();							break;		// BRN (LBRN relative)
	case 0x22:	tick(5);	branchLong(HI());									break;		// BHI (LBHI relative)
	case 0x23:	tick(5);	branchLong(LS());									break;		// BLS (LBLS relative)
	case 0x24:	tick(5);	branchLong(!carry());								break;		// BCC (LBCC relative)
	case 0x25:	tick(5);	branchLong(carry());								break;		// BCS (LBCS relative)
	case 0x26:	tick(5);	branchLong(!zero());								break;		// BNE (LBNE relative)
	case 0x27:	tick(5);	branchLong(zero());									break;		// BEQ (LBEQ relative)
	case 0x28:	tick(5);	branchLong(!overflow());							break;		// BVC (LBVC relative)
	case 0x29:	tick(5);	branchLong(overflow());								break;		// BVS (LBVS relative)
	case 0x2a:	tick(5);	branchLong(!negative());							break;		// BPL (LBPL relative)
	case 0x2b: 	tick(5);	branchLong(negative());								break;		// BMI (LBMI relative)
	case 0x2c:	tick(5);	branchLong(GE());									break;		// BGE (LBGE relative)
	case 0x2d:	tick(5);	branchLong(LT());									break;		// BLT (LBLT relative)
	case 0x2e:	tick(5);	branchLong(GT());									break;		// BGT (LBGT relative)
	case 0x2f:	tick(5);	branchLong(LE());									break;		// BLE (LBLE relative)

	// STS
	case 0xdf:	tick(6);	Processor::setWord(Address_direct(), st(S()));		break;		// ST (STS direct)
	case 0xef:	tick(6);	Processor::setWord(Address_indexed(), st(S()));		break;		// ST (STS indexed)
	case 0xff:	tick(7);	Processor::setWord(Address_extended(), st(S()));	break;		// ST (STS extended)

	// STY
	case 0x9f:	tick(6);	Processor::setWord(Address_direct(), st(Y()));		break;		// ST (STY direct)
	case 0xaf:	tick(6);	Processor::setWord(Address_indexed(), st(Y()));		break;		// ST (STY indexed)
	case 0xbf:	tick(7);	Processor::setWord(Address_extended(), st(Y()));	break;		// ST (STY extended)

	// SWI
	case 0x3f:	tick(11);	swi2();												break;		// SWI (SWI2 inherent)

	default:
		UNREACHABLE;
	}
}

void EightBit::mc6809::execute11() {

	assert(!m_prefix10 && m_prefix11);
	assert(cycles() == 0);

	switch (opcode()) {

	// CMP

	// CMPU
	case 0x83:	tick(5);	cmp(U(), AM_immediate_word());						break;		// CMP (CMPU, immediate)
	case 0x93:	tick(7);	cmp(U(), AM_direct_word());							break;		// CMP (CMPU, direct)
	case 0xa3:	tick(7);	cmp(U(), AM_indexed_word());						break;		// CMP (CMPU, indexed)
	case 0xb3:	tick(8);	cmp(U(), AM_extended_word());						break;		// CMP (CMPU, extended)

	// CMPS
	case 0x8c:	tick(5);	cmp(S(), AM_immediate_word());						break;		// CMP (CMPS, immediate)
	case 0x9c:	tick(7);	cmp(S(), AM_direct_word());							break;		// CMP (CMPS, direct)
	case 0xac:	tick(7);	cmp(S(), AM_indexed_word());						break;		// CMP (CMPS, indexed)
	case 0xbc:	tick(8);	cmp(S(), AM_extended_word());						break;		// CMP (CMPS, extended)

	// SWI
	case 0x3f:	tick(11);	swi3();												break;		// SWI (SWI3 inherent)

	default:
		UNREACHABLE;
	}
}

//

void EightBit::mc6809::push(const uint8_t value) {
	pushS(value);
}

uint8_t EightBit::mc6809::pop() {
	return popS();
}

void EightBit::mc6809::push(register16_t& stack, const uint8_t value) {
	Processor::busWrite(--stack, value);
}

uint8_t EightBit::mc6809::pop(register16_t& stack) {
	return Processor::busRead(stack++);
}

//

EightBit::register16_t& EightBit::mc6809::RR(const int which) {
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

EightBit::register16_t EightBit::mc6809::Address_relative_byte() {
	return PC() + (int8_t)fetchByte();
}

EightBit::register16_t EightBit::mc6809::Address_relative_word() {
	return PC() + (int16_t)fetchWord().word;
}

EightBit::register16_t EightBit::mc6809::Address_direct() {
	return register16_t(fetchByte(), DP());
}

EightBit::register16_t EightBit::mc6809::Address_indexed() {
	const auto type = fetchByte();
	auto& r = RR((type & (Bit6 | Bit5)) >> 5);

	register16_t address = Mask16;
	if (type & Bit7) {
		const auto indirect = type & Bit4;
		switch (type & Mask4) {
		case 0b0000:	// ,R+
			ASSUME(!indirect);
			tick(2);
			address = r++;
			break;
		case 0b0001:	// ,R++
			tick(3);
			address = r;
			r += 2;
			break;
		case 0b0010:	// ,-R
			ASSUME(!indirect);
			tick(2);
			address = --r;
			break;
		case 0b0011:	// ,--R
			tick(3);
			r -= 2;
			address = r;
			break;
		case 0b0100:	// ,R
			address = r;
			break;
		case 0b0101:	// B,R
			tick(1);
			address = r + (int8_t)B();
			break;
		case 0b0110:	// A,R
			tick(1);
			address = r + (int8_t)A();
			break;
		case 0b1000:	// n,R (eight-bit)
			tick(1);
			address = r + (int8_t)fetchByte();
			break;
		case 0b1001:	// n,R (sixteen-bit)
			tick(4);
			address = r + (int16_t)fetchWord().word;
			break;
		case 0b1011:	// D,R
			tick(4);
			address = r + D();
			break;
		case 0b1100:	// n,PCR (eight-bit)
			tick(1);
			address = Address_relative_byte();
			break;
		case 0b1101:	// n,PCR (sixteen-bit)
			tick(2);
			address = Address_relative_word();
			break;
		case 0b1111:	// [n]
			assert(indirect);
			tick(2);
			address = Address_extended();
			break;
		default:
			UNREACHABLE;
		}
		if (indirect) {
			tick(3);
			address = Processor::getWord(address);
		}
	} else {
		// EA = ,R + 5-bit offset
		tick();
		address = r + signExtend(5, type & Mask5);
	}
	return address;
}

EightBit::register16_t EightBit::mc6809::Address_extended() {
	return fetchWord();
}

//

uint8_t EightBit::mc6809::AM_immediate_byte() {
	return fetchByte();
}

uint8_t EightBit::mc6809::AM_direct_byte() {
	return Processor::busRead(Address_direct());
}

uint8_t EightBit::mc6809::AM_indexed_byte() {
	return Processor::busRead(Address_indexed());
}

uint8_t EightBit::mc6809::AM_extended_byte() {
	return Processor::busRead(Address_extended());
}

//

EightBit::register16_t EightBit::mc6809::AM_immediate_word() {
	return fetchWord();
}

EightBit::register16_t EightBit::mc6809::AM_direct_word() {
	return Processor::getWord(Address_direct());
}

EightBit::register16_t EightBit::mc6809::AM_indexed_word() {
	return Processor::getWord(Address_indexed());
}

EightBit::register16_t EightBit::mc6809::AM_extended_word() {
	return Processor::getWord(Address_extended());
}

//

void EightBit::mc6809::saveEntireRegisterState() {
	setFlag(CC(), EF);
	saveRegisterState();
}

void EightBit::mc6809::savePartialRegisterState() {
	clearFlag(CC(), EF);
	saveRegisterState();
}

void EightBit::mc6809::saveRegisterState() {
	psh(S(), CC() & EF ? 0xff : 0b10000001);
}

void EightBit::mc6809::restoreRegisterState() {
	pul(S(), CC() & EF ? 0xff : 0b10000001);
}

//

uint8_t EightBit::mc6809::adc(const uint8_t operand, const uint8_t data) {
	return add(operand, data, carry());
}

uint8_t EightBit::mc6809::add(const uint8_t operand, const uint8_t data, const uint8_t carry) {
	const register16_t addition = operand + data + carry;
	adjustAddition(operand, data, addition);
	return addition.low;
}

EightBit::register16_t EightBit::mc6809::add(const register16_t operand, const register16_t data) {
	const uint32_t addition = operand.word + data.word;
	adjustAddition(operand, data, addition);
	return addition & Mask16;
}

uint8_t EightBit::mc6809::andr(const uint8_t operand, const uint8_t data) {
	return through((uint8_t)(operand & data));
}

uint8_t EightBit::mc6809::asl(uint8_t operand) {
	setFlag(CC(), CF, operand & Bit7);
	adjustNZ(operand <<= 1);
	const auto overflow = carry() ^ (negative() >> 3);
	setFlag(CC(), VF, overflow);
	return operand;
}

uint8_t EightBit::mc6809::asr(uint8_t operand) {
	setFlag(CC(), CF, operand & Bit0);
	const uint8_t result = (operand >> 1) | Bit7;
	adjustNZ(result);
	return result;
}

void EightBit::mc6809::bit(const uint8_t operand, const uint8_t data) {
	andr(operand, data);
}

uint8_t EightBit::mc6809::clr() {
	clearFlag(CC(), CF);
	return through((uint8_t)0U);
}

void EightBit::mc6809::cmp(const uint8_t operand, const uint8_t data) {
	sub(operand, data);
}

void EightBit::mc6809::cmp(const register16_t operand, const register16_t data) {
	sub(operand, data);
}

uint8_t EightBit::mc6809::com(const uint8_t operand) {
	setFlag(CC(), CF);
	return through((uint8_t)~operand);
}

void EightBit::mc6809::cwai(const uint8_t data) {
	CC() &= data;
	saveEntireRegisterState();
	halt();
}

uint8_t EightBit::mc6809::da(uint8_t operand) {

	setFlag(CC(), CF, operand > 0x99);

	const auto lowAdjust = halfCarry() || (lowNibble(operand) > 9);
	const auto highAdjust = carry() || (operand > 0x99);

	if (lowAdjust)
		operand += 6;
	if (highAdjust)
		operand += 0x60;

	return through(operand);
}

uint8_t EightBit::mc6809::dec(const uint8_t operand) {
	const register16_t subtraction = operand - 1;
	const auto result = subtraction.low;
	adjustNZ(result);
	adjustOverflow(operand, 1, subtraction);
	return result;
}

uint8_t EightBit::mc6809::eorr(const uint8_t operand, const uint8_t data) {
	return through((uint8_t)(operand ^ data));
}

uint8_t& EightBit::mc6809::referenceTransfer8(const int specifier) {
	switch (specifier) {
	case 0b1000:
		return A();
	case 0b1001:
		return B();
	case 0b1010:
		return CC();
	case 0b1011:
		return DP();
	default:
		UNREACHABLE;
	}
}

EightBit::register16_t& EightBit::mc6809::referenceTransfer16(const int specifier) {
	switch (specifier) {
	case 0b0000:
		return D();
	case 0b0001:
		return X();
	case 0b0010:
		return Y();
	case 0b0011:
		return U();
	case 0b0100:
		return S();
	case 0b0101:
		return PC();
	default:
		UNREACHABLE;
	}
}

void EightBit::mc6809::exg(const uint8_t data) {

	const auto reg1 = highNibble(data);
	const auto reg2 = lowNibble(data);

	const bool type8 = !(reg1 & Bit3);	// 8 bit exchange?
	ASSUME(type8 == !(reg2 & Bit3));	// Regardless, the register exchange must be equivalent

	if (type8)
		std::swap(referenceTransfer8(reg1), referenceTransfer8(reg2));
	else
		std::swap(referenceTransfer16(reg1), referenceTransfer16(reg2));
}

uint8_t EightBit::mc6809::inc(uint8_t operand) {
	const register16_t addition = operand + 1;
	const auto result = addition.low;
	adjustNZ(result);
	adjustOverflow(operand, 1, addition);
	adjustHalfCarry(operand, 1, result);
	return result;
}

void EightBit::mc6809::jsr(const register16_t address) {
	call(address);
}

uint8_t EightBit::mc6809::lsr(uint8_t operand) {
	setFlag(CC(), CF, operand & Bit0);
	adjustNZ(operand >>= 1);
	return operand;
}

EightBit::register16_t EightBit::mc6809::mul(const uint8_t first, const uint8_t second) {
	const register16_t result = first * second;
	adjustZero(result);
	setFlag(CC(), CF, result.low & Bit7);
	return result;
}

uint8_t EightBit::mc6809::neg(uint8_t operand) {
	setFlag(CC(), VF, operand == Bit7);
	const register16_t result = 0 - operand;
	operand = result.low;
	adjustNZ(operand);
	adjustCarry(result);
	return operand;
}

uint8_t EightBit::mc6809::orr(const uint8_t operand, const uint8_t data) {
	return through((uint8_t)(operand | data));
}

void EightBit::mc6809::psh(register16_t& stack, const uint8_t data) {
	if (data & Bit7) {
		tick(2);
		pushWord(stack, PC());
	}
	if (data & Bit6) {
		tick(2);
		// Pushing to the S stack means we must be pushing U
		pushWord(stack, &stack == &S() ? U() : S());
	}
	if (data & Bit5) {
		tick(2);
		pushWord(stack, Y());
	}
	if (data & Bit4) {
		tick(2);
		pushWord(stack, X());
	}
	if (data & Bit3) {
		tick();
		push(stack, DP());
	}
	if (data & Bit2) {
		tick();
		push(stack, B());
	}
	if (data & Bit1) {
		tick();
		push(stack, A());
	}
	if (data & Bit0) {
		tick();
		push(stack, CC());
	}
}

void EightBit::mc6809::pul(register16_t& stack, const uint8_t data) {
	if (data & Bit0) {
		tick();
		CC() = pop(stack);
	}
	if (data & Bit1) {
		tick();
		A() = pop(stack);
	}
	if (data & Bit2) {
		tick();
		B() = pop(stack);
	}
	if (data & Bit3) {
		tick();
		DP() = pop(stack);
	}
	if (data & Bit4) {
		tick(2);
		X() = popWord(stack);
	}
	if (data & Bit5) {
		tick(2);
		Y() = popWord(stack);
	}
	if (data & Bit6) {
		tick(2);
		// Pulling from the S stack means we must be pulling U
		(&stack == &S() ? U() : S()) = popWord(stack);
	}
	if (data & Bit7) {
		tick(2);
		PC() = popWord(stack);
	}
}

uint8_t EightBit::mc6809::rol(const uint8_t operand) {
	const auto carryIn = carry();
	setFlag(CC(), CF, operand & Bit7);
	setFlag(CC(), VF, ((operand & Bit7) >> 7) ^ ((operand & Bit6) >> 6));
	const uint8_t result = (operand << 1) | carryIn;
	adjustNZ(result);
	return result;
}
		
uint8_t EightBit::mc6809::ror(const uint8_t operand) {
	const auto carryIn = carry();
	setFlag(CC(), CF, operand & Bit0);
	const uint8_t result = (operand >> 1) | (carryIn << 7);
	adjustNZ(result);
	return result;
}

void EightBit::mc6809::rti() {
	restoreRegisterState();
}

void EightBit::mc6809::rts() {
	ret();
}

void EightBit::mc6809::swi() {
	saveEntireRegisterState();
	setFlag(CC(), IF);	// Disable IRQ
	setFlag(CC(), FF);	// Disable FIRQ
	jump(getWordPaged(0xff, SWIvector));
}

void EightBit::mc6809::swi2() {
	saveEntireRegisterState();
	jump(getWordPaged(0xff, SWI2vector));
}

void EightBit::mc6809::swi3() {
	saveEntireRegisterState();
	jump(getWordPaged(0xff, SWI3vector));
}

uint8_t EightBit::mc6809::sex(const uint8_t from) {
	adjustNZ(from);
	return from & Bit7 ? Mask8 : 0;
}

void EightBit::mc6809::tfr(const uint8_t data) {

	const auto reg1 = highNibble(data);
	const auto reg2 = lowNibble(data);

	const bool type8 = !!(reg1 & Bit3);	// 8 bit transfer?
	ASSUME(type8 == !!(reg2 & Bit3));	// Regardless, the register transfer must be equivalent

	if (type8)
		referenceTransfer8(reg2) = referenceTransfer8(reg1);
	else
		referenceTransfer16(reg2) = referenceTransfer16(reg1);
}

uint8_t EightBit::mc6809::sbc(const uint8_t operand, const uint8_t data) {
	return sub(operand, data, carry());
}

uint8_t EightBit::mc6809::sub(const uint8_t operand, const uint8_t data, const uint8_t carry) {
	const register16_t subtraction = operand - data - carry;
	adjustSubtraction(operand, data, subtraction);
	return subtraction.low;
}

EightBit::register16_t EightBit::mc6809::sub(const register16_t operand, const register16_t data) {
	const uint32_t subtraction = operand.word - data.word;
	adjustSubtraction(operand, data, subtraction);
	return subtraction & Mask16;
}

void EightBit::mc6809::tst(const uint8_t data) {
	cmp(data, 0);
}

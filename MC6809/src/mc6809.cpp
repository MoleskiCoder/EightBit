#include "stdafx.h"
#include "../inc/mc6809.h"

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
	memoryRead({ RESETvector, 0xff });
	raiseNMI();
	lowerBA();
	raiseBS();
	DP() = 0;
	CC() = setBit(CC(), IF);	// Disable IRQ
	CC() = setBit(CC(), FF);	// Disable FIRQ
	memoryRead();
	memoryRead();
	memoryRead();
	jump(getWord());
	eat();
}

void EightBit::mc6809::handleNMI() {
	raiseNMI();
	lowerBA();
	raiseBS();
	memoryRead();
	memoryRead();
	eat();
	saveEntireRegisterState();
	eat();
	CC() = setBit(CC(), IF);	// Disable IRQ
	CC() = setBit(CC(), FF);	// Disable FIRQ
	jump(getWordPaged(0xff, NMIvector));
	eat();
}

void EightBit::mc6809::handleINT() {
	BigEndianProcessor::handleINT();
	lowerBA();
	raiseBS();
	memoryRead();
	memoryRead();
	eat();
	saveEntireRegisterState();
	eat();
	CC() = setBit(CC(), IF);	// Disable IRQ
	jump(getWordPaged(0xff, IRQvector));
	eat();
}

void EightBit::mc6809::handleFIRQ() {
	raiseFIRQ();
	lowerBA();
	raiseBS();
	memoryRead();
	memoryRead();
	eat();
	savePartialRegisterState();
	eat();
	CC() = setBit(CC(), IF);	// Disable IRQ
	CC() = setBit(CC(), FF);	// Disable FIRQ
	jump(getWordPaged(0xff, FIRQvector));
	eat();
}

//

void EightBit::mc6809::busWrite() {
	tick();
	lowerRW();
	Processor::busWrite();
}

uint8_t EightBit::mc6809::busRead() {
	tick();
	raiseRW();
	return Processor::busRead();
}

//

void EightBit::mc6809::call(register16_t destination) {
	memoryRead(destination);
	eat();
	BigEndianProcessor::pushWord(PC());
	jump(destination);
}

void EightBit::mc6809::ret() {
	BigEndianProcessor::ret();
	eat();
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
	assert(cycles() == 1);	// One fetch

	switch (opcode()) {

	case 0x10:	m_prefix10 = true;	Processor::execute(fetchByte());	break;
	case 0x11:	m_prefix11 = true;	Processor::execute(fetchByte());	break;

	// ABX
	case 0x3a:	memoryRead(); X() += B(); eat();						break;		// ABX (inherent)

	// ADC
	case 0x89:	A() = adc(A(), AM_immediate_byte());					break;		// ADC (ADCA immediate)
	case 0x99:	A() = adc(A(), AM_direct_byte());						break;		// ADC (ADCA direct)
	case 0xa9:	A() = adc(A(), AM_indexed_byte());						break;		// ADC (ADCA indexed)
	case 0xb9:	A() = adc(A(), AM_extended_byte());						break;		// ADC (ADCA extended)

	case 0xc9:	B() = adc(B(), AM_immediate_byte());					break;		// ADC (ADCB immediate)
	case 0xd9:	B() = adc(B(), AM_direct_byte());						break;		// ADC (ADCB direct)
	case 0xe9:	B() = adc(B(), AM_indexed_byte());						break;		// ADC (ADCB indexed)
	case 0xf9:	B() = adc(B(), AM_extended_byte());						break;		// ADC (ADCB extended)

	// ADD
	case 0x8b:	A() = add(A(), AM_immediate_byte());					break;		// ADD (ADDA immediate)
	case 0x9b:	A() = add(A(), AM_direct_byte());						break;		// ADD (ADDA direct)
	case 0xab:	A() = add(A(), AM_indexed_byte());						break;		// ADD (ADDA indexed)
	case 0xbb:	A() = add(A(), AM_extended_byte());						break;		// ADD (ADDA extended)

	case 0xcb:	B() = add(B(), AM_immediate_byte());					break;		// ADD (ADDB immediate)
	case 0xdb:	B() = add(B(), AM_direct_byte());						break;		// ADD (ADDB direct)
	case 0xeb:	B() = add(B(), AM_indexed_byte());						break;		// ADD (ADDB indexed)
	case 0xfb:	B() = add(B(), AM_extended_byte());						break;		// ADD (ADDB extended)

	case 0xc3:	D() = add(D(), AM_immediate_word());	 				break;		// ADD (ADDD immediate)
	case 0xd3:	D() = add(D(), AM_direct_word());						break;		// ADD (ADDD direct)
	case 0xe3:	D() = add(D(), AM_indexed_word());						break;		// ADD (ADDD indexed)
	case 0xf3:	D() = add(D(), AM_extended_word());						break;		// ADD (ADDD extended)

	// AND
	case 0x84:	A() = andr(A(), AM_immediate_byte());					break;		// AND (ANDA immediate)
	case 0x94:	A() = andr(A(), AM_direct_byte());						break;		// AND (ANDA direct)
	case 0xa4:	A() = andr(A(), AM_indexed_byte());						break;		// AND (ANDA indexed)
	case 0xb4:	A() = andr(A(), AM_extended_byte());					break;		// AND (ANDA extended)
			
	case 0xc4:	B() = andr(B(), AM_immediate_byte());					break;		// AND (ANDB immediate)
	case 0xd4:	B() = andr(B(), AM_direct_byte());						break;		// AND (ANDB direct)
	case 0xe4:	B() = andr(B(), AM_indexed_byte());						break;		// AND (ANDB indexed)
	case 0xf4:	B() = andr(B(), AM_extended_byte());					break;		// AND (ANDB extended)

	case 0x1c:	CC() &= AM_immediate_byte(); eat();						break;		// AND (ANDCC immediate)

	// ASL/LSL
	case 0x08:	RMW(AM_direct_byte, asl);								break;		// ASL (direct)
	case 0x48:	memoryRead(); A() = asl(A());							break;		// ASL (ASLA inherent)
	case 0x58:	memoryRead(); B() = asl(B());							break;		// ASL (ASLB inherent)
	case 0x68:	RMW(AM_indexed_byte, asl);								break;		// ASL (indexed)
	case 0x78:	RMW(AM_extended_byte, asl);								break;		// ASL (extended)

	// ASR
	case 0x07:	RMW(AM_direct_byte, asr);								break;		// ASR (direct)
	case 0x47:	memoryRead(); A() = asr(A());							break;		// ASR (ASRA inherent)
	case 0x57:	memoryRead(); B() = asr(B());							break;		// ASR (ASRB inherent)
	case 0x67:	RMW(AM_indexed_byte, asr);								break;		// ASR (indexed)
	case 0x77:	RMW(AM_extended_byte, asr);								break;		// ASR (extended)

	// BIT
	case 0x85:	bit(A(), AM_immediate_byte());							break;		// BIT (BITA immediate)
	case 0x95:	bit(A(), AM_direct_byte());								break;		// BIT (BITA direct)
	case 0xa5:	bit(A(), AM_indexed_byte());							break;		// BIT (BITA indexed)
	case 0xb5:	bit(A(), AM_extended_byte());							break;		// BIT (BITA extended)

	case 0xc5:	bit(B(), AM_immediate_byte());							break;		// BIT (BITB immediate)
	case 0xd5:	bit(B(), AM_direct_byte());								break;		// BIT (BITB direct)
	case 0xe5:	bit(B(), AM_indexed_byte());							break;		// BIT (BITB indexed)
	case 0xf5:	bit(B(), AM_extended_byte());							break;		// BIT (BITB extended)

	// CLR
	case 0x0f:	RMW(AM_direct_byte, clr);								break;		// CLR (direct)
	case 0x4f:	memoryRead(); A() = clr(); 								break;		// CLR (CLRA implied)
	case 0x5f:	memoryRead(); B() = clr(); 								break;		// CLR (CLRB implied)
	case 0x6f:	RMW(AM_indexed_byte, clr);								break;		// CLR (indexed)
	case 0x7f:	RMW(AM_extended_byte, clr);								break;		// CLR (extended)

	// CMP

	// CMPA
	case 0x81:	cmp(A(), AM_immediate_byte());							break;		// CMP (CMPA, immediate)
	case 0x91:	cmp(A(), AM_direct_byte());								break;		// CMP (CMPA, direct)
	case 0xa1:	cmp(A(), AM_indexed_byte());							break;		// CMP (CMPA, indexed)
	case 0xb1:	cmp(A(), AM_extended_byte());							break;		// CMP (CMPA, extended)

	// CMPB
	case 0xc1:	cmp(B(), AM_immediate_byte());							break;		// CMP (CMPB, immediate)
	case 0xd1:	cmp(B(), AM_direct_byte());								break;		// CMP (CMPB, direct)
	case 0xe1:	cmp(B(), AM_indexed_byte());							break;		// CMP (CMPB, indexed)
	case 0xf1:	cmp(B(), AM_extended_byte());							break;		// CMP (CMPB, extended)

	// CMPX
	case 0x8c:	cmp(X(), AM_immediate_word());							break;		// CMP (CMPX, immediate)
	case 0x9c:	cmp(X(), AM_direct_word());								break;		// CMP (CMPX, direct)
	case 0xac:	cmp(X(), AM_indexed_word());							break;		// CMP (CMPX, indexed)
	case 0xbc:	cmp(X(), AM_extended_word());							break;		// CMP (CMPX, extended)

	// COM
	case 0x03:	RMW(AM_direct_byte, com);								break;		// COM (direct)
	case 0x43:	memoryRead(); A() = com(A());							break;		// COM (COMA inherent)
	case 0x53:	memoryRead(); B() = com(B());							break;		// COM (COMB inherent)
	case 0x63:	RMW(AM_indexed_byte, com);								break;		// COM (indexed)
	case 0x73:	RMW(AM_extended_byte, com);								break;		// COM (extended)

	// CWAI
	case 0x3c:	cwai(AM_direct_byte());									break;		// CWAI (direct)

	// DAA
	case 0x19:	memoryRead(); A() = da(A()); 							break;		// DAA (inherent)

	// DEC
	case 0x0a:	RMW(AM_direct_byte, dec);								break;		// DEC (direct)
	case 0x4a:	memoryRead(); A() = dec(A());							break;		// DEC (DECA inherent)
	case 0x5a:	memoryRead(); B() = dec(B());							break;		// DEC (DECB inherent)
	case 0x6a:	RMW(AM_indexed_byte, dec);								break;		// DEC (indexed)
	case 0x7a:	RMW(AM_extended_byte, dec);								break;		// DEC (extended)

	// EOR

	// EORA
	case 0x88:	A() = eorr(A(), AM_immediate_byte());					break;		// EOR (EORA immediate)
	case 0x98:	A() = eorr(A(), AM_direct_byte());						break;		// EOR (EORA direct)
	case 0xa8:	A() = eorr(A(), AM_indexed_byte());						break;		// EOR (EORA indexed)
	case 0xb8:	A() = eorr(A(), AM_extended_byte());					break;		// EOR (EORA extended)

	// EORB
	case 0xc8:	B() = eorr(B(), AM_immediate_byte());					break;		// EOR (EORB immediate)
	case 0xd8:	B() = eorr(B(), AM_direct_byte());						break;		// EOR (EORB direct)
	case 0xe8:	B() = eorr(B(), AM_indexed_byte());						break;		// EOR (EORB indexed)
	case 0xf8:	B() = eorr(B(), AM_extended_byte());					break;		// EOR (EORB extended)

	// EXG
	case 0x1e:	exg(AM_immediate_byte());								break;		// EXG (R1,R2 immediate)

	// INC
	case 0x0c:	RMW(AM_direct_byte, inc);								break;		// INC (direct)
	case 0x4c:	memoryRead(); A() = inc(A());							break;		// INC (INCA inherent)
	case 0x5c:	memoryRead(); B() = inc(B());							break;		// INC (INCB inherent)
	case 0x6c:	RMW(AM_indexed_byte, inc);								break;		// INC (indexed)
	case 0x7c:	RMW(AM_extended_byte, inc);								break;		// INC (extended)

	// JMP
	case 0x0e:	jump(Address_direct());									break;		// JMP (direct)
	case 0x6e:	jump(Address_indexed());								break;		// JMP (indexed)
	case 0x7e:	jump(Address_extended());								break;		// JMP (extended)

	// JSR
	case 0x9d:	jsr(Address_direct());									break;		// JSR (direct)
	case 0xad:	jsr(Address_indexed());									break;		// JSR (indexed)
	case 0xbd:	jsr(Address_extended());								break;		// JSR (extended)

	// LD

	// LDA
	case 0x86:	A() = through(AM_immediate_byte());						break;		// LD (LDA immediate)
	case 0x96:	A() = through(AM_direct_byte());						break;		// LD (LDA direct)
	case 0xa6:	A() = through(AM_indexed_byte());						break;		// LD (LDA indexed)
	case 0xb6:	A() = through(AM_extended_byte());						break;		// LD (LDA extended)

	// LDB
	case 0xc6:	B() = through(AM_immediate_byte());						break;		// LD (LDB immediate)
	case 0xd6:	B() = through(AM_direct_byte());						break;		// LD (LDB direct)
	case 0xe6:	B() = through(AM_indexed_byte());						break;		// LD (LDB indexed)
	case 0xf6:	B() = through(AM_extended_byte());						break;		// LD (LDB extended)

	// LDD
	case 0xcc:	D() = through(AM_immediate_word());						break;		// LD (LDD immediate)
	case 0xdc:	D() = through(AM_direct_word());						break;		// LD (LDD direct)
	case 0xec:	D() = through(AM_indexed_word());						break;		// LD (LDD indexed)
	case 0xfc:	D() = through(AM_extended_word());						break;		// LD (LDD extended)

	// LDU
	case 0xce:	U() = through(AM_immediate_word());						break;		// LD (LDU immediate)
	case 0xde:	U() = through(AM_direct_word());						break;		// LD (LDU direct)
	case 0xee:	U() = through(AM_indexed_word());						break;		// LD (LDU indexed)
	case 0xfe:	U() = through(AM_extended_word());						break;		// LD (LDU extended)

	// LDX
	case 0x8e:	X() = through(AM_immediate_word());						break;		// LD (LDX immediate)
	case 0x9e:	X() = through(AM_direct_word());						break;		// LD (LDX direct)
	case 0xae:	X() = through(AM_indexed_word());						break;		// LD (LDX indexed)
	case 0xbe:	X() = through(AM_extended_word());						break;		// LD (LDX extended)

	// LEA
	case 0x30:	adjustZero(X() = Address_indexed());					break;		// LEA (LEAX indexed)
	case 0x31:	adjustZero(Y() = Address_indexed());					break;		// LEA (LEAY indexed)
	case 0x32:	S() = Address_indexed();								break;		// LEA (LEAS indexed)
	case 0x33:	U() = Address_indexed();								break;		// LEA (LEAU indexed)

	// LSR
	case 0x04:	RMW(AM_direct_byte, lsr);								break;		// LSR (direct)
	case 0x44:	memoryRead(); A() = lsr(A());							break;		// LSR (LSRA inherent)
	case 0x54:	memoryRead(); B() = lsr(B());							break;		// LSR (LSRB inherent)
	case 0x64:	RMW(AM_indexed_byte, lsr);								break;		// LSR (indexed)
	case 0x74:	RMW(AM_extended_byte, lsr);								break;		// LSR (extended)

	// MUL
	case 0x3d:	memoryRead(); D() = mul(A(), B()); 						break;		// MUL (inherent)

	// NEG
	case 0x00:	RMW(AM_direct_byte, neg);								break;		// NEG (direct)
	case 0x40:	memoryRead(); A() = neg(A());							break;		// NEG (NEGA, inherent)
	case 0x50:	memoryRead(); B() = neg(B());							break;		// NEG (NEGB, inherent)
	case 0x60:	RMW(AM_indexed_byte, neg);								break;		// NEG (indexed)
	case 0x70:	RMW(AM_extended_byte, neg);								break;		// NEG (extended)

	// NOP
	case 0x12:	memoryRead();											break;		// NOP (inherent)

	// OR

	// ORA
	case 0x8a:	A() = orr(A(), AM_immediate_byte());					break;		// OR (ORA immediate)
	case 0x9a:	A() = orr(A(), AM_direct_byte());						break;		// OR (ORA direct)
	case 0xaa:	A() = orr(A(), AM_indexed_byte());						break;		// OR (ORA indexed)
	case 0xba:	A() = orr(A(), AM_extended_byte());						break;		// OR (ORA extended)

	// ORB
	case 0xca:	B() = orr(B(), AM_immediate_byte());					break;		// OR (ORB immediate)
	case 0xda:	B() = orr(B(), AM_direct_byte());						break;		// OR (ORB direct)
	case 0xea:	B() = orr(B(), AM_indexed_byte());						break;		// OR (ORB indexed)
	case 0xfa:	B() = orr(B(), AM_extended_byte());						break;		// OR (ORB extended)

	// ORCC
	case 0x1a:	CC() |= AM_immediate_byte(); eat();						break;		// OR (ORCC immediate)

	// PSH
	case 0x34:	psh(S(), AM_immediate_byte());							break;		// PSH (PSHS immediate)
	case 0x36:	psh(U(), AM_immediate_byte());							break;		// PSH (PSHU immediate)

	// PUL
	case 0x35:	pul(S(), AM_immediate_byte());							break;		// PUL (PULS immediate)
	case 0x37:	pul(U(), AM_immediate_byte());							break;		// PUL (PULU immediate)

	// ROL
	case 0x09:	RMW(AM_direct_byte, rol);								break;		// ROL (direct)
	case 0x49:	memoryRead(); A() = rol(A());							break;		// ROL (ROLA inherent)
	case 0x59:	memoryRead(); B() = rol(B());							break;		// ROL (ROLB inherent)
	case 0x69:	RMW(AM_indexed_byte, rol);								break;		// ROL (indexed)
	case 0x79:	RMW(AM_extended_byte, rol);								break;		// ROL (extended)

	// ROR
	case 0x06:	RMW(AM_direct_byte, ror);								break;		// ROR (direct)
	case 0x46:	memoryRead(); A() = ror(A());							break;		// ROR (RORA inherent)
	case 0x56:	memoryRead(); B() = ror(B());							break;		// ROR (RORB inherent)
	case 0x66:	RMW(AM_indexed_byte, ror);								break;		// ROR (indexed)
	case 0x76:	RMW(AM_extended_byte, ror);								break;		// ROR (extended)

	// RTI
	case 0x3B:	memoryRead(); rti();									break;		// RTI (inherent)

	// RTS
	case 0x39:	memoryRead(); ret(); 									break;		// RTS (inherent)

	// SBC

	// SBCA
	case 0x82:	A() = sbc(A(), AM_immediate_byte());					break;		// SBC (SBCA immediate)
	case 0x92:	A() = sbc(A(), AM_direct_byte());						break;		// SBC (SBCA direct)
	case 0xa2:	A() = sbc(A(), AM_indexed_byte());						break;		// SBC (SBCA indexed)
	case 0xb2:	A() = sbc(A(), AM_extended_byte());						break;		// SBC (SBCB extended)

	// SBCB
	case 0xc2:	B() = sbc(B(), AM_immediate_byte());					break;		// SBC (SBCB immediate)
	case 0xd2:	B() = sbc(B(), AM_direct_byte());						break;		// SBC (SBCB direct)
	case 0xe2:	B() = sbc(B(), AM_indexed_byte());						break;		// SBC (SBCB indexed)
	case 0xf2:	B() = sbc(B(), AM_extended_byte());						break;		// SBC (SBCB extended)

	// SEX
	case 0x1d:	memoryRead(); A() = sex(B());							break;		// SEX (inherent)

	// ST

	// STA
	case 0x97:	memoryWrite(Address_direct(), through(A()));			break;		// ST (STA direct)
	case 0xa7:	memoryWrite(Address_indexed(), through(A()));			break;		// ST (STA indexed)
	case 0xb7:	memoryWrite(Address_extended(), through(A()));			break;		// ST (STA extended)

	// STB
	case 0xd7:	memoryWrite(Address_direct(), through(B()));			break;		// ST (STB direct)
	case 0xe7:	memoryWrite(Address_indexed(), through(B()));			break;		// ST (STB indexed)
	case 0xf7:	memoryWrite(Address_extended(), through(B()));			break;		// ST (STB extended)

	// STD
	case 0xdd:	Processor::setWord(Address_direct(), through(D()));		break;		// ST (STD direct)
	case 0xed:	Processor::setWord(Address_indexed(), through(D()));	break;		// ST (STD indexed)
	case 0xfd:	Processor::setWord(Address_extended(), through(D()));	break;		// ST (STD extended)

	// STU
	case 0xdf:	Processor::setWord(Address_direct(), through(U()));		break;		// ST (STU direct)
	case 0xef:	Processor::setWord(Address_indexed(), through(U()));	break;		// ST (STU indexed)
	case 0xff:	Processor::setWord(Address_extended(), through(U()));	break;		// ST (STU extended)

	// STX
	case 0x9f:	Processor::setWord(Address_direct(), through(X()));		break;		// ST (STX direct)
	case 0xaf:	Processor::setWord(Address_indexed(), through(X()));	break;		// ST (STX indexed)
	case 0xbf:	Processor::setWord(Address_extended(), through(X()));	break;		// ST (STX extended)

	// SUB

	// SUBA
	case 0x80:	A() = sub(A(), AM_immediate_byte());					break;		// SUB (SUBA immediate)
	case 0x90:	A() = sub(A(), AM_direct_byte());						break;		// SUB (SUBA direct)
	case 0xa0:	A() = sub(A(), AM_indexed_byte());						break;		// SUB (SUBA indexed)
	case 0xb0:	A() = sub(A(), AM_extended_byte());						break;		// SUB (SUBA extended)

	// SUBB
	case 0xc0:	B() = sub(B(), AM_immediate_byte());					break;		// SUB (SUBB immediate)
	case 0xd0:	B() = sub(B(), AM_direct_byte());						break;		// SUB (SUBB direct)
	case 0xe0:	B() = sub(B(), AM_indexed_byte());						break;		// SUB (SUBB indexed)
	case 0xf0:	B() = sub(B(), AM_extended_byte());						break;		// SUB (SUBB extended)

	// SUBD
	case 0x83:	D() = sub(D(), AM_immediate_word());					break;		// SUB (SUBD immediate)
	case 0x93:	D() = sub(D(), AM_direct_word());						break;		// SUB (SUBD direct)
	case 0xa3:	D() = sub(D(), AM_indexed_word());						break;		// SUB (SUBD indexed)
	case 0xb3:	D() = sub(D(), AM_extended_word());						break;		// SUB (SUBD extended)

	// SWI
	case 0x3f:	memoryRead(); swi();									break;		// SWI (inherent) XXXX

	// SYNC
	case 0x13:	memoryRead(); halt();									break;		// SYNC (inherent)XXXX

	// TFR
	case 0x1f:	tfr(AM_immediate_byte());								break;		// TFR (immediate)

	// TST
	case 0x0d:	tst(AM_direct_byte());									break;		// TST (direct)
	case 0x4d:	memoryRead(); tst(A());									break;		// TST (TSTA inherent)
	case 0x5d:	memoryRead(); tst(B()); 								break;		// TST (TSTB inherent)
	case 0x6d:	tst(AM_indexed_byte());									break;		// TST (indexed)
	case 0x7d:	tst(AM_extended_byte());								break;		// TST (extended)

	// Branching

	case 0x16:	jump(Address_relative_word());							break;		// BRA (LBRA relative)
	case 0x17:	jsr(Address_relative_word());							break;		// BSR (LBSR relative)
	case 0x20:	jump(Address_relative_byte());							break;		// BRA (relative)
	case 0x21:	Address_relative_byte();								break;		// BRN (relative)
	case 0x22:	branchShort(HI());										break;		// BHI (relative)
	case 0x23:	branchShort(LS());										break;		// BLS (relative)
	case 0x24:	branchShort(!carry());									break;		// BCC (relative)
	case 0x25:	branchShort(carry());									break;		// BCS (relative)
	case 0x26:	branchShort(!zero());									break;		// BNE (relative)
	case 0x27:	branchShort(zero());									break;		// BEQ (relative)
	case 0x28: 	branchShort(!overflow());								break;		// BVC (relative)
	case 0x29: 	branchShort(overflow());								break;		// BVS (relative)
	case 0x2a: 	branchShort(!negative());								break;		// BPL (relative)
	case 0x2b: 	branchShort(negative());								break;		// BMI (relative)
	case 0x2c:	branchShort(GE());										break;		// BGE (relative)
	case 0x2d:	branchShort(LT());										break;		// BLT (relative)
	case 0x2e:	branchShort(GT());										break;		// BGT (relative)
	case 0x2f:	branchShort(LE());										break;		// BLE (relative)

	case 0x8d:	jsr(Address_relative_byte());							break;		// BSR (relative)

	default:
		UNREACHABLE;
	}
}

void EightBit::mc6809::execute10() {

	assert(m_prefix10 && !m_prefix11);
	assert(cycles() == 2);	// Two fetches

	switch (opcode()) {

	// CMP

	// CMPD
	case 0x83:	cmp(D(), AM_immediate_word());							break;		// CMP (CMPD, immediate)
	case 0x93:	cmp(D(), AM_direct_word());								break;		// CMP (CMPD, direct)
	case 0xa3:	cmp(D(), AM_indexed_word());							break;		// CMP (CMPD, indexed)
	case 0xb3:	cmp(D(), AM_extended_word());							break;		// CMP (CMPD, extended)

	// CMPY
	case 0x8c:	cmp(Y(), AM_immediate_word());							break;		// CMP (CMPY, immediate)
	case 0x9c:	cmp(Y(), AM_direct_word());								break;		// CMP (CMPY, direct)
	case 0xac:	cmp(Y(), AM_indexed_word());							break;		// CMP (CMPY, indexed)
	case 0xbc:	cmp(Y(), AM_extended_word());							break;		// CMP (CMPY, extended)

	// LD

	// LDS
	case 0xce:	S() = through(AM_immediate_word());						break;		// LD (LDS immediate)
	case 0xde:	S() = through(AM_direct_word());						break;		// LD (LDS direct)
	case 0xee:	S() = through(AM_indexed_word());						break;		// LD (LDS indexed)
	case 0xfe:	S() = through(AM_extended_word());						break;		// LD (LDS extended)

	// LDY
	case 0x8e:	Y() = through(AM_immediate_word());						break;		// LD (LDY immediate)
	case 0x9e:	Y() = through(AM_direct_word());						break;		// LD (LDY direct)
	case 0xae:	Y() = through(AM_indexed_word());						break;		// LD (LDY indexed)
	case 0xbe:	Y() = through(AM_extended_word());						break;		// LD (LDY extended)

	// Branching

	case 0x21:	Address_relative_word();								break;		// BRN (LBRN relative)
	case 0x22:	branchLong(HI());										break;		// BHI (LBHI relative)
	case 0x23:	branchLong(LS());										break;		// BLS (LBLS relative)
	case 0x24:	branchLong(!carry());									break;		// BCC (LBCC relative)
	case 0x25:	branchLong(carry());									break;		// BCS (LBCS relative)
	case 0x26:	branchLong(!zero());									break;		// BNE (LBNE relative)
	case 0x27:	branchLong(zero());										break;		// BEQ (LBEQ relative)
	case 0x28:	branchLong(!overflow());								break;		// BVC (LBVC relative)
	case 0x29:	branchLong(overflow());									break;		// BVS (LBVS relative)
	case 0x2a:	branchLong(!negative());								break;		// BPL (LBPL relative)
	case 0x2b: 	branchLong(negative());									break;		// BMI (LBMI relative)
	case 0x2c:	branchLong(GE());										break;		// BGE (LBGE relative)
	case 0x2d:	branchLong(LT());										break;		// BLT (LBLT relative)
	case 0x2e:	branchLong(GT());										break;		// BGT (LBGT relative)
	case 0x2f:	branchLong(LE());										break;		// BLE (LBLE relative)

	// STS
	case 0xdf:	Processor::setWord(Address_direct(), through(S()));		break;		// ST (STS direct)
	case 0xef:	Processor::setWord(Address_indexed(), through(S()));	break;		// ST (STS indexed)
	case 0xff:	Processor::setWord(Address_extended(), through(S()));	break;		// ST (STS extended)

	// STY
	case 0x9f:	Processor::setWord(Address_direct(), through(Y()));		break;		// ST (STY direct)
	case 0xaf:	Processor::setWord(Address_indexed(), through(Y()));	break;		// ST (STY indexed)
	case 0xbf:	Processor::setWord(Address_extended(), through(Y()));	break;		// ST (STY extended)

	// SWI
	case 0x3f:	memoryRead(); swi2();									break;		// SWI (SWI2 inherent)

	default:
		UNREACHABLE;
	}
}

void EightBit::mc6809::execute11() {

	assert(!m_prefix10 && m_prefix11);
	assert(cycles() == 2);	// Two fetches

	switch (opcode()) {

	// CMP

	// CMPU
	case 0x83:	cmp(U(), AM_immediate_word());							break;		// CMP (CMPU, immediate)
	case 0x93:	cmp(U(), AM_direct_word());								break;		// CMP (CMPU, direct)
	case 0xa3:	cmp(U(), AM_indexed_word());							break;		// CMP (CMPU, indexed)
	case 0xb3:	cmp(U(), AM_extended_word());							break;		// CMP (CMPU, extended)

	// CMPS
	case 0x8c:	cmp(S(), AM_immediate_word());							break;		// CMP (CMPS, immediate)
	case 0x9c:	cmp(S(), AM_direct_word());								break;		// CMP (CMPS, direct)
	case 0xac:	cmp(S(), AM_indexed_word());							break;		// CMP (CMPS, indexed)
	case 0xbc:	cmp(S(), AM_extended_word());							break;		// CMP (CMPS, extended)

	// SWI
	case 0x3f:	memoryRead(); swi3();									break;		// SWI (SWI3 inherent)

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
	memoryWrite(--stack, value);
}

uint8_t EightBit::mc6809::pop(register16_t& stack) {
	return memoryRead(stack++);
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
	const auto address = PC() + (int8_t)fetchByte();
	eat();
	return address;
}

EightBit::register16_t EightBit::mc6809::Address_relative_word() {
	return PC() + (int16_t)fetchWord().word;
}

EightBit::register16_t EightBit::mc6809::Address_direct() {
	const auto offset = fetchByte();
	eat();
	return register16_t(offset, DP());
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
			address = r++;
			memoryRead(PC());
			eat(2);
			break;
		case 0b0001:	// ,R++
			address = r;
			r += 2;
			memoryRead(PC());
			eat(3);
			break;
		case 0b0010:	// ,-R
			ASSUME(!indirect);
			address = --r;
			memoryRead(PC());
			eat(2);
			break;
		case 0b0011:	// ,--R
			r -= 2;
			address = r;
			memoryRead(PC());
			eat(3);
			break;
		case 0b0100:	// ,R
			address = r;
			memoryRead();
			break;
		case 0b0101:	// B,R
			address = r + (int8_t)B();
			memoryRead(PC());
			eat();
			break;
		case 0b0110:	// A,R
			address = r + (int8_t)A();
			memoryRead(PC());
			eat();
			break;
		case 0b1000:	// n,R (eight-bit)
			address = r + (int8_t)fetchByte();
			eat();
			break;
		case 0b1001:	// n,R (sixteen-bit)
			address = r + (int16_t)fetchWord().word;
			memoryRead(PC());
			eat(2);
			break;
		case 0b1011:	// D,R
			address = r + D();
			memoryRead(PC());
			memoryRead(PC() + 1);
			memoryRead(PC() + 2);
			eat(2);
			break;
		case 0b1100:	// n,PCR (eight-bit)
			address = Address_relative_byte();
			break;
		case 0b1101:	// n,PCR (sixteen-bit)
			address = Address_relative_word();
			memoryRead(PC());
			eat(3);
			break;
		case 0b1111:	// [n]
			assert(indirect);
			address = Address_extended();
			memoryRead(PC());
			break;
		default:
			UNREACHABLE;
		}
		if (indirect) {
			address = Processor::getWord(address);
			eat();
		}
	} else {
		// EA = ,R + 5-bit offset
		address = r + signExtend(5, type & Mask5);
		memoryRead(PC());
		eat();
	}
	return address;
}

EightBit::register16_t EightBit::mc6809::Address_extended() {
	const auto ea = fetchWord();
	eat();
	return ea;
}

//

uint8_t EightBit::mc6809::AM_immediate_byte() {
	return fetchByte();
}

uint8_t EightBit::mc6809::AM_direct_byte() {
	return memoryRead(Address_direct());
}

uint8_t EightBit::mc6809::AM_indexed_byte() {
	return memoryRead(Address_indexed());
}

uint8_t EightBit::mc6809::AM_extended_byte() {
	return memoryRead(Address_extended());
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
	CC() = setBit(CC(), EF);
	saveRegisterState();
}

void EightBit::mc6809::savePartialRegisterState() {
	CC() = clearBit(CC(), EF);
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
	eat();
	return addition & Mask16;
}

uint8_t EightBit::mc6809::andr(const uint8_t operand, const uint8_t data) {
	return through((uint8_t)(operand & data));
}

uint8_t EightBit::mc6809::asl(uint8_t operand) {
	CC() = setBit(CC(), CF, operand & Bit7);
	adjustNZ(operand <<= 1);
	const auto overflow = carry() ^ (negative() >> 3);
	CC() = setBit(CC(), VF, overflow);
	return operand;
}

uint8_t EightBit::mc6809::asr(uint8_t operand) {
	CC() = setBit(CC(), CF, operand & Bit0);
	const uint8_t result = (operand >> 1) | Bit7;
	adjustNZ(result);
	return result;
}

void EightBit::mc6809::bit(const uint8_t operand, const uint8_t data) {
	andr(operand, data);
}

uint8_t EightBit::mc6809::clr(uint8_t) {
	CC() = clearBit(CC(), CF);
	return through((uint8_t)0U);
}

void EightBit::mc6809::cmp(const uint8_t operand, const uint8_t data) {
	sub(operand, data);
}

void EightBit::mc6809::cmp(const register16_t operand, const register16_t data) {
	sub(operand, data);
}

uint8_t EightBit::mc6809::com(const uint8_t operand) {
	CC() = setBit(CC(), CF);
	return through((uint8_t)~operand);
}

void EightBit::mc6809::cwai(const uint8_t data) {
	CC() &= data;
	memoryRead(PC());
	eat();
	saveEntireRegisterState();
	halt();
}

uint8_t EightBit::mc6809::da(uint8_t operand) {

	CC() = setBit(CC(), CF, operand > 0x99);

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

uint8_t EightBit::mc6809::eorr(const uint8_t operand, const uint8_t data) noexcept {
	return through((uint8_t)(operand ^ data));
}

uint8_t& EightBit::mc6809::referenceTransfer8(const int specifier) noexcept {
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

	eat(6);
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
	CC() = setBit(CC(), CF, operand & Bit0);
	adjustNZ(operand >>= 1);
	return operand;
}

EightBit::register16_t EightBit::mc6809::mul(const uint8_t first, const uint8_t second) {
	const register16_t result = first * second;
	adjustZero(result);
	CC() = setBit(CC(), CF, result.low & Bit7);
	eat(9);
	return result;
}

uint8_t EightBit::mc6809::neg(uint8_t operand) {
	CC() = setBit(CC(), VF, operand == Bit7);
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
	eat(2);
	memoryRead(stack);
	if (data & Bit7)
		pushWord(stack, PC());
	if (data & Bit6)
		// Pushing to the S stack means we must be pushing U
		pushWord(stack, &stack == &S() ? U() : S());
	if (data & Bit5)
		pushWord(stack, Y());
	if (data & Bit4)
		pushWord(stack, X());
	if (data & Bit3)
		push(stack, DP());
	if (data & Bit2)
		push(stack, B());
	if (data & Bit1)
		push(stack, A());
	if (data & Bit0)
		push(stack, CC());
}

void EightBit::mc6809::pul(register16_t& stack, const uint8_t data) {
	eat(2);
	if (data & Bit0)
		CC() = pop(stack);
	if (data & Bit1)
		A() = pop(stack);
	if (data & Bit2)
		B() = pop(stack);
	if (data & Bit3)
		DP() = pop(stack);
	if (data & Bit4) 
		X() = popWord(stack);
	if (data & Bit5)
		Y() = popWord(stack);
	if (data & Bit6)
		// Pulling from the S stack means we must be pulling U
		(&stack == &S() ? U() : S()) = popWord(stack);
	if (data & Bit7)
		PC() = popWord(stack);
	memoryRead(stack);
}

uint8_t EightBit::mc6809::rol(const uint8_t operand) {
	const auto carryIn = carry();
	CC() = setBit(CC(), CF, operand & Bit7);
	CC() = setBit(CC(), VF, ((operand & Bit7) >> 7) ^ ((operand & Bit6) >> 6));
	const uint8_t result = (operand << 1) | carryIn;
	adjustNZ(result);
	return result;
}
		
uint8_t EightBit::mc6809::ror(const uint8_t operand) {
	const auto carryIn = carry();
	CC() = setBit(CC(), CF, operand & Bit0);
	const uint8_t result = (operand >> 1) | (carryIn << 7);
	adjustNZ(result);
	return result;
}

void EightBit::mc6809::rti() {
	restoreRegisterState();
	eat();
}

void EightBit::mc6809::swi() {
	eat();
	saveEntireRegisterState();
	CC() = setBit(CC(), IF);	// Disable IRQ
	CC() = setBit(CC(), FF);	// Disable FIRQ
	jump(getWordPaged(0xff, SWIvector));
	eat();
}

void EightBit::mc6809::swi2() {
	eat();
	saveEntireRegisterState();
	jump(getWordPaged(0xff, SWI2vector));
	eat();
}

void EightBit::mc6809::swi3() {
	eat();
	saveEntireRegisterState();
	jump(getWordPaged(0xff, SWI3vector));
	eat();
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

	eat(4);
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
	eat();
	return subtraction & Mask16;
}

void EightBit::mc6809::tst(const uint8_t data) {
	cmp(data, 0);
}

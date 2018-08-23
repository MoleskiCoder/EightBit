#include "stdafx.h"
#include "mc6809.h"

#include <algorithm>

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
	case 0x3a:	addCycles(3);	X() += B();								break;		// ABX (inherent)

	// ADC
	case 0x89:	addCycles(2);	A() = adc(A(), AM_immediate_byte());	break;		// ADC (ADCA immediate)
	case 0x99:	addCycles(4);	A() = adc(A(), AM_direct_byte());		break;		// ADC (ADCA direct)
	case 0xa9:	addCycles(4);	A() = adc(A(), AM_indexed_byte());		break;		// ADC (ADCA indexed)
	case 0xb9:	addCycles(4);	A() = adc(A(), AM_extended_byte());		break;		// ADC (ADCA extended)

	case 0xc9:	addCycles(2);	B() = adc(B(), AM_immediate_byte());	break;		// ADC (ADCB immediate)
	case 0xd9:	addCycles(4);	B() = adc(B(), AM_direct_byte());		break;		// ADC (ADCB direct)
	case 0xe9:	addCycles(4);	B() = adc(B(), AM_indexed_byte());		break;		// ADC (ADCB indexed)
	case 0xf9:	addCycles(4);	B() = adc(B(), AM_extended_byte());		break;		// ADC (ADCB extended)

	// ADD
	case 0x8b: addCycles(2);	A() = add(A(), AM_immediate_byte());	break;		// ADD (ADDA immediate)
	case 0x9b: addCycles(4);	A() = add(A(), AM_direct_byte());		break;		// ADD (ADDA direct)
	case 0xab: addCycles(4);	A() = add(A(), AM_indexed_byte());		break;		// ADD (ADDA indexed)
	case 0xbb: addCycles(5);	A() = add(A(), AM_extended_byte());		break;		// ADD (ADDA extended)

	case 0xcb: addCycles(2);	B() = add(B(), AM_immediate_byte());	break;		// ADD (ADDB immediate)
	case 0xdb: addCycles(4);	B() = add(B(), AM_direct_byte());		break;		// ADD (ADDB direct)
	case 0xeb: addCycles(4);	B() = add(B(), AM_indexed_byte());		break;		// ADD (ADDB indexed)
	case 0xfb: addCycles(5);	B() = add(B(), AM_extended_byte());		break;		// ADD (ADDB extended)

	case 0xc3: addCycles(4);	D() = add(D(), AM_immediate_word());	break;		// ADD (ADDD immediate)
	case 0xd3: addCycles(6);	D() = add(D(), AM_direct_word());		break;		// ADD (ADDD direct)
	case 0xe3: addCycles(6);	D() = add(D(), AM_indexed_word());		break;		// ADD (ADDD indexed)
	case 0xf3: addCycles(7);	D() = add(D(), AM_extended_word());		break;		// ADD (ADDD extended)

	// AND
	case 0x84:	addCycles(2);	A() = andr(A(), AM_immediate_byte());	break;		// AND (ANDA immediate)
	case 0x94:	addCycles(4);	A() = andr(A(), AM_direct_byte());		break;		// AND (ANDA direct)
	case 0xa4:	addCycles(4);	A() = andr(A(), AM_indexed_byte());		break;		// AND (ANDA indexed)
	case 0xb4:	addCycles(5);	A() = andr(A(), AM_extended_byte());	break;		// AND (ANDA extended)

	case 0xc4:	addCycles(2);	B() = andr(B(), AM_immediate_byte());	break;		// AND (ANDB immediate)
	case 0xd4:	addCycles(4);	B() = andr(B(), AM_direct_byte());		break;		// AND (ANDB direct)
	case 0xe4:	addCycles(4);	B() = andr(B(), AM_indexed_byte());		break;		// AND (ANDB indexed)
	case 0xf4:	addCycles(5);	B() = andr(B(), AM_extended_byte());	break;		// AND (ANDB extended)

	case 0x1c:	addCycles(3);	CC() &= AM_immediate_byte();			break;		// AND (ANDCC immediate)

	// ASL/LSL
	case 0x08:	addCycles(6);	BUS().write(asl(AM_direct_byte()));		break;		// ASL (direct)
	case 0x48:	addCycles(2);	A() = asl(A());							break;		// ASL (ASLA inherent)
	case 0x58:	addCycles(2);	B() = asl(B());							break;		// ASL (ASLB inherent)
	case 0x68:	addCycles(6);	BUS().write(asl(AM_indexed_byte()));	break;		// ASL (indexed)
	case 0x78:	addCycles(7);	BUS().write(asl(AM_extended_byte()));	break;		// ASL (extended)

	// ASR
	case 0x07:	addCycles(6);	BUS().write(asr(AM_direct_byte()));		break;		// ASR (direct)
	case 0x47:	addCycles(2);	A() = asr(A());							break;		// ASR (ASRA inherent)
	case 0x57:	addCycles(2);	B() = asr(B());							break;		// ASR (ASRB inherent)
	case 0x67:	addCycles(6);	BUS().write(asr(AM_indexed_byte()));	break;		// ASR (indexed)
	case 0x77:	addCycles(7);	BUS().write(asr(AM_extended_byte()));	break;		// ASR (extended)

	// BIT
	case 0x85:	addCycles(2);	andr(A(), AM_immediate_byte());			break;		// BIT (BITA immediate)
	case 0x95:	addCycles(4);	andr(A(), AM_direct_byte());			break;		// BIT (BITA direct)
	case 0xa5:	addCycles(4);	andr(A(), AM_indexed_byte());			break;		// BIT (BITA indexed)
	case 0xb5:	addCycles(5);	andr(A(), AM_extended_byte());			break;		// BIT (BITA extended)

	case 0xc5:	addCycles(2);	andr(B(), AM_immediate_byte());			break;		// BIT (BITB immediate)
	case 0xd5:	addCycles(4);	andr(B(), AM_direct_byte());			break;		// BIT (BITB direct)
	case 0xe5:	addCycles(4);	andr(B(), AM_indexed_byte());			break;		// BIT (BITB indexed)
	case 0xf5:	addCycles(5);	andr(B(), AM_extended_byte());			break;		// BIT (BITB extended)

	// CLR
	case 0x0f:	addCycles(6);	BUS().write(Address_direct(), clr());	break;		// CLR (direct)
	case 0x4f:	addCycles(2);	A() = clr();							break;		// CLR (CLRA implied)
	case 0x5f:	addCycles(2);	B() = clr();							break;		// CLR (CLRB implied)
	case 0x6f:	addCycles(6);	BUS().write(Address_indexed(), clr());	break;		// CLR (indexed)
	case 0x7f:	addCycles(7);	BUS().write(Address_extended(), clr());	break;		// CLR (extended)

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

	// COM
	case 0x03:	addCycles(6);	BUS().write(com(AM_direct_byte()));		break;		// COM (direct)
	case 0x43:	addCycles(2);	A() = com(A());							break;		// COM (COMA inherent)
	case 0x53:	addCycles(2);	B() = com(B());							break;		// COM (COMB inherent)
	case 0x63:	addCycles(6);	BUS().write(com(AM_indexed_byte()));	break;		// COM (indexed)
	case 0x73:	addCycles(7);	BUS().write(com(AM_extended_byte()));	break;		// COM (extended)

	// CWAI
	case 0x3c:	addCycles(20);	cwai(AM_direct_byte());					break;		// CWAI (direct)

	// DAA
	case 0x19:	addCycles(2);	A() = da(A());							break;		// DAA (inherent)

	// DEC
	case 0x0a:	addCycles(6);	BUS().write(dec(AM_direct_byte()));		break;		// DEC (direct)
	case 0x4a:	addCycles(2);	A() = dec(A());							break;		// DEC (DECA inherent)
	case 0x5a:	addCycles(2);	B() = dec(B());							break;		// DEC (DECB inherent)
	case 0x6a:	addCycles(6);	BUS().write(dec(AM_indexed_byte()));	break;		// DEC (indexed)
	case 0x7a:	addCycles(7);	BUS().write(dec(AM_extended_byte()));	break;		// DEC (extended)

	// EOR

	// EORA
	case 0x88:	addCycles(2);	A() = eor(A(), AM_immediate_byte());	break;		// EOR (EORA immediate)
	case 0x98:	addCycles(4);	A() = eor(A(), AM_direct_byte());		break;		// EOR (EORA direct)
	case 0xa8:	addCycles(4);	A() = eor(A(), AM_indexed_byte());		break;		// EOR (EORA indexed)
	case 0xb8:	addCycles(5);	A() = eor(A(), AM_extended_byte());		break;		// EOR (EORA extended)

	// EORB
	case 0xc8:	addCycles(2);	B() = eor(B(), AM_immediate_byte());	break;		// EOR (EORB immediate)
	case 0xd8:	addCycles(4);	B() = eor(B(), AM_direct_byte());		break;		// EOR (EORB direct)
	case 0xe8:	addCycles(4);	B() = eor(B(), AM_indexed_byte());		break;		// EOR (EORB indexed)
	case 0xf8:	addCycles(5);	B() = eor(B(), AM_extended_byte());		break;		// EOR (EORB extended)

	// EXG
	case 0x1e:	addCycles(8);	exg(AM_immediate_byte());				break;		// EXG (R1,R2 immediate)

	// INC
	case 0x0c:	addCycles(6);	BUS().write(inc(AM_direct_byte()));		break;		// INC (direct)
	case 0x4c:	addCycles(2);	A() = inc(A());							break;		// INC (INCA inherent)
	case 0x5c:	addCycles(2);	B() = inc(B());							break;		// INC (INCB inherent)
	case 0x6c:	addCycles(6);	BUS().write(inc(AM_indexed_byte()));	break;		// INC (indexed)
	case 0x7c:	addCycles(7);	BUS().write(inc(AM_extended_byte()));	break;		// INC (extended)

	// JMP
	case 0x0e:	addCycles(6);	jump(Address_direct());					break;		// JMP (direct)
	case 0x6e:	addCycles(6);	jump(Address_indexed());				break;		// JMP (indexed)
	case 0x7e:	addCycles(7);	jump(Address_extended());				break;		// JMP (extended)

	// JSR
	case 0x9d:	addCycles(6);	call(Address_direct());					break;		// JSR (direct)
	case 0xad:	addCycles(6);	call(Address_indexed());				break;		// JSR (indexed)
	case 0xbd:	addCycles(7);	call(Address_extended());				break;		// JSR (extended)

	// LD

	// LDA
	case 0x86:	addCycles(2);	A() = ld(AM_immediate_byte());			break;		// LD (LDA immediate)
	case 0x96:	addCycles(4);	A() = ld(AM_direct_byte());				break;		// LD (LDA direct)
	case 0xa6:	addCycles(4);	A() = ld(AM_indexed_byte());			break;		// LD (LDA indexed)
	case 0xb6:	addCycles(5);	A() = ld(AM_extended_byte());			break;		// LD (LDA extended)

	// LDB
	case 0xc6:	addCycles(2);	B() = ld(AM_immediate_byte());			break;		// LD (LDB immediate)
	case 0xd6:	addCycles(4);	B() = ld(AM_direct_byte());				break;		// LD (LDB direct)
	case 0xe6:	addCycles(4);	B() = ld(AM_indexed_byte());			break;		// LD (LDB indexed)
	case 0xf6:	addCycles(5);	B() = ld(AM_extended_byte());			break;		// LD (LDB extended)

	// LDD
	case 0xcc:	addCycles(3);	D() = ld(AM_immediate_word());			break;		// LD (LDD immediate)
	case 0xdc:	addCycles(5);	D() = ld(AM_direct_word());				break;		// LD (LDD direct)
	case 0xec:	addCycles(5);	D() = ld(AM_indexed_word());			break;		// LD (LDD indexed)
	case 0xfc:	addCycles(6);	D() = ld(AM_extended_word());			break;		// LD (LDD extended)

	// LDU
	case 0xce:	addCycles(3);	U() = ld(AM_immediate_word());			break;		// LD (LDU immediate)
	case 0xde:	addCycles(5);	U() = ld(AM_direct_word());				break;		// LD (LDU direct)
	case 0xee:	addCycles(5);	U() = ld(AM_indexed_word());			break;		// LD (LDU indexed)
	case 0xfe:	addCycles(6);	U() = ld(AM_extended_word());			break;		// LD (LDU extended)

	// LDX
	case 0x8e:	addCycles(3);	X() = ld(AM_immediate_word());			break;		// LD (LDX immediate)
	case 0x9e:	addCycles(5);	X() = ld(AM_direct_word());				break;		// LD (LDX direct)
	case 0xae:	addCycles(5);	X() = ld(AM_indexed_word());			break;		// LD (LDX indexed)
	case 0xbe:	addCycles(6);	X() = ld(AM_extended_word());			break;		// LD (LDX extended)

	// LEA
	case 0x30:	addCycles(4);	adjustZero(X() = Address_indexed());	break;		// LEA (LEAX indexed)
	case 0x31:	addCycles(4);	adjustZero(Y() = Address_indexed());	break;		// LEA (LEAY indexed)
	case 0x32:	addCycles(4);	S() = Address_indexed();				break;		// LEA (LEAS indexed)
	case 0x33:	addCycles(4);	U() = Address_indexed();				break;		// LEA (LEAU indexed)

	// LSR
	case 0x04:	addCycles(6);	BUS().write(lsr(AM_direct_byte()));		break;		// LSR (direct)
	case 0x44:	addCycles(2);	A() = lsr(A());							break;		// LSR (LSRA inherent)
	case 0x54:	addCycles(2);	B() = lsr(B());							break;		// LSR (LSRB inherent)
	case 0x64:	addCycles(6);	BUS().write(lsr(AM_indexed_byte()));	break;		// LSR (indexed)
	case 0x74:	addCycles(7);	BUS().write(lsr(AM_extended_byte()));	break;		// LSR (extended)

	// MUL
	case 0x3d:	addCycles(11);	D() = mul(A(), B());					break;		// MUL (inherent)

	// NEG
	case 0x00:	addCycles(6);	BUS().write(neg(AM_direct_byte()));		break;		// NEG (direct)
	case 0x40:	addCycles(2);	A() = neg(A());							break;		// NEG (NEGA, inherent)
	case 0x50:	addCycles(2);	B() = neg(B());							break;		// NEG (NEGB, inherent)
	case 0x60:	addCycles(6);	BUS().write(neg(AM_indexed_byte()));	break;		// NEG (indexed)
	case 0x70:	addCycles(7);	BUS().write(neg(AM_extended_byte()));	break;		// NEG (extended)

	// NOP
	case 0x12:	addCycles(2);											break;		// NOP (inherent)

	// OR

	// ORA
	case 0x8a:	addCycles(2);	A() = orr(A(), AM_immediate_byte());	break;		// OR (ORA immediate)
	case 0x9a:	addCycles(4);	A() = orr(A(), AM_direct_byte());		break;		// OR (ORA direct)
	case 0xaa:	addCycles(4);	A() = orr(A(), AM_indexed_byte());		break;		// OR (ORA indexed)
	case 0xba:	addCycles(5);	A() = orr(A(), AM_extended_byte());		break;		// OR (ORA extended)

	// ORB
	case 0xca:	addCycles(2);	A() = orr(A(), AM_immediate_byte());	break;		// OR (ORB immediate)
	case 0xda:	addCycles(4);	A() = orr(A(), AM_direct_byte());		break;		// OR (ORB direct)
	case 0xea:	addCycles(4);	A() = orr(A(), AM_indexed_byte());		break;		// OR (ORB indexed)
	case 0xfa:	addCycles(5);	A() = orr(A(), AM_extended_byte());		break;		// OR (ORB extended)

	// ORCC
	case 0x1a:	addCycles(3);	CC() |= AM_immediate_byte();			break;		// OR (ORCC immediate)

	// PSH
	case 0x34:	addCycles(5);	pshs(AM_immediate_byte());				break;		// PSH (PSHS immediate)
	case 0x36:	addCycles(5);	pshu(AM_immediate_byte());				break;		// PSH (PSHU immediate)

	// PUL
	case 0x35:	addCycles(5);	puls(AM_immediate_byte());				break;		// PUL (PULS immediate)
	case 0x37:	addCycles(5);	pulu(AM_immediate_byte());				break;		// PUL (PULU immediate)

	// ROL
	case 0x09:	addCycles(6);	BUS().write(rol(AM_direct_byte()));		break;		// ROL (direct)
	case 0x49:	addCycles(2);	A() = rol(A());							break;		// ROL (ROLA inherent)
	case 0x59:	addCycles(2);	B() = rol(B());							break;		// ROL (ROLB inherent)
	case 0x69:	addCycles(6);	BUS().write(rol(AM_indexed_byte()));	break;		// ROL (indexed)
	case 0x79:	addCycles(7);	BUS().write(rol(AM_extended_byte()));	break;		// ROL (extended)

	// ROR
	case 0x06:	addCycles(6);	BUS().write(ror(AM_direct_byte()));		break;		// ROR (direct)
	case 0x46:	addCycles(2);	A() = ror(A());							break;		// ROR (RORA inherent)
	case 0x56:	addCycles(2);	B() = ror(B());							break;		// ROR (RORB inherent)
	case 0x66:	addCycles(6);	BUS().write(ror(AM_indexed_byte()));	break;		// ROR (indexed)
	case 0x76:	addCycles(7);	BUS().write(ror(AM_extended_byte()));	break;		// ROR (extended)

	// RTI
	case 0x3B:	addCycles(6);	rti();									break;		// RTI (inherent)

	// RTS
	case 0x39:	addCycles(5);	rts();									break;		// RTS (inherent)

	// SBC

	// SBCA
	case 0x82:	addCycles(4);	break;		// SBC (SBCA immediate)
	case 0x92:	addCycles(4);	break;		// SBC (SBCA direct)
	case 0xa2:	addCycles(4);	break;		// SBC (SBCA indexed)
	case 0xb2:	addCycles(5);	break;		// SBC (SBCB extended)

	// SBCB
	case 0xc2:	addCycles(4);	break;		// SBC (SBCB immediate)
	case 0xd2:	addCycles(4);	break;		// SBC (SBCB direct)
	case 0xe2:	addCycles(4);	break;		// SBC (SBCB indexed)
	case 0xf2:	addCycles(5);	break;		// SBC (SBCB extended)

	// SEX
	case 0x1d:	addCycles(2);	A() = sex(B());							break;		// SEX (inherent)

	// ST

	// STA
	case 0x97:	addCycles(4);	break;		// ST (STA direct)
	case 0xa7:	addCycles(4);	break;		// ST (STA indexed)
	case 0xb7:	addCycles(5);	break;		// ST (STA extended)

	// STB
	case 0xd7:	addCycles(4);	break;		// ST (STB direct)
	case 0xe7:	addCycles(4);	break;		// ST (STB indexed)
	case 0xf7:	addCycles(5);	break;		// ST (STB extended)

	// STD
	case 0xdd:	addCycles(5);	break;		// ST (STB direct)
	case 0xed:	addCycles(5);	break;		// ST (STB indexed)
	case 0xfd:	addCycles(6);	break;		// ST (STB extended)

	// STU
	case 0xdf:	addCycles(5);	break;		// ST (STU direct)
	case 0xef:	addCycles(5);	break;		// ST (STU indexed)
	case 0xff:	addCycles(6);	break;		// ST (STU extended)

	// STX
	case 0x9f:	addCycles(5);	break;		// ST (STX direct)
	case 0xaf:	addCycles(5);	break;		// ST (STX indexed)
	case 0xbf:	addCycles(6);	break;		// ST (STX extended)

	// SUB

	// SUBA
	case 0x80:	addCycles(2);	break;		// SUB (SUBA immediate)
	case 0x90:	addCycles(4);	break;		// SUB (SUBA direct)
	case 0xa0:	addCycles(4);	break;		// SUB (SUBA indexed)
	case 0xb0:	addCycles(5);	break;		// SUB (SUBA extended)

	// SUBB
	case 0xc0:	addCycles(2);	break;		// SUB (SUBB immediate)
	case 0xd0:	addCycles(4);	break;		// SUB (SUBB direct)
	case 0xe0:	addCycles(4);	break;		// SUB (SUBB indexed)
	case 0xf0:	addCycles(5);	break;		// SUB (SUBB extended)

	// SUBD
	case 0x83:	addCycles(4);	break;		// SUB (SUBD immediate)
	case 0x93:	addCycles(6);	break;		// SUB (SUBD direct)
	case 0xa3:	addCycles(6);	break;		// SUB (SUBD indexed)
	case 0xb3:	addCycles(7);	break;		// SUB (SUBD extended)

	// SWI
	case 0x3f:	addCycles(19);	swi();									break;		// SWI (inherent)

	// SYNC
	case 0x13:	addCycles(4);	halt();									break;		// SYNC (inherent)

	// TFR
	case 0x1f:	addCycles(6);	tfr(AM_immediate_byte());				break;		// TFR (immediate)

	// TST
	case 0x0d:	addCycles(6);	break;		// TST (direct)
	case 0x4d:	addCycles(2);	break;		// TST (TSTA inherent)
	case 0x5d:	addCycles(2);	break;		// TST (TSTB inherent)
	case 0x6d:	addCycles(6);	break;		// TST (indexed)
	case 0x7d:	addCycles(7);	break;		// TST (extended)

	// Branching

	case 0x16:	addCycles(5);	jump(Address_relative_word());			break;		// BRA (LBRA relative)
	case 0x17:	addCycles(9);	call(Address_relative_word());			break;		// BSR (LBSR relative)
	case 0x20:	addCycles(3);	jump(Address_relative_byte());			break;		// BRA (relative)
	case 0x21:	addCycles(3);	Address_relative_byte();				break;		// BRN (relative)
	case 0x22:	addCycles(3);	branchShort(BHI());						break;		// BHI (relative)
	case 0x23:	addCycles(3);	branchShort(BLS());						break;		// BLS (relative)
	case 0x24:	addCycles(3);	branchShort(!carry());					break;		// BCC (relative)
	case 0x25:	addCycles(3);	branchShort(carry());					break;		// BCS (relative)
	case 0x26:	addCycles(3);	branchShort(!zero());					break;		// BNE (relative)
	case 0x27:	addCycles(3);	branchShort(zero());					break;		// BEQ (relative)
	case 0x28: 	addCycles(3);	branchShort(!overflow());				break;		// BVC (relative)
	case 0x29: 	addCycles(3);	branchShort(overflow());				break;		// BVS (relative)
	case 0x2a: 	addCycles(3);	branchShort(!negative());				break;		// BPL (relative)
	case 0x2b: 	addCycles(3);	branchShort(negative());				break;		// BMI (relative)
	case 0x2c:	addCycles(3);	branchShort(BGE());						break;		// BGE (relative)
	case 0x2d:	addCycles(3);	branchShort(BLT());						break;		// BLT (relative)
	case 0x2e:	addCycles(3);	branchShort(BGT());						break;		// BGT (relative)
	case 0x2f:	addCycles(3);	branchShort(BLE());						break;		// BLE (relative)

	case 0x8d:	addCycles(7);	call(Address_relative_byte());			break;		// BSR (relative)

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

	// LD

	// LDS
	case 0xce:	addCycles(4);	S() = ld(AM_immediate_word());			break;		// LD (LDS immediate)
	case 0xde:	addCycles(6);	S() = ld(AM_direct_word());				break;		// LD (LDS direct)
	case 0xee:	addCycles(6);	S() = ld(AM_indexed_word());			break;		// LD (LDS indexed)
	case 0xfe:	addCycles(7);	S() = ld(AM_extended_word());			break;		// LD (LDS extended)

	// LDY
	case 0x8e:	addCycles(4);	Y() = ld(AM_immediate_word());			break;		// LD (LDY immediate)
	case 0x9e:	addCycles(6);	Y() = ld(AM_direct_word());				break;		// LD (LDY direct)
	case 0xae:	addCycles(6);	Y() = ld(AM_indexed_word());			break;		// LD (LDY indexed)
	case 0xbe:	addCycles(7);	Y() = ld(AM_extended_word());			break;		// LD (LDY extended)

	// Branching

	case 0x21:	addCycles(5);	Address_relative_word();				break;		// BRN (LBRN relative)
	case 0x22:	addCycles(5);	if (branchLong(BHI())) addCycle();		break;		// BHI (LBHI relative)
	case 0x23:	addCycles(5);	if (branchLong(BLS())) addCycle();		break;		// BLS (LBLS relative)
	case 0x24:	addCycles(5);	if (branchLong(!carry())) addCycle();	break;		// BCC (LBCC relative)
	case 0x25:	addCycles(5);	if (branchLong(carry())) addCycle();	break;		// BCS (LBCS relative)
	case 0x26:	addCycles(5);	if (branchLong(!zero())) addCycle();	break;		// BNE (LBNE relative)
	case 0x27:	addCycles(5);	if (branchLong(zero())) addCycle();		break;		// BEQ (LBEQ relative)
	case 0x28:	addCycles(5);	if (branchLong(!overflow())) addCycle();break;		// BVC (LBVC relative)
	case 0x29:	addCycles(5);	if (branchLong(overflow())) addCycle();	break;		// BVS (LBVS relative)
	case 0x2a:	addCycles(5);	if (branchLong(!negative())) addCycle();break;		// BPL (LBPL relative)
	case 0x2b: 	addCycles(5);	if (branchLong(negative())) addCycle();	break;		// BMI (LBMI relative)
	case 0x2c:	addCycles(5);	if (branchLong(BGE())) addCycle();		break;		// BGE (LBGE relative)
	case 0x2d:	addCycles(5);	if (branchLong(BLT())) addCycle();		break;		// BLT (LBLT relative)
	case 0x2e:	addCycles(5);	if (branchLong(BGT())) addCycle();		break;		// BGT (LBGT relative)
	case 0x2f:	addCycles(5);	if (branchLong(BLE())) addCycle();		break;		// BLE (LBLE relative)

	// STS
	case 0xdf:	addCycles(6);	break;		// ST (STS direct)
	case 0xef:	addCycles(6);	break;		// ST (STS indexed)
	case 0xff:	addCycles(7);	break;		// ST (STS extended)

	// STY
	case 0x9f:	addCycles(6);	break;		// ST (STY direct)
	case 0xaf:	addCycles(6);	break;		// ST (STY indexed)
	case 0xbf:	addCycles(7);	break;		// ST (STY extended)

	// SWI
	case 0x3f:	addCycles(20);	swi2();									break;		// SWI (SWI2 inherent)

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

	// SWI
	case 0x3f:	addCycles(20);	swi3();									break;		// SWI (SWI3 inherent)

	default:
		UNREACHABLE;
	}

	m_prefix11 = false;

	ASSUME(cycles() > 0);
	return cycles();
}

//

void EightBit::mc6809::push(register16_t& stack, uint8_t value) {
	BUS().write(stack--, value);
}

uint8_t EightBit::mc6809::pop(register16_t& stack) {
	return BUS().read(++stack);
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
			addCycles(2);
			address = r++;
			break;
		case 0b0001:	// ,R++
			addCycles(3);
			address = r;
			r += 2;
			break;
		case 0b0010:	// ,-R
			ASSUME(!indirect);
			addCycles(2);
			address = --r;
			break;
		case 0b0011:	// ,--R
			addCycles(3);
			r -= 2;
			address = r;
			break;
		case 0b0100:	// ,R
			address = r;
			break;
		case 0b0101:	// B,R
			addCycles(1);
			address = r + (int8_t)B();
			break;
		case 0b0110:	// A,R
			addCycles(1);
			address = r + (int8_t)A();
			break;
		case 0b1000:	// n,R (eight-bit)
			addCycles(1);
			address = r + (int8_t)fetchByte();
			break;
		case 0b1001:	// n,R (sixteen-bit)
			addCycles(4);
			address = r + (int16_t)fetchWord().word;
			break;
		case 0b1011:	// D,R
			addCycles(4);
			address = r + D();
			break;
		case 0b1100:	// n,PCR (eight-bit)
			addCycles(1);
			address = PC() + (int8_t)fetchByte();
			break;
		case 0b1101:	// n,PCR (sixteen-bit)
			addCycles(1);
			address = PC() + (int16_t)fetchWord().word;
			break;
		default:
			UNREACHABLE;
		}
		if (indirect) {
			addCycles(3);
			BUS().ADDRESS() = address;
			address = fetchWord();
		}
	} else {
		// EA = ,R + 5-bit offset
		addCycle();
		address = r + (type & Mask5);
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
	return BUS().read(Address_direct());
}

uint8_t EightBit::mc6809::AM_indexed_byte() {
	return BUS().read(Address_indexed());
}

uint8_t EightBit::mc6809::AM_extended_byte() {
	return BUS().read(Address_extended());
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
	CC() |= EF;
	pushWordS(PC());
	pushWordS(U());
	pushWordS(Y());
	pushWordS(X());
	pushS(DP());
	pushS(B());
	pushS(A());
	pushS(CC());
}

//

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
	adjustAddition(operand, data, addition);
	return addition & Mask16;
}

uint8_t EightBit::mc6809::andr(uint8_t operand, uint8_t data) {
	clearFlag(CC(), VF);
	adjustNZ(operand &= data);
	return operand;
}

uint8_t EightBit::mc6809::asl(uint8_t operand) {
	setFlag(CC(), CF, operand & Bit7);
	adjustNZ(operand <<= 1);
	const auto overflow = (CC() & CF) ^ ((CC() & NF) >> 3);
	setFlag(CC(), VF, overflow);
	return operand;
}

uint8_t EightBit::mc6809::asr(uint8_t operand) {
	setFlag(CC(), CF, operand & Bit7);
	adjustNZ(operand >>= 1);
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
	adjustSubtraction(operand, data, difference);
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
	saveEntireRegisterState();
	halt();
}

uint8_t EightBit::mc6809::da(uint8_t operand) {

	clearFlag(CC(), VF);
	setFlag(CC(), CF, A() > 0x99);

	const auto lowAdjust = (CC() & HF) || (lowNibble(A()) > 9);
	const auto highAdjust = (CC() & CF) || (A() > 0x99);

	if (lowAdjust)
		A() += 6;
	if (highAdjust)
		A() += 0x60;

	adjustNZ(A());
}

uint8_t EightBit::mc6809::dec(uint8_t operand) {
	const uint8_t result = operand - 1;
	adjustNZ(result);
	adjustOverflow(operand, 1, result);
	return result;
}

uint8_t EightBit::mc6809::eor(uint8_t operand, uint8_t data) {
	clearFlag(CC(), VF);
	adjustNZ(operand ^= data);
	return operand;
}

uint8_t& EightBit::mc6809::referenceTransfer8(int specifier) {
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

EightBit::register16_t& EightBit::mc6809::referenceTransfer16(int specifier) {
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

void EightBit::mc6809::exg(uint8_t data) {

	const auto reg1 = highNibble(data);
	const auto reg2 = lowNibble(data);

	const bool type16 = !!(reg1 & Bit3);	// 16 bit exchange?
	ASSUME(type16 == !!(reg2 & Bit3));		// Regardless, the register exchange must be equivalent

	if (type16)
		std::swap(referenceTransfer16(reg1), referenceTransfer16(reg2));
	else
		std::swap(referenceTransfer8(reg1), referenceTransfer8(reg2));
}

uint8_t EightBit::mc6809::inc(uint8_t operand) {
	const uint8_t result = operand + 1;
	adjustNZ(result);
	adjustOverflow(operand, 1, result);
	return result;
}

uint8_t EightBit::mc6809::ld(uint8_t data) {
	clearFlag(CC(), VF);
	adjustNZ(data);
	return data;
}

EightBit::register16_t EightBit::mc6809::ld(register16_t data) {
	clearFlag(CC(), VF);
	adjustNZ(data);
	return data;
}

uint8_t EightBit::mc6809::lsr(uint8_t operand) {
	setFlag(CC(), CF, operand & Bit0);
	adjustNZ(operand >>= 1);
	return operand;
}

EightBit::register16_t EightBit::mc6809::mul(uint8_t first, uint8_t second) {
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

uint8_t EightBit::mc6809::orr(uint8_t operand, uint8_t data) {
	clearFlag(CC(), VF);
	adjustNZ(operand |= data);
	return operand;
}

void EightBit::mc6809::pshs(uint8_t data) {
	if (data & Bit7) {
		addCycles(2);
		pushWordS(PC());
	}
	if (data & Bit6) {
		addCycles(2);
		pushWordS(U());
	}
	if (data & Bit5) {
		addCycles(2);
		pushWordS(Y());
	}
	if (data & Bit4) {
		addCycles(2);
		pushWordS(X());
	}
	if (data & Bit3) {
		addCycle();
		pushS(DP());
	}
	if (data & Bit2) {
		addCycle();
		pushS(B());
	}
	if (data & Bit1) {
		addCycle();
		pushS(A());
	}
	if (data & Bit0) {
		addCycle();
		pushS(CC());
	}
}

void EightBit::mc6809::pshu(uint8_t data) {
	if (data & Bit7) {
		addCycles(2);
		pushWordU(PC());
	}
	if (data & Bit6) {
		addCycles(2);
		pushWordU(S());
	}
	if (data & Bit5) {
		addCycles(2);
		pushWordU(Y());
	}
	if (data & Bit4) {
		addCycles(2);
		pushWordU(X());
	}
	if (data & Bit3) {
		addCycle();
		pushU(DP());
	}
	if (data & Bit2) {
		addCycle();
		pushU(B());
	}
	if (data & Bit1) {
		addCycle();
		pushU(A());
	}
	if (data & Bit0) {
		addCycle();
		pushU(CC());
	}
}

void EightBit::mc6809::puls(uint8_t data) {
	if (data & Bit0) {
		addCycle();
		CC() = popS();
	}
	if (data & Bit1) {
		addCycle();
		A() = popS();
	}
	if (data & Bit2) {
		addCycle();
		B() = popS();
	}
	if (data & Bit3) {
		addCycle();
		DP() = popS();
	}
	if (data & Bit4) {
		addCycles(2);
		X() = popWordS();
	}
	if (data & Bit5) {
		addCycles(2);
		Y() = popWordS();
	}
	if (data & Bit6) {
		addCycles(2);
		U() = popWordS();
	}
	if (data & Bit7) {
		addCycles(2);
		PC() = popWordS();
	}
}

void EightBit::mc6809::pulu(uint8_t data) {
	if (data & Bit0) {
		addCycle();
		CC() = popU();
	}
	if (data & Bit1) {
		addCycle();
		A() = popU();
	}
	if (data & Bit2) {
		addCycle();
		B() = popU();
	}
	if (data & Bit3) {
		addCycle();
		DP() = popU();
	}
	if (data & Bit4) {
		addCycles(2);
		X() = popWordU();
	}
	if (data & Bit5) {
		addCycles(2);
		Y() = popWordU();
	}
	if (data & Bit6) {
		addCycles(2);
		S() = popWordU();
	}
	if (data & Bit7) {
		addCycles(2);
		PC() = popWordU();
	}
}

uint8_t EightBit::mc6809::rol(uint8_t operand) {
	const auto carry = CC() & CF;
	setFlag(CC(), CF, operand & Bit7);
	setFlag(CC(), VF, ((operand & Bit7) >> 7) ^ ((operand & Bit6) >> 6));
	operand <<= 1;
	operand |= carry;
	adjustNZ(operand);
	return operand;
}
		
uint8_t EightBit::mc6809::ror(uint8_t operand) {
	const auto carry = CC() & CF;
	setFlag(CC(), CF, operand & Bit0);
	operand >>= 1;
	operand |= (carry << 7);
	adjustNZ(operand);
	return operand;
}

void EightBit::mc6809::rti() {
	CC() = popS();
	if (CC() & EF) {
		addCycles(9);	// One cycle per byte
		A() = popS();
		B() = popS();
		DP() = popS();
		X() = popWordS();
		Y() = popWordS();
		U() = popWordS();
	}
	ret();
}

void EightBit::mc6809::rts() {
	ret();
}

void EightBit::mc6809::swi() {
	saveEntireRegisterState();
	setFlag(CC(), (IF | FF));
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

uint8_t EightBit::mc6809::sex(uint8_t from) {
	const uint8_t result = from & Bit7 ? Mask8 : 0;
	adjustNZ(from);
	return result;
}

void EightBit::mc6809::tfr(uint8_t data) {

	const auto reg1 = highNibble(data);
	const auto reg2 = lowNibble(data);

	const bool type16 = !!(reg1 & Bit3);	// 16 bit transfer?
	ASSUME(type16 == !!(reg2 & Bit3));		// Regardless, the register transfer must be equivalent

	if (type16)
		referenceTransfer16(reg2) = referenceTransfer16(reg1);
	else
		referenceTransfer8(reg2) = referenceTransfer8(reg1);
}
